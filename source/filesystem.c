#include <stdio.h>
#include <string.h>
#include <3ds.h>

#include "installerIcon_bin.h"
#include "folderIcon_bin.h"

#include "filesystem.h"
#include "smdh.h"
#include "utils.h"

static char cwd[1024] = "/3ds/";

FS_archive sdmcArchive;

// File header
#define _3DSX_MAGIC 0x58534433 // '3DSX'
typedef struct
{
	u32 magic;
	u16 headerSize, relocHdrSize;
	u32 formatVer;
	u32 flags;

	// Sizes of the code, rodata and data segments +
	// size of the BSS section (uninitialized latter half of the data segment)
	u32 codeSegSize, rodataSegSize, dataSegSize, bssSize;
	// offset and size of smdh
	u32 smdhOffset, smdhSize;
	// offset to filesystem
	u32 fsOffset;
} _3DSX_Header;

void initFilesystem(void)
{
	fsInit();
	sdmcInit();
}

void exitFilesystem(void)
{
	sdmcExit();
	fsExit();
}

void openSDArchive()
{
	sdmcArchive=(FS_archive){0x00000009, (FS_path){PATH_EMPTY, 1, (u8*)""}};
	FSUSER_OpenArchive(NULL, &sdmcArchive);
}

void closeSDArchive()
{
	FSUSER_CloseArchive(NULL, &sdmcArchive);
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

static void loadSmdh(menuEntry_s* entry, const char* path)
{
	static char smdhPath[1024];
	char *p;

	memset(smdhPath, 0, sizeof(smdhPath));
	strncpy(smdhPath, path, sizeof(smdhPath));

	for(p = smdhPath + sizeof(smdhPath)-1; p > smdhPath; --p) {
		if(*p == '.') {
			/* this should always be true */
			if(strcmp(p, ".3dsx") == 0) {
				static smdh_s smdh;

				u32 bytesRead;
				Result ret;
				Handle fileHandle;
				bool gotsmdh = false;

				_3DSX_Header header;

				// first check for embedded smdh
				ret = FSUSER_OpenFile(NULL, &fileHandle, sdmcArchive, FS_makePath(PATH_CHAR, path), FS_OPEN_READ, FS_ATTRIBUTE_NONE);
				if (ret == 0)
				{
					ret=FSFILE_Read(fileHandle, &bytesRead, 0x0, &header, sizeof(header));
					if (ret == 0 && bytesRead == sizeof(header))
					{
						if (header.headerSize >= 44 )
						{
							ret=FSFILE_Read(fileHandle, &bytesRead, header.smdhOffset, &smdh, sizeof(smdh));
							if (ret == 0 && bytesRead == sizeof(smdh)) gotsmdh = true;
						}
					}
					FSFILE_Close(fileHandle);
				}

				if (!gotsmdh) {
					strcpy(p, ".smdh");
					if(fileExists(smdhPath, &sdmcArchive)) {
						if(!loadFile(smdhPath, &smdh, &sdmcArchive, sizeof(smdh))) gotsmdh = true;
					}
				}

				if (gotsmdh) extractSmdhData(&smdh, entry->name, entry->description, entry->author, entry->iconData);

			}
		}
	}
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

void addFileToMenu(menu_s* m, char* execPath)
{
	if(!m || !execPath)return;

	static menuEntry_s tmpEntry;

	if(!fileExists(execPath, &sdmcArchive))return;

	int i, l=-1; for(i=0; execPath[i]; i++) if(execPath[i]=='/')l=i;

	initMenuEntry(&tmpEntry, execPath, &execPath[l+1], execPath, "Unknown publisher", (u8*)installerIcon_bin, MENU_ENTRY_FILE);

	loadSmdh(&tmpEntry, execPath);

	addMenuEntryCopy(m, &tmpEntry);
}

void addDirectoryToMenu(menu_s* m, char* path)
{
	if(!m || !path)return;

	static menuEntry_s tmpEntry;

	int i, l=-1; for(i=0; path[i]; i++) if(path[i]=='/')l=i;

	initMenuEntry(&tmpEntry, path, &path[l+1], path, "", (u8*)folderIcon_bin, MENU_ENTRY_FOLDER);

	addMenuEntryCopy(m, &tmpEntry);
}

void scanHomebrewDirectory(menu_s* m)
{
	Handle dirHandle;
	FS_path dirPath=FS_makePath(PATH_CHAR, cwd);
	FSUSER_OpenDirectory(NULL, &dirHandle, sdmcArchive, dirPath);
	
	static char fullPath[1024];
	u32 entriesRead;
	do
	{
		static FS_dirent entry;
		memset(&entry,0,sizeof(FS_dirent));
		entriesRead=0;
		FSDIR_Read(dirHandle, &entriesRead, 1, &entry);
		if(entriesRead)
		{
			strncpy(fullPath, cwd, 1024);
			int n=strlen(fullPath);
			unicodeToChar(&fullPath[n], entry.name, 1024-n);
			if(entry.isDirectory) //directories
			{
				addDirectoryToMenu(m, fullPath);
			}else{ //stray executables
				n=strlen(fullPath);
				if(n>5 && !strcmp(".3dsx", &fullPath[n-5]))addFileToMenu(m, fullPath);
			}
		}
	}while(entriesRead);

	FSDIR_Close(dirHandle);

	sortMenu(m);
}

void changeDirectory(const char* path)
{
	if(strcmp(path, "..") == 0) {
		char *p = cwd + strlen(cwd)-2;
		while(p > cwd && *p != '/') *p-- = 0;
	}
	else {
		strncpy(cwd, path, sizeof(cwd));
		strcat(cwd, "/");
	}
}

void printDirectory(void)
{
	gfxDrawText(GFX_TOP, GFX_LEFT, NULL, cwd, 10, 10);
}
