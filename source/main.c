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

void renderFrame()
{
	//background stuff
	drawBackground();

	//top screen stuff
	gfxDrawText(true, "hello", 100, 100);

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

	menuEntry_s entry;
	initMenuEntry(&entry, "Exploit installer", "Selecting this will install the payload to your gamecart !", (u8*)installerIcon_bin);
	initMenu(&menu,&entry,1);

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
