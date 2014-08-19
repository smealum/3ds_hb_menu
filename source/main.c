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
#include "controls.h"
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
		else if(bubbles[i].y / 240 && bubbles[i].y % 240 > 100)
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
		gfxDrawSpriteAlphaBlendFade(bubbles[i].y / 240, (u8*)bubble_bin, 32, 32, 
			((bubbles[i].y / 240) ? -64 : 0) + bubbles[i].y % 240, 
			((bubbles[i].y / 240) ? 0 : -40) + bubbles[i].x + (bubbles[i].wobble >> 8), bubbles[i].fade);
	}
}

void drawStatusBar()
{
	// status bar
	gfxDrawSpriteAlphaBlend(true, (u8*)top_bar_bin, 16, 400, 240 - 16, 0);
	if(wifiStatus)
	{
		gfxDrawSpriteAlphaBlend(true, (u8*)wifi_full_bin, 16, 32, 240 - 16, 0);
	}
	else
	{
		gfxDrawSpriteAlphaBlend(true, (u8*)wifi_none_bin, 16, 32, 240 - 16, 0);
	}
	if(!charging)
	{
		switch(batteryLevel)
		{
		case 5:
			gfxDrawSpriteAlphaBlend(true, (u8*)battery_full_bin, 16, 32, 240 - 16, 400 - 32);
			break;
		case 4:
			gfxDrawSpriteAlphaBlend(true, (u8*)battery_mid_high_bin, 16, 32, 240 - 16, 400 - 32);
			break;
		case 3:
			gfxDrawSpriteAlphaBlend(true, (u8*)battery_mid_low_bin, 16, 32, 240 - 16, 400 - 32);
			break;
		case 2:
			gfxDrawSpriteAlphaBlend(true, (u8*)battery_low_bin, 16, 32, 240 - 16, 400 - 32);
			break;
		default:
			gfxDrawSpriteAlphaBlend(true, (u8*)battery_lowest_bin, 16, 32, 240 - 16, 400 - 32);
			break;
		}
	}
	else
	{
		gfxDrawSpriteAlphaBlend(true, (u8*)battery_charging_bin, 16, 32, 240 - 16, 400 - 32);
	}
}

void drawLogo()
{
	// logo
	char str[256];
	debugValues[0] = charging;
	sprintf(str, "hello %d %d %08X %08X\n", debugValues[0], debugValues[1], (unsigned int)debugValues[2], (unsigned int)debugValues[3]);
	gfxDrawText(true, str, 8, 100);
	gfxDrawSpriteAlpha(true, (u8*)logo_bin, 182, 245, 28, 80);
}

void renderFrame()
{
	//background stuff
	drawBackground();

	//top screen stuff
	drawBubbles();
	drawStatusBar();
	drawLogo();

	//menu stuff
	drawMenu(&menu);
}
void renderFrameBrew()
{
	//background stuff
	drawBackgroundBrew();

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

int main()
{
	srvInit();
	_aptInit(APPID_APPLICATION);
	initFilesystem();
	gfxInit();

	initControls();
	initBackground();
	
	initMenu(&menu);
	scanHomebrewDirectory(&menu, "/3ds/");

	srand(svcGetSystemTick());

	updateControls();
	if((keysDown() & PAD_L) || (keysDown() & PAD_R))
	{
		brewMode = true;
	}


	APP_STATUS status;
	if(brewMode)
	{
		while((status=aptGetStatus())!=APP_EXITING)
		{
			ACU_GetWifiStatus(acHandle, &wifiStatus);
			PTMU_GetBatteryLevel(ptmHandle, &batteryLevel);
			PTMU_GetBatteryChargeState(ptmHandle, &charging);
			updateControls();
			if(updateMenu(&menu))break;
			renderFrameBrew();
			gfxFlushBuffers();
			gfxSwapBuffers();
			svcSleepThread(8333333);
		}
	}
	else
	{
		while((status=aptGetStatus())!=APP_EXITING)
		{
			ACU_GetWifiStatus(acHandle, &wifiStatus);
			PTMU_GetBatteryLevel(ptmHandle, &batteryLevel);
			PTMU_GetBatteryChargeState(ptmHandle, &charging);
			updateControls();
			if(updateMenu(&menu))break;
			renderFrame();
			gfxFlushBuffers();
			gfxSwapBuffers();
			svcSleepThread(8333333);
		}
	}

	// cleanup whatever we have to cleanup
	// TODO : call whatever needs to be called to free main heap
	exitControls();
	gfxExit();
	exitFilesystem();
	_aptExit();
	srvExit();

	//open file that we're going to boot up
	fsInit();
	Handle fileHandle;
	menuEntry_s* me=getMenuEntry(&menu, menu.selectedEntry); //TODO : check that it's not NULL ?
	debugValues[2]=FSUSER_OpenFileDirectly(NULL, &fileHandle, sdmcArchive, FS_makePath(PATH_CHAR, me->executablePath), FS_OPEN_READ, FS_ATTRIBUTE_NONE);
	fsExit();

	u32 blockAddr;
	svcControlMemory(&blockAddr, 0x08000000, 0x0, 0x1910000, MEMOP_FREE, 0x0);

	//jump to bootloader
	void (*callBootloader)(Handle h, Handle file)=(void*)0x000F0000;
	callBootloader(0x00000000, fileHandle);
	return 0;
}
