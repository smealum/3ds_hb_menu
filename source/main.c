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

int main()
{
	if(initSrv())*(u32*)NULL=0xBABE0001;
	
	aptInit(APPID_APPLICATION);

	gfxInit();

	hidInit(NULL);

	APP_STATUS status;
	while((status=aptGetStatus())!=APP_EXITING)
	{
		u32 PAD=hidSharedMem[7];

		gfxRenderFrame();
		gfxSwapBuffers();
		svc_sleepThread(16666666);
	}

	hidExit();
	gfxExit();
	aptExit();
	svc_exitProcess();
	return 0;
}
