#include <stdio.h>
#include <string.h>
#include <3ds.h>

#include "installerIcon_bin.h"
#include "folderIcon_bin.h"

#include "filesystem.h"
#include "smdh.h"
#include "utils.h"

#define DEFAULT_DIRECTORY "/3ds"

static char cwd[1024] = DEFAULT_DIRECTORY "/";

FS_Archive sdmcArchive;

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
	sdmcArchive=(FS_Archive){0x00000009, (FS_Path){PATH_EMPTY, 1, (u8*)""}};
	FSUSER_OpenArchive(&sdmcArchive);
}

void closeSDArchive()
{
	FSUSER_CloseArchive(&sdmcArchive);
}

int loadFile(char* path, void* dst, FS_Archive* archive, u64 maxSize)
{
	if(!path || !dst || !archive)return -1;

	u64 size;
	u32 bytesRead;
	Result ret;
	Handle fileHandle;

	ret=FSUSER_OpenFile(&fileHandle, *archive, fsMakePath(PATH_ASCII, path), FS_OPEN_READ, 0);
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

static void loadSmdh(menuEntry_s* entry, const char* path, bool isAppFolder)
{
	static char smdhPath[1024];
	char *p;

	memset(smdhPath, 0, sizeof(smdhPath));
	strncpy(smdhPath, path, sizeof(smdhPath));
	char* fnamep = NULL;

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
				ret = FSUSER_OpenFile(&fileHandle, sdmcArchive, fsMakePath(PATH_ASCII, path), FS_OPEN_READ, 0);
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
				if (!gotsmdh) {
					strcpy(p, ".icn");
					if(fileExists(smdhPath, &sdmcArchive)) {
						if(!loadFile(smdhPath, &smdh, &sdmcArchive, sizeof(smdh))) gotsmdh = true;
					}
				}
				if (!gotsmdh && fnamep && isAppFolder) {
					strcpy(fnamep, "icon.smdh");
					if(fileExists(smdhPath, &sdmcArchive)) {
						if(!loadFile(smdhPath, &smdh, &sdmcArchive, sizeof(smdh))) gotsmdh = true;
					}
				}
				if (!gotsmdh && fnamep && isAppFolder) {
					strcpy(fnamep, "icon.icn");
					if(fileExists(smdhPath, &sdmcArchive)) {
						if(!loadFile(smdhPath, &smdh, &sdmcArchive, sizeof(smdh))) gotsmdh = true;
					}
				}

				if (gotsmdh) extractSmdhData(&smdh, entry->name, entry->description, entry->author, entry->iconData);

			}
		} else if(*p == '/') {
			fnamep = p+1;
		}
	}
}

bool fileExists(char* path, FS_Archive* archive)
{
	if(!path || !archive)return false;

	Result ret;
	Handle fileHandle;

	ret=FSUSER_OpenFile(&fileHandle, *archive, fsMakePath(PATH_ASCII, path), FS_OPEN_READ, 0);
	if(ret!=0)return false;

	ret=FSFILE_Close(fileHandle);
	if(ret!=0)return false;

	return true;
}

extern int debugValues[100];

void addExecutableToMenu(menu_s* m, char* execPath)
{
	if(!m || !execPath)return;

	static menuEntry_s tmpEntry;

	if(!fileExists(execPath, &sdmcArchive))return;

	int i, l=-1; for(i=0; execPath[i]; i++) if(execPath[i]=='/')l=i;

	initMenuEntry(&tmpEntry, execPath, &execPath[l+1], execPath, "Unknown publisher", (u8*)installerIcon_bin, MENU_ENTRY_FILE);

	loadSmdh(&tmpEntry, execPath, false);

	static char xmlPath[128];
	snprintf(xmlPath, 128, "%s", execPath);
	l = strlen(xmlPath);
	xmlPath[l-1] = 0;
	xmlPath[l-2] = 'l';
	xmlPath[l-3] = 'm';
	xmlPath[l-4] = 'x';
	if(fileExists(xmlPath, &sdmcArchive)) loadDescriptor(&tmpEntry.descriptor, xmlPath);

	addMenuEntryCopy(m, &tmpEntry);
}

void addDirectoryToMenu(menu_s* m, char* path)
{
	if(!m || !path)return;

	static menuEntry_s tmpEntry;
	static char execPath[128];
	static char xmlPath[128];

	int i, l=-1; for(i=0; path[i]; i++) if(path[i]=='/') l=i;

	// Check for old-style application folder
	bool exists = false;
	if (strcmp(path, DEFAULT_DIRECTORY)==0)
	{
		snprintf(execPath, 128, "%s/boot.3dsx", path);
		exists = fileExists(execPath, &sdmcArchive);
	}
	if (!exists)
	{
		snprintf(execPath, 128, "%s/%s.3dsx", path, &path[l+1]);
		exists = fileExists(execPath, &sdmcArchive);
	}
	if (exists)
	{
		// Add the application
		initMenuEntry(&tmpEntry, execPath, &path[l+1], execPath, "Unknown publisher", (u8*)installerIcon_bin, MENU_ENTRY_FILE);

		// Load the icon
		loadSmdh(&tmpEntry, execPath, true);

		// Load the descriptor
		snprintf(xmlPath, 128, "%s/descriptor.xml", path);
		if(!fileExists(xmlPath, &sdmcArchive))snprintf(xmlPath, 128, "%s/%s.xml", path, &path[l+1]);
		loadDescriptor(&tmpEntry.descriptor, xmlPath);
	} else
	{
		// This is a normal folder
		initMenuEntry(&tmpEntry, path, &path[l+1], path, "", (u8*)folderIcon_bin, MENU_ENTRY_FOLDER);
	}

	addMenuEntryCopy(m, &tmpEntry);
}

// should go in menu.c ?
void createMenuEntryShortcut(menu_s* m, shortcut_s* s)
{
	if(!m || !s)return;

	static menuEntry_s tmpEntry;
	static smdh_s tmpSmdh;

	char* execPath = s->executable;

	if(!fileExists(execPath, &sdmcArchive))return;

	int i, l=-1; for(i=0; execPath[i]; i++) if(execPath[i]=='/') l=i;

	char* iconPath = s->icon;
	int ret = loadFile(iconPath, &tmpSmdh, &sdmcArchive, sizeof(smdh_s));

	if(!ret)
	{
		initEmptyMenuEntry(&tmpEntry);
		ret = extractSmdhData(&tmpSmdh, tmpEntry.name, tmpEntry.description, tmpEntry.author, tmpEntry.iconData);
		strncpy(tmpEntry.executablePath, execPath, ENTRY_PATHLENGTH);
	}

	if(ret) initMenuEntry(&tmpEntry, execPath, &execPath[l+1], execPath, "Unknown publisher", (u8*)installerIcon_bin, MENU_ENTRY_FILE);
	
	if(s->name) strncpy(tmpEntry.name, s->name, ENTRY_NAMELENGTH);
	if(s->description) strncpy(tmpEntry.description, s->description, ENTRY_DESCLENGTH);
	if(s->author) strncpy(tmpEntry.author, s->author, ENTRY_AUTHORLENGTH);

	if(s->arg)
	{
		strncpy(tmpEntry.arg, s->arg, ENTRY_ARGLENGTH);
	}

	if(fileExists(s->descriptor, &sdmcArchive)) loadDescriptor(&tmpEntry.descriptor, s->descriptor);

	addMenuEntryCopyAt(m, &tmpEntry, 1);
}

void addShortcutToMenu(menu_s* m, char* shortcutPath)
{
	if(!m || !shortcutPath)return;

	static shortcut_s tmpShortcut;

	Result ret = createShortcut(&tmpShortcut, shortcutPath);
	if(!ret) createMenuEntryShortcut(m, &tmpShortcut);

	freeShortcut(&tmpShortcut);
}

void scanHomebrewDirectory(menu_s* m)
{
	Handle dirHandle;
	FS_Path dirPath=fsMakePath(PATH_ASCII, cwd);
	FSUSER_OpenDirectory(&dirHandle, sdmcArchive, dirPath);
	
	static char fullPath[1024];
	u32 entriesRead;
	do
	{
		static FS_DirectoryEntry entry;
		memset(&entry,0,sizeof(FS_DirectoryEntry));
		entriesRead=0;
		FSDIR_Read(dirHandle, &entriesRead, 1, &entry);
		if(entriesRead)
		{
			strncpy(fullPath, cwd, 1024);
			int n=strlen(fullPath);
			unicodeToChar(&fullPath[n], entry.name, 1024-n);
			if(entry.attributes & FS_ATTRIBUTE_DIRECTORY) //directories
			{
				addDirectoryToMenu(m, fullPath);
			}else{ //stray executables and shortcuts
				n=strlen(fullPath);
				if(n>5 && !strcmp(".3dsx", &fullPath[n-5]))addExecutableToMenu(m, fullPath);
				if(n>4 && !strcmp(".xml", &fullPath[n-4]))addShortcutToMenu(m, fullPath);
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
