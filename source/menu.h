#ifndef MENU_H
#define MENU_H

#include <ctr/types.h>

#define ENTRY_NAMELENGTH (64)
#define ENTRY_DESCLENGTH (64)
#define ENTRY_ICON_WIDTH (48)
#define ENTRY_ICON_HEIGHT (48)
#define ENTRY_ICONSIZE (ENTRY_ICON_WIDTH*ENTRY_ICON_HEIGHT*3)

typedef struct
{
	char name[ENTRY_NAMELENGTH+1];
	char description[ENTRY_DESCLENGTH+1];
	u8 iconData[ENTRY_ICONSIZE];
}menuEntry_s;

//switch to a list ?
typedef struct
{
	menuEntry_s* entries;
	u16 numEntries;
	u16 selectedEntry;
	u32 scrollLocation;
}menu_s;

//menu stuff
void initMenu(menu_s* m, menuEntry_s* entries, u16 numEntries);
void drawMenu(menu_s* m);

//menu entry stuff
void initMenuEntry(menuEntry_s* me, char* name, char* description, u8* iconData);
void drawMenuEntry(menuEntry_s* me, bool top, u16 x, u16 y);

#endif
