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
#include "netloader.h"
#include "regionfree.h"
#include "boot.h"

bool brewMode = false;
u8 sdmcCurrent = 0;
u64 nextSdCheck = 0;

menu_s menu;
u32 wifiStatus = 0;
u8 batteryLevel = 5;
u8 charging = 0;
int rebootCounter;

static enum
{
	HBMENU_DEFAULT,
	HBMENU_REGIONFREE,
	HBMENU_NETLOADER_ACTIVE,
	HBMENU_NETLOADER_UNAVAILABLE_NINJHAX2,
	HBMENU_NETLOADER_ERROR,
} hbmenu_state = HBMENU_DEFAULT;

int debugValues[100];

void drawDebug()
{
	char str[256];
	sprintf(str, "hello3 %08X %d %d %d %d %d %d %d\n%d %d %d %d\n%d %d %d %d\n%d %d %d %d\n", debugValues[50], debugValues[51], debugValues[52], debugValues[53], debugValues[54], debugValues[55], debugValues[56], debugValues[57], debugValues[58], debugValues[59], debugValues[60], debugValues[61], debugValues[62], debugValues[63], debugValues[64], debugValues[65], debugValues[66], debugValues[67], debugValues[68], debugValues[69]);
	gfxDrawText(GFX_TOP, GFX_LEFT, NULL, str, 32, 100);
}

void renderFrame(u8 bgColor[3], u8 waterBorderColor[3], u8 waterColor[3])
{
	// background stuff
	drawBackground(bgColor, waterBorderColor, waterColor);

	// status bar
	drawStatusBar(wifiStatus, charging, batteryLevel);

	// current directory
	printDirectory();

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
			"                                                                                            B : Cancel\n",
			0);
	}else if(!sdmcCurrent)
	{
		//no SD
		drawError(GFX_BOTTOM,
			"No SD detected",
			"    It looks like your 3DS doesn't have an SD inserted into it.\n"
			"    Please insert an SD card for optimal homebrew launcher performance !\n",
			0);
	}else if(sdmcCurrent<0)
	{
		//SD error
		drawError(GFX_BOTTOM,
			"SD Error",
			"    Something unexpected happened when trying to mount your SD card.\n"
			"    Try taking it out and putting it back in. If that doesn't work,\n"
			"please try again with another SD card.",
			0);
	}else if(hbmenu_state == HBMENU_NETLOADER_ACTIVE){
		char bof[256];
		u32 ip = gethostid();
		sprintf(bof,
			"    NetLoader Active\n"
			"    IP: %lu.%lu.%lu.%lu, Port: %d\n\n"
			"                                                                                            B : Cancel\n",
			ip & 0xFF, (ip>>8)&0xFF, (ip>>16)&0xFF, (ip>>24)&0xFF, NETLOADER_PORT);

		drawError(GFX_BOTTOM,
			"NetLoader",
			bof,
			0);
	}else if(hbmenu_state == HBMENU_NETLOADER_UNAVAILABLE_NINJHAX2){
		drawError(GFX_BOTTOM,
			"NetLoader",
			"    The NetLoader is currently unavailable. :(\n"
			"    This might be normal and fixable. Try and enable it ?\n\n"
			"                                                                                            A : Yes\n"
			"                                                                                            B : No\n",
			0);
	}else if(hbmenu_state == HBMENU_REGIONFREE){
		if(regionFreeGamecardIn)
		{
			drawError(GFX_BOTTOM,
				"Region free launcher",
				"    The region free launcher is ready to run your out-of-region gamecard !\n\n"
				"                                                                                                                 A : Play\n"
				"                                                                                                                 B : Cancel\n",
				10-drawMenuEntry(&gamecardMenuEntry, GFX_BOTTOM, 240, 9, true));
		}else{
			drawError(GFX_BOTTOM,
				"Region free launcher",
				"    The region free loader cannot detect a gamecard in the slot.\n"
				"    Please insert a gamecard in your console before continuing.\n\n"
				"                                                                                            B : Cancel\n",
				0);
		}
	}else if(hbmenu_state == HBMENU_NETLOADER_ERROR){
		netloader_draw_error();
	}else{
		//got SD
		drawMenu(&menu);
	}
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

// handled in main
// doing it in main is preferred because execution ends in launching another 3dsx
void __appInit()
{
	srvInit();
}

// same
void __appExit()
{
	srvExit();
}

int main()
{
	aptInit();
	gfxInitDefault();
	initFilesystem();
	openSDArchive();
	hidInit(NULL);
	acInit();
	ptmInit();
	regionFreeInit();
	netloader_init();

	initBackground();
	initErrors();
	initMenu(&menu);

	u8 sdmcPrevious = 0;
	FSUSER_IsSdmcDetected(NULL, &sdmcCurrent);
	if(sdmcCurrent == 1)
	{
		scanHomebrewDirectory(&menu);
	}
	sdmcPrevious = sdmcCurrent;
	nextSdCheck = osGetTime()+250;

	srand(svcGetSystemTick());

	rebootCounter=257;

	while(aptMainLoop())
	{
		if (nextSdCheck < osGetTime())
		{
			regionFreeUpdate();

			FSUSER_IsSdmcDetected(NULL, &sdmcCurrent);

			if(sdmcCurrent == 1 && (sdmcPrevious == 0 || sdmcPrevious < 0))
			{
				closeSDArchive();
				openSDArchive();
				scanHomebrewDirectory(&menu);
			}
			else if(sdmcCurrent < 1 && sdmcPrevious == 1)
			{
				clearMenuEntries(&menu);
			}
			sdmcPrevious = sdmcCurrent;
			nextSdCheck = osGetTime()+250;
		}

		ACU_GetWifiStatus(NULL, &wifiStatus);
		PTMU_GetBatteryLevel(NULL, &batteryLevel);
		PTMU_GetBatteryChargeState(NULL, &charging);
		hidScanInput();

		updateBackground();

		if(hbmenu_state == HBMENU_NETLOADER_ACTIVE){
			if(hidKeysDown()&KEY_B){
				netloader_deactivate();
				hbmenu_state = HBMENU_DEFAULT;
			}else{
				int rc = netloader_loop();
				if(rc > 0)
				{
					netloader_boot = true;
					break;
				}else if(rc < 0){
					hbmenu_state = HBMENU_NETLOADER_ERROR;
				}
			}
		}else if(hbmenu_state == HBMENU_NETLOADER_UNAVAILABLE_NINJHAX2){
			if(hidKeysDown()&KEY_B){
				hbmenu_state = HBMENU_DEFAULT;
			}else if(hidKeysDown()&KEY_A){
				if(isNinjhax2())
				{
					// basically just relaunch boot.3dsx w/ scanning in hopes of getting netloader capabilities
					static char hbmenuPath[] = "/boot.3dsx";
					netloadedPath = hbmenuPath; // fine since it's static
					netloader_boot = true;
					break;
				}
			}
		}else if(hbmenu_state == HBMENU_REGIONFREE){
			if(hidKeysDown()&KEY_B){
				hbmenu_state = HBMENU_DEFAULT;
			}else if(hidKeysDown()&KEY_A && regionFreeGamecardIn)
			{
				// region free menu entry is selected so we can just break out like updateMenu() normally would
				break;
			}
		}else if(hbmenu_state == HBMENU_NETLOADER_ERROR){
			if(hidKeysDown()&KEY_B)
				hbmenu_state = HBMENU_DEFAULT;
		}else if(rebootCounter==256){
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
			if(hidKeysDown()&KEY_Y)
			{
				if(netloader_activate() == 0) hbmenu_state = HBMENU_NETLOADER_ACTIVE;
				else if(isNinjhax2()) hbmenu_state = HBMENU_NETLOADER_UNAVAILABLE_NINJHAX2;
			}
			if(secretCode())brewMode = true;
			else if(hidKeysDown()&KEY_B) {
					changeDirectory("..");
					clearMenuEntries(&menu);
					initMenu(&menu);
					scanHomebrewDirectory(&menu);
			}
			else if(updateMenu(&menu))
			{
				menuEntry_s* me = getMenuEntry(&menu, menu.selectedEntry);
				if(me && me->type == MENU_ENTRY_FOLDER) {
					changeDirectory(me->executablePath);
					clearMenuEntries(&menu);
					initMenu(&menu);
					scanHomebrewDirectory(&menu);
				} else
				if(me && !strcmp(me->executablePath, REGIONFREE_PATH) && regionFreeAvailable && !netloader_boot)
				{
					hbmenu_state = HBMENU_REGIONFREE;
					regionFreeUpdate();
				}else break;
			}
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

	menuEntry_s* me = getMenuEntry(&menu, menu.selectedEntry);

	if(netloader_boot)
	{
		me = malloc(sizeof(menuEntry_s));
		initMenuEntry(me, netloadedPath, "netloaded app", "", "", NULL, MENU_ENTRY_FILE);
	}

	scanMenuEntry(me);

	// cleanup whatever we have to cleanup
	netloader_exit();
	ptmExit();
	acExit();
	hidExit();
	gfxExit();
	exitFilesystem();
	closeSDArchive();
	aptExit();

	if (!strcmp(me->executablePath, REGIONFREE_PATH) && regionFreeAvailable && !netloader_boot)return regionFreeRun();

	regionFreeExit();

	return bootApp(me->executablePath, &me->metadata);
}
