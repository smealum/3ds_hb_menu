#pragma once
#include "gfx.h"

#define ENTRY_PATHLENGTH (128)
#define ENTRY_NAMELENGTH (64)
#define ENTRY_DESCLENGTH (128)
#define ENTRY_ICON_WIDTH (48)
#define ENTRY_ICON_HEIGHT (48)
#define ENTRY_ICONSIZE (ENTRY_ICON_WIDTH*ENTRY_ICON_HEIGHT*3)
#define ENTRY_WIDTH (64)

#define ENTRY_BGCOLOR (u8[]){221, 221, 231}
#define ENTRY_BGCOLOR_SELECTED (u8[]){150, 150, 160}

#define fptToInt(v) ((v)>>10)
#define intToFpt(v) ((v)<<10)

typedef struct menuEntry_s
{
	char executablePath[ENTRY_PATHLENGTH+1];
	char name[ENTRY_NAMELENGTH+1];
	char description[ENTRY_DESCLENGTH+1];
	u8 iconData[ENTRY_ICONSIZE];
	struct menuEntry_s* next;
}menuEntry_s;

typedef struct
{
	menuEntry_s* entries;
	u16 numEntries;
	u16 selectedEntry;
	s32 scrollLocation; //10 bit fixed point
	s32 scrollVelocity; //10 bit fixed point
}menu_s;

//menu stuff
void initMenu(menu_s* m);
void drawMenu(menu_s* m);
bool updateMenu(menu_s* m);
void addMenuEntry(menu_s* m, menuEntry_s* me);
void addMenuEntryCopy(menu_s* m, menuEntry_s* me);
void createMenuEntry(menu_s* m, char* execPath, char* name, char* description, u8* iconData);
menuEntry_s* getMenuEntry(menu_s* m, u16 n);

//menu entry stuff
void initEmptyMenuEntry(menuEntry_s* me);
void initMenuEntry(menuEntry_s* me, char* execPath, char* name, char* description, u8* iconData);
void drawMenuEntry(menuEntry_s* me, gfxScreen_t screen, u16 x, u16 y, bool selected);
