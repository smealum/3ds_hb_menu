#include <string.h>
#include <3ds/types.h>
#include <3ds/FS.h>
#include <3ds/svc.h>

#include "installerIcon_bin.h"

#include "filesystem.h"

FS_archive sdmcArchive;

void initFilesystem(void)
{
	fsInit();
	sdmcArchive=(FS_archive){0x00000009, (FS_path){PATH_EMPTY, 1, (u8*)""}};
	FSUSER_OpenArchive(NULL, &sdmcArchive);
}

void exitFilesystem(void)
{
	FSUSER_CloseArchive(NULL, &sdmcArchive);
	fsExit();
}

void unicodeToChar(char* dst, u16* src, int max)
{
	if(!src || !dst)return;
	int n=0;
	while(*src && n<max-1){*(dst++)=(*(src++))&0xFF;n++;}
	*dst=0x00;
}

void addDirectoryToMenu(menu_s* m, char* path)
{
	addMenuEntry(m, path, "test !", (u8*)installerIcon_bin);
}

void scanHomebrewDirectory(menu_s* m, char* path)
{
	if(!path)return;

	Handle dirHandle;
	FS_path dirPath=FS_makePath(PATH_CHAR, path);
	FSUSER_OpenDirectory(NULL, &dirHandle, sdmcArchive, dirPath);
	
	u32 entriesRead=0;
	do
	{
		u16 entryBuffer[512];
		memset(entryBuffer,0,1024);
		FSDIR_Read(dirHandle, &entriesRead, 1, (u16*)entryBuffer);
		if(entriesRead && entryBuffer[0x10E]) //only grab directories
		{
			static char fullPath[1024];
			strncpy(fullPath, path, 1024);
			int n=strlen(fullPath);
			unicodeToChar(&fullPath[n], entryBuffer, 1024-n);
			addDirectoryToMenu(m, fullPath);
		}
	}while(entriesRead);

	svcCloseHandle(dirHandle);
}
