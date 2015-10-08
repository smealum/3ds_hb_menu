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
#include "titles.h"

bool brewMode = false;
u8 sdmcCurrent = 0;
u64 nextSdCheck = 0;

menu_s menu;
u32 wifiStatus = 0;
u8 batteryLevel = 5;
u8 charging = 0;
int rebootCounter;
titleBrowser_s titleBrowser;

static enum
{
	HBMENU_DEFAULT,
	HBMENU_REGIONFREE,
	HBMENU_TITLESELECT,
	HBMENU_TITLETARGET_ERROR,
	HBMENU_NETLOADER_ACTIVE,
	HBMENU_NETLOADER_UNAVAILABLE_NINJHAX2,
	HBMENU_NETLOADER_ERROR,
} hbmenu_state = HBMENU_DEFAULT;

int debugValues[100];

void drawDebug()
{
	char str[256];
	sprintf(str, "hello3 %08X %d %d %d %d %d %d %d\n\n%08X %08X %08X %08X\n\n%08X %08X %08X %08X\n\n%08X %08X %08X %08X\n\n", debugValues[50], debugValues[51], debugValues[52], debugValues[53], debugValues[54], debugValues[55], debugValues[56], debugValues[57], debugValues[58], debugValues[59], debugValues[60], debugValues[61], debugValues[62], debugValues[63], debugValues[64], debugValues[65], debugValues[66], debugValues[67], debugValues[68], debugValues[69]);
	gfxDrawText(GFX_TOP, GFX_LEFT, NULL, str, 48, 100);
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
			"    NetLoader Active - waiting for 3dslink connection\n"
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
	}else if(hbmenu_state == HBMENU_TITLESELECT){
		drawTitleBrowser(&titleBrowser);
	}else if(hbmenu_state == HBMENU_TITLETARGET_ERROR){
		drawError(GFX_BOTTOM,
			"Missing target title",
			"    The application you are trying to run requested a specific target title.\n"
			"    Please make sure you have that title !\n\n"
			"                                                                                            B : Back\n",
			0);
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
}

// same
void __appExit()
{
}

int main()
{
	srvInit();
	aptInit();
	gfxInitDefault();
	initFilesystem();
	openSDArchive();
	hidInit(NULL);
	irrstInit(NULL);
	acInit();
	ptmInit();
	titlesInit();
	regionFreeInit();
	netloader_init();

	// offset potential issues caused by homebrew that just ran
	aptOpenSession();
	APT_SetAppCpuTimeLimit(NULL, 0);
	aptCloseSession();

	initBackground();
	initErrors();
	initMenu(&menu);
	initTitleBrowser(&titleBrowser, NULL);

	u8 sdmcPrevious = 0;
	FSUSER_IsSdmcDetected(NULL, &sdmcCurrent);
	if(sdmcCurrent == 1)
	{
		scanHomebrewDirectory(&menu, "/3ds/");
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
				scanHomebrewDirectory(&menu, "/3ds/");
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

		menuEntry_s* me = getMenuEntry(&menu, menu.selectedEntry);
		debugValues[50] = me->descriptor.numTargetTitles;
		debugValues[51] = me->descriptor.selectTargetProcess;
		if(me->descriptor.numTargetTitles)
		{
			debugValues[58] = (me->descriptor.targetTitles[0].tid >> 32) & 0xFFFFFFFF;
			debugValues[59] = me->descriptor.targetTitles[0].tid & 0xFFFFFFFF;
		}

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
		}else if(hbmenu_state == HBMENU_TITLETARGET_ERROR){
			if(hidKeysDown()&KEY_B){
				hbmenu_state = HBMENU_DEFAULT;
			}
		}else if(hbmenu_state == HBMENU_TITLESELECT){
			if(hidKeysDown()&KEY_A && titleBrowser.selected)
			{
				targetProcessId = -2;
				target_title = *titleBrowser.selected;
				break;
			}
			else if(hidKeysDown()&KEY_B)hbmenu_state = HBMENU_DEFAULT;
			else updateTitleBrowser(&titleBrowser);
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
			else if(updateMenu(&menu))
			{
				menuEntry_s* me = getMenuEntry(&menu, menu.selectedEntry);
				if(me && !strcmp(me->executablePath, REGIONFREE_PATH) && regionFreeAvailable && !netloader_boot)
				{
					hbmenu_state = HBMENU_REGIONFREE;
					regionFreeUpdate();
				}else
				{
					// if appropriate, look for specified titles in list
					if(me->descriptor.numTargetTitles)
					{
						// go through target title list in order so that first ones on list have priority
						int i;
						titleInfo_s* ret = NULL;
						for(i=0; i<me->descriptor.numTargetTitles; i++)
						{
							ret = findTitleBrowser(&titleBrowser, me->descriptor.targetTitles[i].mediatype, me->descriptor.targetTitles[i].tid);
							if(ret)break;
						}

						if(ret)
						{
							targetProcessId = -2;
							target_title = *ret;
							break;
						}

						// if we get here, we aint found shit
						// if appropriate, let user select target title
						if(me->descriptor.selectTargetProcess) hbmenu_state = HBMENU_TITLESELECT;
						else hbmenu_state = HBMENU_TITLETARGET_ERROR;
					}else
					{
						if(me->descriptor.selectTargetProcess) hbmenu_state = HBMENU_TITLESELECT;
						else break;
					}


				}
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
		initMenuEntry(me, netloadedPath, "netloaded app", "", "", NULL);
	}

	scanMenuEntry(me);

	// cleanup whatever we have to cleanup
	netloader_exit();
	titlesExit();
	ptmExit();
	acExit();
	irrstExit();
	hidExit();
	gfxExit();
	closeSDArchive();
	exitFilesystem();
	aptExit();
	srvExit();

	if(!strcmp(me->executablePath, REGIONFREE_PATH) && regionFreeAvailable && !netloader_boot)return regionFreeRun();
	
	regionFreeExit();

	return bootApp(me->executablePath, &me->descriptor.executableMetadata);
}
