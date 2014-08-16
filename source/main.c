#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctr/types.h>
#include <ctr/srv.h>
#include <ctr/APT.h>
#include <ctr/GSP.h>
#include <ctr/GX.h>
#include <ctr/HID.h>
#include <ctr/FS.h>
#include <ctr/svc.h>

#include "gfx.h"
#include "background.h"

void renderFrame()
{
	//background stuff
	drawBackground();

	//top screen stuff
	gfxDrawText(true, "hello", 100, 100);

	//sub screen stuff
	static u8 testSprite[48*48*3];
	memset(testSprite, 0x80, 48*48*3);
	gfxDrawSprite(false, testSprite, 48, 48, 100, 20);
}

int main()
{
	initSrv();
	
	aptInit(APPID_APPLICATION);

	gfxInit();

	hidInit(NULL);

	initBackground();

	APP_STATUS status;
	while((status=aptGetStatus())!=APP_EXITING)
	{
		u32 PAD=hidSharedMem[7];

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
