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

void unicodeToChar(char* dst, u16* src, int max)
{
	if(!src || !dst)return;
	int n=0;
	while(*src && n<max-1){*(dst++)=(*(src++))&0xFF;n++;}
	*dst=0x00;
}

int main()
{
	srvInit();
	aptInit(APPID_APPLICATION);
	fsInit();
	gfxInit();

	initControls();
	initBackground();
	
	initMenu(&menu);

	Handle dirHandle;
	FS_path dirPath=FS_makePath(PATH_CHAR, "/");
	FS_archive sdmcArchive=(FS_archive){0x00000009, (FS_path){PATH_EMPTY, 1, (u8*)""}};
	debugValues[2]=FSUSER_OpenArchive(NULL, &sdmcArchive);
	debugValues[3]=FSUSER_OpenDirectory(NULL, &dirHandle, sdmcArchive, dirPath);
	
	u32 entriesRead=0;
	do
	{
		u32 entryBuffer[1024];
		memset(entryBuffer,0,1024);
		FSDIR_Read(dirHandle, &entriesRead, 1, (u16*)entryBuffer);
		if(entriesRead)
		{
			static char str[0x80];
			unicodeToChar(str, (u16*)entryBuffer, 0x80);
			addMenuEntry(&menu, str, "test !", (u8*)installerIcon_bin);
		}
	}while(entriesRead);

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
	fsExit();
	aptExit();
	svcExitProcess();
	return 0;
}
