#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/HID.h>

#include "menu.h"
#include "gfx.h"

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

void addMenuEntry(menu_s* m, menuEntry_s* me)
{
	if(!m || !me)return;

	me->next=m->entries;
	m->entries=me;
	m->numEntries++;
}

void addMenuEntryCopy(menu_s* m, menuEntry_s* me)
{
	if(!m || !me)return;

	menuEntry_s* me2=malloc(sizeof(menuEntry_s));
	if(!me2)return;

	memcpy(me2, me, sizeof(menuEntry_s));
	
	addMenuEntry(m, me2);
}

void createMenuEntry(menu_s* m, char* execPath, char* name, char* description, u8* iconData)
{
	if(!m || !name || !description || !iconData)return;

	menuEntry_s* me=malloc(sizeof(menuEntry_s));
	if(!me)return;

	initMenuEntry(me, execPath, name, description, iconData);
	
	addMenuEntry(m, me);
}

menuEntry_s* getMenuEntry(menu_s* m, u16 n)
{
	if(!m || n>=m->numEntries)return NULL;
	menuEntry_s* me=m->entries;
	while(n && me){me=me->next; n--;}
	return me;
}

//return true when we're ready to boot something
//(TEMP ?)
bool updateMenu(menu_s* m)
{
	if(!m)return false;

	//controls
	s8 move=0;

	if(hidKeysDown()&KEY_DOWN)move++;
	if(hidKeysDown()&KEY_UP)move--;
	if(hidKeysDown()&KEY_RIGHT)move+=4;
	if(hidKeysDown()&KEY_LEFT)move-=4;

	if(move+m->selectedEntry<0)m->selectedEntry=0;
	else if(move+m->selectedEntry>=m->numEntries)m->selectedEntry=m->numEntries-1;
	else m->selectedEntry+=move;

	if(hidKeysDown()&KEY_A)return true;

	//scrolling code
	s32 target=intToFpt(getEntryLocation(m, m->selectedEntry));

	if(target>intToFpt(240-ENTRY_WIDTH) || (m->selectedEntry==0 && m->numEntries>3))
		m->scrollVelocity+=(intToFpt(240-ENTRY_WIDTH)-target)/SCROLLING_SPEED;
	if(target<0 || (m->selectedEntry==m->numEntries-1 && m->numEntries>3))
		m->scrollVelocity+=(intToFpt(0)-target)/SCROLLING_SPEED;

	m->scrollLocation+=m->scrollVelocity;
	m->scrollVelocity=(m->scrollVelocity*3)/4;

	return false;
}

void initEmptyMenuEntry(menuEntry_s* me)
{
	if(!me)return;

	me->name[0]=0x00;
	me->description[0]=0x00;
	me->executablePath[0]=0x00;

	me->next=NULL;
}

void initMenuEntry(menuEntry_s* me, char* execPath, char* name, char* description, u8* iconData)
{
	if(!me)return;

	initEmptyMenuEntry(me);

	strncpy(me->executablePath, execPath, ENTRY_PATHLENGTH);
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
	gfxDrawText(top, me->executablePath, x+8+ENTRY_ICON_WIDTH-4-16, y+8+ENTRY_ICON_HEIGHT+8);
}
