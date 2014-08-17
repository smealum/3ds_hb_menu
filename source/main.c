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

#include "gfx.h"
#include "menu.h"
#include "background.h"
#include "controls.h"

#include "installerIcon_bin.h"
#include "logo_bin.h"

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
	gfxDrawSpriteAlpha(true, (u8*)logo_bin, 182, 245, 28, 80);

	//menu stuff
	drawMenu(&menu);
}

int main()
{
	srvInit();
	aptInit(APPID_APPLICATION);
	gfxInit();

	initControls();
	initBackground();

	static menuEntry_s entries[8];
	int i; for(i=0; i<8; i++)initMenuEntry(&entries[i], "Ninjhax installer", "Selecting this will install the payload to your gamecart !", (u8*)installerIcon_bin);
	initMenu(&menu,entries,8);

	srvGetServiceHandle((Handle*)&debugValues[2], "fs:USER");

	APP_STATUS status;
	while((status=aptGetStatus())!=APP_EXITING)
	{
		updateControls();
		updateMenu(&menu);
		renderFrame();
		gfxFlushBuffers();
		gfxSwapBuffers();
		svcSleepThread(16666666);
	}

	hidExit();
	gfxExit();
	aptExit();
	svcExitProcess();
	return 0;
}
