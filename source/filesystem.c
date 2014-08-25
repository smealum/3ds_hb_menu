#include <stdio.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/FS.h>
#include <3ds/svc.h>

#include "installerIcon_bin.h"

#include "filesystem.h"
#include "smdh.h"
#include "utils.h"

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

int loadFile(char* path, void* dst, FS_archive* archive, u64 maxSize)
{
	if(!path || !dst || !archive)return -1;

	u64 size;
	u32 bytesRead;
	Result ret;
	Handle fileHandle;

	ret=FSUSER_OpenFile(NULL, &fileHandle, *archive, FS_makePath(PATH_CHAR, path), FS_OPEN_READ, FS_ATTRIBUTE_NONE);
	if(ret!=0)return ret;

	ret=FSFILE_GetSize(fileHandle, &size);
	if(ret!=0)goto loadFileExit;
	if(size>maxSize){ret=-2; goto loadFileExit;}

	ret=FSFILE_Read(fileHandle, &bytesRead, 0x0, dst, size);
	if(ret!=0)goto loadFileExit;
	if(bytesRead<size){ret=-3; goto loadFileExit;}

	loadFileExit:
	FSFILE_Close(fileHandle);
	return ret;
}

bool fileExists(char* path, FS_archive* archive)
{
	if(!path || !archive)return false;

	Result ret;
	Handle fileHandle;

	ret=FSUSER_OpenFile(NULL, &fileHandle, *archive, FS_makePath(PATH_CHAR, path), FS_OPEN_READ, FS_ATTRIBUTE_NONE);
	if(ret!=0)return false;

	ret=FSFILE_Close(fileHandle);
	if(ret!=0)return false;

	return true;
}

extern int debugValues[4];

void addDirectoryToMenu(menu_s* m, char* path)
{
	static menuEntry_s tmpEntry;
	static smdh_s tmpSmdh;
	static char execPath[128];
	static char iconPath[128];

	snprintf(execPath, 128, "%s/boot.3dsx", path);
	if(!fileExists(execPath, &sdmcArchive))return;

	snprintf(iconPath, 128, "%s/icon.bin", path);
	int ret=loadFile(iconPath, &tmpSmdh, &sdmcArchive, sizeof(smdh_s));
	
	if(!ret)
	{
		initEmptyMenuEntry(&tmpEntry);
		ret=extractSmdhData(&tmpSmdh, tmpEntry.name, tmpEntry.description, tmpEntry.iconData);
		strncpy(tmpEntry.executablePath, execPath, ENTRY_PATHLENGTH);
	}

	if(ret)initMenuEntry(&tmpEntry, execPath, path, "", (u8*)installerIcon_bin);

	addMenuEntryCopy(m, &tmpEntry);
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
		static FS_dirent entry;
		memset(&entry,0,sizeof(FS_dirent));
		FSDIR_Read(dirHandle, &entriesRead, 1, &entry);
		if(entriesRead && entry.isDirectory) //only grab directories
		{
			static char fullPath[1024];
			strncpy(fullPath, path, 1024);
			int n=strlen(fullPath);
			unicodeToChar(&fullPath[n], entry.name, 1024-n);
			addDirectoryToMenu(m, fullPath);
		}
	}while(entriesRead);

	FSDIR_Close(dirHandle);
}
