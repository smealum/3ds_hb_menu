#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <3ds.h>

#include "gfx.h"
#include "menu.h"
#include "background.h"
#include "statusbar.h"
#include "filesystem.h"

bool brewMode = false;

menu_s menu;
u32 wifiStatus = 0;
u8 batteryLevel = 5;
u8 charging = 0;

int debugValues[100];

void drawDebug()
{
	// logo
	char str[256];
	debugValues[0] = charging;
	sprintf(str, "hello %d %d %08X %08X\n", debugValues[0], debugValues[1], (unsigned int)debugValues[2], (unsigned int)debugValues[3]);
	gfxDrawText(GFX_TOP, GFX_LEFT, str, 8, 100);
}

void renderFrame(u8 bgColor[3], u8 waterBorderColor[3], u8 waterColor[3])
{
	// background stuff
	drawBackground(bgColor, waterBorderColor, waterColor);

	// status bar
	drawStatusBar(wifiStatus, charging, batteryLevel);

	// debug text
	drawDebug();

	//menu stuff
	drawMenu(&menu);
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
	aptInit();
	initFilesystem();
	gfxInit();
	hidInit(NULL);
	acInit();
	ptmInit();

	aptSetupEventHandler();

	initBackground();
	
	initMenu(&menu);
	scanHomebrewDirectory(&menu, "/3ds/");

	srand(svcGetSystemTick());

	APP_STATUS status;
	while((status=aptGetStatus())!=APP_EXITING)
	{
		if(status == APP_RUNNING)
		{
			ACU_GetWifiStatus(NULL, &wifiStatus);
			PTMU_GetBatteryLevel(NULL, &batteryLevel);
			PTMU_GetBatteryChargeState(NULL, &charging);
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
		}
		else if(status == APP_SUSPENDING)
		{
			aptReturnToMenu();
		}
		else if(status == APP_SLEEPMODE)
		{
			aptWaitStatusEvent();
		}

		svcSleepThread(8333333);
	}

	// cleanup whatever we have to cleanup
	ptmExit();
	acExit();
	hidExit();
	gfxExit();
	exitFilesystem();
	aptExit();
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
