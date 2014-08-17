#include <stdlib.h>
#include <string.h>

#include "menu.h"
#include "gfx.h"
#include "controls.h"

#define SCROLLING_SPEED (16) //lower is faster

void initMenu(menu_s* m)
{
	if(!m)return;

	m->entries=NULL;
	m->numEntries=0;
	m->selectedEntry=0;
	m->scrollLocation=0;
	m->scrollVelocity=0;
}

static inline s16 getEntryLocation(menu_s* m, int n)
{
	return 240-(n+1)*ENTRY_WIDTH+fptToInt(m->scrollLocation);
}

void drawMenu(menu_s* m)
{
	if(!m)return;

	menuEntry_s* me=m->entries;
	int i=0;
	while(me)
	{
		drawMenuEntry(me, false, getEntryLocation(m,i), 16, i==m->selectedEntry);
		me=me->next;
		i++;
	}	
}

void addMenuEntry(menu_s* m, char* name, char* description, u8* iconData)
{
	if(!m || !name || !description || !iconData)return;

	menuEntry_s* me=malloc(sizeof(menuEntry_s));
	if(!me)return;

	initMenuEntry(me, name, description, iconData);
	me->next=m->entries;

	m->entries=me;
	m->numEntries++;
}

extern int debugValues[4];

void updateMenu(menu_s* m)
{
	if(!m)return;

	//controls
	s8 move=0;

	if(keysDown()&PAD_DOWN)move++;
	if(keysDown()&PAD_UP)move--;
	if(keysDown()&PAD_RIGHT)move+=4;
	if(keysDown()&PAD_LEFT)move-=4;

	if(move+m->selectedEntry<0)m->selectedEntry=0;
	else if(move+m->selectedEntry>=m->numEntries)m->selectedEntry=m->numEntries-1;
	else m->selectedEntry+=move;

	//scrolling code
	s32 target=intToFpt(getEntryLocation(m, m->selectedEntry));
	debugValues[0]=target;
	debugValues[1]=m->scrollLocation;

	if(target>intToFpt(240-ENTRY_WIDTH) || m->selectedEntry==0)m->scrollVelocity+=(intToFpt(240-ENTRY_WIDTH)-target)/SCROLLING_SPEED;
	if(target<0 || m->selectedEntry==m->numEntries-1)m->scrollVelocity+=(intToFpt(0)-target)/SCROLLING_SPEED;

	m->scrollLocation+=m->scrollVelocity;
	m->scrollVelocity=(m->scrollVelocity*3)/4;
}

void initMenuEntry(menuEntry_s* me, char* name, char* description, u8* iconData)
{
	if(!me)return;

	strncpy(me->name, name, ENTRY_NAMELENGTH);
	strncpy(me->description, description, ENTRY_DESCLENGTH);
	memcpy(me->iconData, iconData, ENTRY_ICONSIZE);
}

void drawMenuEntry(menuEntry_s* me, bool top, u16 x, u16 y, bool selected)
{
	if(!me)return;

	//TODO : proper template sort of thing ?
	gfxDrawRectangle(top, selected?ENTRY_BGCOLOR_SELECTED:ENTRY_BGCOLOR, x+2, y, ENTRY_WIDTH-4, 288);
	gfxDrawSprite(top, me->iconData, ENTRY_ICON_WIDTH, ENTRY_ICON_HEIGHT, x+8, y+8);
	gfxDrawText(top, me->name, x+8+ENTRY_ICON_WIDTH-4, y+8+ENTRY_ICON_HEIGHT+8);
}
