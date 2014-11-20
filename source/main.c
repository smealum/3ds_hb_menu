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
#include "hb.h"

bool brewMode = false;
u32 sdmcCurrent = 0;

menu_s menu;
u32 wifiStatus = 0;
u8 batteryLevel = 5;
u8 charging = 0;
int rebootCounter;

int debugValues[100];

void drawDebug()
{
	char str[256];
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
	// drawDebug();

	//menu stuff
	if(rebootCounter<257)
	{
		//about to reboot
		drawError(GFX_BOTTOM,
			"Reboot",
			"    You're about to reboot your console into home menu.\n\n"
			"                                                                                            A : Proceed\n"
			"                                                                                            B : Cancel\n");
	}else if(!sdmcCurrent)
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
void (*callBootloader)(Handle hb, Handle file);
void (*setArgs)(u32* src, u32 length);

static void launchFile(void)
{
	//jump to bootloader
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
	initHb();

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

	rebootCounter=257;

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

		if(rebootCounter==256)
		{
			if(hidKeysDown()&KEY_A)
			{
				//reboot
				aptOpenSession();
					APT_HardwareResetAsync(NULL);
				aptCloseSession();
				rebootCounter--;
			}else if(hidKeysDown()&KEY_B)
			{
				rebootCounter++;
			}
		}else if(rebootCounter==257){
			if(hidKeysDown()&KEY_START)rebootCounter--;
			if(secretCode())brewMode = true;
			else if(updateMenu(&menu))break;
		}

		if(brewMode)renderFrame(BGCOLOR, BEERBORDERCOLOR, BEERCOLOR);
		else renderFrame(BGCOLOR, WATERBORDERCOLOR, WATERCOLOR);

		if(rebootCounter<256)
		{
			if(rebootCounter<0)rebootCounter=0;
			gfxFadeScreen(GFX_TOP, GFX_LEFT, rebootCounter);
			gfxFadeScreen(GFX_BOTTOM, GFX_BOTTOM, rebootCounter);
			if(rebootCounter>0)rebootCounter-=6;
		}

		gfxFlushBuffers();
		gfxSwapBuffers();

		gspWaitForVBlank();
	}

	//get bootloader addresses before exiting everything
	HB_GetBootloaderAddresses((void**)&callBootloader, (void**)&setArgs);

	// cleanup whatever we have to cleanup
	exitHb();
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
	argbuffer[0]=1;
	snprintf((char*)&argbuffer[1], 0x200*4, "sdmc:%s", me->executablePath);
	setArgs(argbuffer, 0x200*4);

	// Override return address to homebrew booting code
	__system_retAddr = launchFile;
	return 0;
}
