#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/srv.h>
#include <3ds/APT.h>
#include <3ds/HID.h>
#include <3ds/GSP.h>
#include <3ds/GX.h>
#include <3ds/FS.h>
#include <3ds/svc.h>
#include <3ds/AC.h>

#include "PTM.h"

#include "gfx.h"
#include "menu.h"
#include "background.h"
#include "filesystem.h"

#include "logo_bin.h"
#include "bubble_bin.h"

#include "top_bar_bin.h"
#include "wifi_full_bin.h"
#include "wifi_none_bin.h"
#include "battery_full_bin.h"
#include "battery_mid_high_bin.h"
#include "battery_mid_low_bin.h"
#include "battery_low_bin.h"
#include "battery_lowest_bin.h"
#include "battery_charging_bin.h"

bool brewMode = false;

menu_s menu;
u32 wifiStatus = 0;
u8 batteryLevel = 5;
u8 charging = 0;

int debugValues[100];

#define BUBBLE_COUNT 15

typedef struct
{
	s32 x, y;
	u8 fade;
	s32 wobble;
	bool wobbleLeft;
}bubble_t;

bubble_t bubbles[BUBBLE_COUNT];

void drawBubbles()
{
	int i = 0;
	//BUBBLES!!
	for(i = 0;i < BUBBLE_COUNT;i += 1)
	{
		bubbles[i].y += 2;
		if(bubbles[i].fade < 10)
		{
			bubbles[i].x = rand() % 400;
			bubbles[i].y = rand() % 10;
			bubbles[i].wobble = ((rand() % 20) - 10) << 8;
			bubbles[i].fade = 15;
		}
		else if(bubbles[i].y >= 240 && bubbles[i].y % 240 > 100)
		{
			bubbles[i].fade -= 10;
		}
		else if(bubbles[i].fade < 255)
		{
			bubbles[i].fade += 10;
		}

		if(bubbles[i].wobbleLeft)
		{
			if(bubbles[i].wobble >> 8 <= -5)
			{
				bubbles[i].wobbleLeft = false;
			}
			else
			{
				bubbles[i].wobble -= 8;
			}
		}
		else
		{
			if(bubbles[i].wobble >> 8 >= 5)
			{
				bubbles[i].wobbleLeft = true;
			}
			else
			{
				bubbles[i].wobble += 8;
			}
		}
		gfxDrawSpriteAlphaBlendFade((bubbles[i].y >= 240) ? (GFX_TOP) : (GFX_BOTTOM), GFX_LEFT, (u8*)bubble_bin, 32, 32, 
			((bubbles[i].y >= 240) ? -64 : 0) + bubbles[i].y % 240, 
			((bubbles[i].y >= 240) ? 0 : -40) + bubbles[i].x + (bubbles[i].wobble >> 8), bubbles[i].fade);
	}
}

u8* batteryLevels[] = {
	(u8*)battery_lowest_bin,
	(u8*)battery_lowest_bin,
	(u8*)battery_low_bin,
	(u8*)battery_mid_low_bin,
	(u8*)battery_mid_high_bin,
	(u8*)battery_full_bin,
};

void drawStatusBar()
{
	// status bar
	gfxDrawSpriteAlphaBlend(GFX_TOP, GFX_LEFT, (u8*)top_bar_bin, 16, 400, 240 - 16, 0);

	if(wifiStatus)
	{
		gfxDrawSpriteAlphaBlend(GFX_TOP, GFX_LEFT, (u8*)wifi_full_bin, 16, 32, 240 - 16, 0);
	}
	else
	{
		gfxDrawSpriteAlphaBlend(GFX_TOP, GFX_LEFT, (u8*)wifi_none_bin, 16, 32, 240 - 16, 0);
	}

	if(!charging)
	{
		gfxDrawSpriteAlphaBlend(GFX_TOP, GFX_LEFT, batteryLevels[batteryLevel], 16, 32, 240 - 16, 400 - 32);
	}
	else
	{
		gfxDrawSpriteAlphaBlend(GFX_TOP, GFX_LEFT, (u8*)battery_charging_bin, 16, 32, 240 - 16, 400 - 32);
	}
}

void drawLogo()
{
	// logo
	char str[256];
	debugValues[0] = charging;
	sprintf(str, "hello %d %d %08X %08X\n", debugValues[0], debugValues[1], (unsigned int)debugValues[2], (unsigned int)debugValues[3]);
	gfxDrawText(GFX_TOP, GFX_LEFT, str, 8, 100);
	gfxDrawSpriteAlphaBlend(GFX_TOP, GFX_LEFT, (u8*)logo_bin, 113, 271, 64, 80);
}

void renderFrame(u8 bgColor[3], u8 waterBorderColor[3], u8 waterColor[3])
{
	//background stuff
	drawBackground(bgColor, waterBorderColor, waterColor);

	//top screen stuff
	drawBubbles();
	drawStatusBar();
	drawLogo();

	//menu stuff
	drawMenu(&menu);
}

//TEMP
extern Handle aptStatusMutex, aptStatusEvent;
extern Handle aptuHandle, aptLockHandle;
Handle acHandle, ptmHandle;
extern NS_APPID currentAppId;

void _aptExit()
{
	svcCloseHandle(ptmHandle);
	svcCloseHandle(acHandle);
	svcCloseHandle(aptStatusMutex);
	svcCloseHandle(aptLockHandle);
	svcCloseHandle(aptStatusEvent);
}

Result _aptInit(NS_APPID appID)
{
	int i = 0;
	for(i = 0;i < BUBBLE_COUNT;i += 1)
	{
		bubbles[i].x = rand() % 400;
		bubbles[i].y = rand() % 240;
		bubbles[i].wobble = ((rand() % 20) - 10) << 8;
		bubbles[i].fade = 15;
	}

	Result ret=0;

	//initialize APT stuff, escape load screen
	srvGetServiceHandle(&aptuHandle, "APT:U");
	if((ret=APT_GetLockHandle(&aptuHandle, 0x0, &aptLockHandle)))return ret;
	svcCloseHandle(aptuHandle);

	srvGetServiceHandle(&acHandle, "ac:u");
	srvGetServiceHandle(&ptmHandle, "ptm:u");

	currentAppId=appID;

	svcCreateEvent(&aptStatusEvent, 0);
	
	return 0;
}

extern void (*__system_retAddr)(void);
static Handle hbHandle;
static void launchFile(void)
{
	//jump to bootloader
	void (*callBootloader)(Handle h, Handle file)=(void*)0x000F0000;
	callBootloader(0x00000000, hbHandle);
}

bool secretCode(void)
{
  static const u32 secret_code[] =
  {
    KEY_UP,
    KEY_UP,
    KEY_DOWN,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_B,
    KEY_A,
  };

  static u32 state   = 0;
  static u32 timeout = 30;
  u32 down = hidKeysDown();

  if(down & secret_code[state])
  {
    ++state;
    timeout = 30;

    if(state == sizeof(secret_code)/sizeof(secret_code[0]))
    {
      state = 0;
      return true;
    }
  }

  if(timeout > 0 && --timeout == 0)
    state = 0;

  return false;
}

int main()
{
	srvInit();
	_aptInit(APPID_APPLICATION);
	initFilesystem();
	gfxInit();
	hidInit(NULL);

	initBackground();
	
	initMenu(&menu);
	scanHomebrewDirectory(&menu, "/3ds/");

	srand(svcGetSystemTick());

	APP_STATUS status;
	while((status=aptGetStatus())!=APP_EXITING)
	{
		ACU_GetWifiStatus(acHandle, &wifiStatus);
		PTMU_GetBatteryLevel(ptmHandle, &batteryLevel);
		PTMU_GetBatteryChargeState(ptmHandle, &charging);
		hidScanInput();
		if(secretCode())
			brewMode = true;
		else if(updateMenu(&menu))
			break;
		if (brewMode)
			renderFrame(BGCOLOR, BEERBORDERCOLOR, BEERCOLOR);
		else
			renderFrame(BGCOLOR, WATERBORDERCOLOR, WATERCOLOR);
		gfxFlushBuffers();
		gfxSwapBuffers();
		svcSleepThread(8333333);
	}

	// cleanup whatever we have to cleanup
	// TODO : call whatever needs to be called to free main heap
	hidExit();
	gfxExit();
	exitFilesystem();
	_aptExit();
	srvExit();

	//open file that we're going to boot up
	fsInit();
	menuEntry_s* me=getMenuEntry(&menu, menu.selectedEntry); //TODO : check that it's not NULL ?
	debugValues[2]=FSUSER_OpenFileDirectly(NULL, &hbHandle, sdmcArchive, FS_makePath(PATH_CHAR, me->executablePath), FS_OPEN_READ, FS_ATTRIBUTE_NONE);
	fsExit();

	// Override return address to homebrew booting code
	__system_retAddr = launchFile;
	return 0;
}
