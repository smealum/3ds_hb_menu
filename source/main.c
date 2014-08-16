#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctr/types.h>
#include <ctr/srv.h>
#include <ctr/APT.h>
#include <ctr/HID.h>
#include <ctr/GSP.h>
#include <ctr/GX.h>
#include <ctr/FS.h>
#include <ctr/svc.h>

#include "gfx.h"
#include "menu.h"
#include "background.h"
#include "controls.h"

#include "installerIcon_bin.h"

menu_s menu;

int debugValues[4];

void renderFrame()
{
	//background stuff
	drawBackground();

	//top screen stuff
	char str[256];
	sprintf(str, "hello %d %d %08X %08X\n", debugValues[0], debugValues[1], (unsigned int)debugValues[2], (unsigned int)debugValues[3]);
	gfxDrawText(true, str, 100, 100);

	//menu stuff
	drawMenu(&menu);
}

int main()
{
	initSrv();
	aptInit(APPID_APPLICATION);
	gfxInit();

	initControls();
	initBackground();

	static menuEntry_s entries[8];
	int i; for(i=0; i<8; i++)initMenuEntry(&entries[i], "Ninjhax installer", "Selecting this will install the payload to your gamecart !", (u8*)installerIcon_bin);
	initMenu(&menu,entries,8);

	APP_STATUS status;
	while((status=aptGetStatus())!=APP_EXITING)
	{
		updateControls();
		updateMenu(&menu);
		renderFrame();
		gfxFlushBuffers();
		gfxSwapBuffers();
		svc_sleepThread(16666666);
	}

	hidExit();
	gfxExit();
	aptExit();
	svc_exitProcess();
	return 0;
}
