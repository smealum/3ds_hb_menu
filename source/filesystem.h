#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <3ds/types.h>
#include "menu.h"

void initFilesystem(void);
void exitFilesystem(void);

void addDirectoryToMenu(menu_s* m, char* path);
void scanHomebrewDirectory(menu_s* m, char* path);

#endif
