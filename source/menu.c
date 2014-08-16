#include <stdlib.h>
#include <string.h>

#include "menu.h"
#include "gfx.h"

void initMenu(menu_s* m, menuEntry_s* entries, u16 numEntries)
{
	if(!m)return;

	m->entries=malloc(sizeof(menuEntry_s)*numEntries);
	memcpy(m->entries, entries, sizeof(menuEntry_s)*numEntries);
	m->numEntries=numEntries;
	m->selectedEntry=0;
	m->scrollLocation=0;
}

void drawMenu(menu_s* m)
{
	if(!m)return;

	int i;
	for(i=0; i<m->numEntries; i++)
	{
		drawMenuEntry(&m->entries[i], false, i*64, 16);
	}
}

void initMenuEntry(menuEntry_s* me, char* name, char* description, u8* iconData)
{
	if(!me)return;

	strncpy(me->name, name, ENTRY_NAMELENGTH);
	strncpy(me->description, description, ENTRY_DESCLENGTH);
	memcpy(me->iconData, iconData, ENTRY_ICONSIZE);
}

void drawMenuEntry(menuEntry_s* me, bool top, u16 x, u16 y)
{
	if(!me)return;

	//TODO : proper template sort of thing ?
	gfxDrawSprite(top, me->iconData, ENTRY_ICON_WIDTH, ENTRY_ICON_HEIGHT, x+8, y+8);
	gfxDrawText(top, me->name, x+16, y+8+ENTRY_ICON_HEIGHT);
}
