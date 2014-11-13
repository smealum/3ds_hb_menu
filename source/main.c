#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <3ds.h>

#include "gfx.h"
#include "menu.h"
#include "background.h"
#include "statusbar.h"
#include "filesystem.h"
#include "error.h"

#define CN_BOOTLOADER_LOC 0x000F0000
#define CN_ARGSETTER_LOC 0x000F2000

bool brewMode = false;
u32 sdmcCurrent = 0;

menu_s menu;
u32 wifiStatus = 0;
u8 batteryLevel = 5;
u8 charging = 0;

int debugValues[100];

void drawDebug()
{
	char str[256];
	// sprintf(str, "hello3 %d %d %08X %08X\n", debugValues[0], debugValues[1], (unsigned int)debugValues[2], (unsigned int)debugValues[3]);
	sprintf(str, "hello3 %d %d %d %d\n", debugValues[0], debugValues[1], debugValues[2], debugValues[3]);
	gfxDrawText(GFX_TOP, GFX_LEFT, NULL, str, 32, 100);
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
	if(!sdmcCurrent)
	{
		//no SD
		drawError(GFX_BOTTOM,
			"No SD detected",
			"    It looks like your 3DS doesn't have an SD inserted into it.\n"
			"    Please insert an SD card for optimal homebrew launcher performance !\n");
	}else if(sdmcCurrent<0)
	{
		//SD error
		drawError(GFX_BOTTOM,
			"SD Error",
			"    Something unexpected happened when trying to mount your SD card.\n"
			"    Try taking it out and putting it back in. If that doesn't work,\n"
			"please try again with another SD card.");
	}else{
		//got SD
		drawMenu(&menu);
	}
}

extern void (*__system_retAddr)(void);
static Handle hbHandle;
static void launchFile(void)
{
	//jump to bootloader
	void (*callBootloader)(Handle hb, Handle file)=(void*)CN_BOOTLOADER_LOC;
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
	gfxInit();
	initFilesystem();
	openSDArchive();
	hidInit(NULL);
	irrstInit(NULL);
	acInit();
	ptmInit();

	initBackground();
	initErrors();
	initMenu(&menu);

	u32 sdmcPrevious = 0;
	FSUSER_IsSdmcDetected(NULL, &sdmcCurrent);
	if(sdmcCurrent == 1)
	{
		scanHomebrewDirectory(&menu, "/3ds/");
	}
	sdmcPrevious = sdmcCurrent;

	srand(svcGetSystemTick());

	while(aptMainLoop())
	{
		FSUSER_IsSdmcDetected(NULL, &sdmcCurrent);
		
		if(sdmcCurrent == 1 && (sdmcPrevious == 0 || sdmcPrevious < 0))
		{
			closeSDArchive();
			openSDArchive();
			scanHomebrewDirectory(&menu, "/3ds/");
		}
		else if(sdmcCurrent < 1 && sdmcPrevious == 1)
		{
			clearMenuEntries(&menu);
		}
		sdmcPrevious = sdmcCurrent;
			
		ACU_GetWifiStatus(NULL, &wifiStatus);
		PTMU_GetBatteryLevel(NULL, &batteryLevel);
		PTMU_GetBatteryChargeState(NULL, &charging);
		hidScanInput();

		updateBackground();

		if(secretCode())brewMode = true;
		else if(updateMenu(&menu))break;

		if(brewMode)renderFrame(BGCOLOR, BEERBORDERCOLOR, BEERCOLOR);
		else renderFrame(BGCOLOR, WATERBORDERCOLOR, WATERCOLOR);

		gfxFlushBuffers();
		gfxSwapBuffers();

		//TEMP
		if(hidKeysDown()&KEY_START)*(u32*)NULL=0xDEADBABE; // trigger crash to reboot console

		gspWaitForVBlank();
	}

	// cleanup whatever we have to cleanup
	ptmExit();
	acExit();
	irrstExit();
	hidExit();
	gfxExit();
	exitFilesystem();
	closeSDArchive();
	aptExit();
	srvExit();

	//open file that we're going to boot up
	fsInit();
	menuEntry_s* me=getMenuEntry(&menu, menu.selectedEntry);
	debugValues[2]=FSUSER_OpenFileDirectly(NULL, &hbHandle, sdmcArchive, FS_makePath(PATH_CHAR, me->executablePath), FS_OPEN_READ, FS_ATTRIBUTE_NONE);
	fsExit();

	//set argv/argc
	static u32 argbuffer[0x200];
	void (*setArgs)(u32* src, u32 length)=(void*)CN_ARGSETTER_LOC;
	argbuffer[0]=1;
	strcpy((char*)&argbuffer[1], me->executablePath);
	setArgs(argbuffer, 0x200*4);

	// Override return address to homebrew booting code
	__system_retAddr = launchFile;
	return 0;
}
