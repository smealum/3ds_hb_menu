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

int debugValues[100];

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

//TEMP
extern Handle aptStatusMutex, aptStatusEvent;
extern Handle aptuHandle, aptLockHandle;
extern NS_APPID currentAppId;

void _aptExit()
{
	svcCloseHandle(aptStatusMutex);
	svcCloseHandle(aptLockHandle);
	svcCloseHandle(aptStatusEvent);
}

Result _aptInit(NS_APPID appID)
{
	Result ret=0;

	//initialize APT stuff, escape load screen
	srvGetServiceHandle(&aptuHandle, "APT:U");
	if((ret=APT_GetLockHandle(&aptuHandle, 0x0, &aptLockHandle)))return ret;
	svcCloseHandle(aptuHandle);

	currentAppId=appID;

	svcCreateEvent(&aptStatusEvent, 0);
	
	return 0;
}

int main()
{
	srvInit();
	_aptInit(APPID_APPLICATION);
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
		if(updateMenu(&menu))break;
		renderFrame();
		gfxFlushBuffers();
		gfxSwapBuffers();
		svcSleepThread(8333333);
	}

	// cleanup whatever we have to cleanup
	// TODO : call whatever needs to be called to free main heap
	exitControls();
	gfxExit();
	exitFilesystem();
	_aptExit();
	srvExit();

	//open file that we're going to boot up
	fsInit();
	Handle fileHandle;
	menuEntry_s* me=getMenuEntry(&menu, menu.selectedEntry); //TODO : check that it's not NULL ?
	debugValues[2]=FSUSER_OpenFileDirectly(NULL, &fileHandle, sdmcArchive, FS_makePath(PATH_CHAR, me->executablePath), FS_OPEN_READ, FS_ATTRIBUTE_NONE);
	fsExit();

	u32 blockAddr;
	svcControlMemory(&blockAddr, 0x08000000, 0x0, 0x1910000, MEMOP_FREE, 0x0);

	//jump to bootloader
	void (*callBootloader)(Handle h, Handle file)=(void*)0x000F0000;
	callBootloader(0x00000000, fileHandle);
	return 0;
}
