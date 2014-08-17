#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <3ds/types.h>
#include "menu.h"

extern FS_archive sdmcArchive;

//system stuff
void initFilesystem(void);
void exitFilesystem(void);

//general fs stuff
int loadFile(char* path, void* dst, FS_archive* archive, u64 maxSize);
bool fileExists(char* path, FS_archive* archive);

//menu fs stuff
void addDirectoryToMenu(menu_s* m, char* path);
void scanHomebrewDirectory(menu_s* m, char* path);

#endif
