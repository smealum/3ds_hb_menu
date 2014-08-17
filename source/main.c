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
#include "filesystem.h"

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
	initFilesystem();
	gfxInit();

	initControls();
	initBackground();
	
	initMenu(&menu);
	scanHomebrewDirectory(&menu, "/3ds/");

	APP_STATUS status;
	while((status=aptGetStatus())!=APP_EXITING)
	{
		updateControls();
		updateMenu(&menu);
		renderFrame();
		gfxFlushBuffers();
		gfxSwapBuffers();
		// svcSleepThread(16666666);
		svcSleepThread(8333333);
	}

	hidExit();
	gfxExit();
	exitFilesystem();
	aptExit();
	svcExitProcess();
	return 0;
}
