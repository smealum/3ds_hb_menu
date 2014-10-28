#include <stdlib.h>
#include <string.h>
#include <3ds.h>

#include "menu.h"

u8 roundLut[]={8, 5, 4, 3, 2, 1, 1, 1, 0};
u8 roundLut2[]={4, 3, 2, 1, 0};
u8 roundLut3[]={0, 1, 2};

#define SCROLLING_SPEED (16) //lower is faster

extern int debugValues[100]; //TEMP

void initMenu(menu_s* m)
{
	if(!m)return;

	m->entries=NULL;
	m->numEntries=0;
	m->selectedEntry=0;
	m->scrollLocation=0;
	m->scrollVelocity=0;
	m->scrollBarSize=0;
	m->scrollBarPos=0;
	m->scrollTarget=0;
	m->atEquilibrium=false;
}

static inline s16 getEntryLocation(menu_s* m, int n)
{
	return 240-(n+1)*ENTRY_WIDTH+fptToInt(m->scrollLocation);
}

void drawScrollBar(menu_s* m)
{
	if(!m)return;

	int scrollBarTotalSize=m->scrollBarSize+3*2;

	int i;
	// background scrollbar thing
		//35,308 - 200,7
		u8 colorBg[]={132, 224, 255};
		for(i=0; i<3; i++)gfxDrawRectangle(GFX_BOTTOM, GFX_LEFT, colorBg, 38-i, 308+roundLut3[i], 1, 7-2*roundLut3[i]);
		gfxDrawRectangle(GFX_BOTTOM, GFX_LEFT, colorBg, 38, 308, 194, 7);
		for(i=0; i<3; i++)gfxDrawRectangle(GFX_BOTTOM, GFX_LEFT, colorBg, 232+i, 308+roundLut3[i], 1, 7-2*roundLut3[i]);
	// actual scrollbar
		u8 color[]={255, 255, 255};
		for(i=0; i<3; i++)gfxDrawRectangle(GFX_BOTTOM, GFX_LEFT, color, 200-scrollBarTotalSize-m->scrollBarPos+38-i, 308+roundLut3[i], 1, 7-2*roundLut3[i]);
		gfxDrawRectangle(GFX_BOTTOM, GFX_LEFT, color, 200-scrollBarTotalSize-m->scrollBarPos+38, 308, m->scrollBarSize, 7);
		for(i=0; i<3; i++)gfxDrawRectangle(GFX_BOTTOM, GFX_LEFT, color, 200-scrollBarTotalSize-m->scrollBarPos+38+m->scrollBarSize+i, 308+roundLut3[i], 1, 7-2*roundLut3[i]);
}

void drawMenu(menu_s* m)
{
	if(!m)return;

	menuEntry_s* me=m->entries;
	int i=0;
	while(me)
	{
		drawMenuEntry(me, GFX_BOTTOM, getEntryLocation(m,i), 9, i==m->selectedEntry);
		me=me->next;
		i++;
	}

	drawScrollBar(m);
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
	circlePosition cstick;
	hidCstickRead(&cstick);

	if(hidKeysDown()&KEY_DOWN)move++;
	if(hidKeysDown()&KEY_UP)move--;
	if(hidKeysDown()&KEY_RIGHT)move+=4;
	if(hidKeysDown()&KEY_LEFT)move-=4;

	u16 oldEntry=m->selectedEntry;
	if(move+m->selectedEntry<0)m->selectedEntry=0;
	else if(move+m->selectedEntry>=m->numEntries)m->selectedEntry=m->numEntries-1;
	else m->selectedEntry+=move;

	if(m->selectedEntry!=oldEntry)m->atEquilibrium=false;

	if(hidKeysDown()&KEY_A)return true;

	//scrolling code
	const int maxScroll=240-(m->numEntries)*ENTRY_WIDTH; //cf getEntryLocation

	if(!m->atEquilibrium)
	{
		m->scrollTarget=intToFpt(getEntryLocation(m, m->selectedEntry));
		if(m->scrollTarget>intToFpt(240-ENTRY_WIDTH) || (m->selectedEntry==0 && m->numEntries>3))
			m->scrollVelocity+=(intToFpt(240-ENTRY_WIDTH)-m->scrollTarget)/SCROLLING_SPEED;
		if(m->scrollTarget<0 || (m->selectedEntry==m->numEntries-1 && m->numEntries>3))
			m->scrollVelocity+=(intToFpt(0)-m->scrollTarget)/SCROLLING_SPEED;
	}else{
		s32 val=-cstick.dy*16; // TODO : make it inversely proportional to numEntries ?
		if(m->scrollLocation>intToFpt(-maxScroll))
		{
			m->scrollVelocity+=(intToFpt(-maxScroll)-m->scrollLocation)/SCROLLING_SPEED;
			if(val<0)m->scrollVelocity+=val;
		}else if(m->scrollLocation<intToFpt(0)){
			m->scrollVelocity-=m->scrollLocation/SCROLLING_SPEED;
			if(val>0)m->scrollVelocity+=val;
		}else m->scrollVelocity+=val;
	}

	m->scrollLocation+=m->scrollVelocity;
	m->scrollVelocity=(m->scrollVelocity*3)/4;

	m->scrollBarSize=40; //TEMP : make it adaptive ?
	m->scrollBarPos=-fptToInt(m->scrollLocation*(200-m->scrollBarSize))/maxScroll;

	if(!m->scrollVelocity)m->atEquilibrium=true;

	debugValues[0]=m->scrollLocation;
	debugValues[1]=m->scrollTarget;
	debugValues[2]=intToFpt(maxScroll);
	debugValues[3]=maxScroll;

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

void drawMenuEntry(menuEntry_s* me, gfxScreen_t screen, u16 x, u16 y, bool selected)
{
	if(!me)return;
	int i;

	//TODO : proper template sort of thing ?
	//main frame
	for(i=0; i<9; i++)gfxDrawRectangle(screen, GFX_LEFT, selected?(ENTRY_BGCOLOR_SELECTED):(ENTRY_BGCOLOR), x+roundLut[i], y+i, 63-roundLut[i]*2, 1);
	gfxDrawRectangle(screen, GFX_LEFT, selected?(ENTRY_BGCOLOR_SELECTED):(ENTRY_BGCOLOR), x, y+9, 63, 276);
	for(i=0; i<9; i++)gfxDrawRectangle(screen, GFX_LEFT, selected?(ENTRY_BGCOLOR_SELECTED):(ENTRY_BGCOLOR), x+roundLut[i], y+294-1-i, 63-roundLut[i]*2, 1);

	//icon frame
	u8 colorIcon[]={225, 225, 225};
	for(i=0; i<5; i++)gfxDrawRectangle(screen, GFX_LEFT, colorIcon, x+3+roundLut2[i], y+4+i, 56-roundLut2[i]*2, 1);
	gfxDrawRectangle(screen, GFX_LEFT, colorIcon, x+3, y+9, 56, 46);
	for(i=0; i<5; i++)gfxDrawRectangle(screen, GFX_LEFT, colorIcon, x+3+roundLut2[i], y+4+56-1-i, 56-roundLut2[i]*2, 1);

	//app specific stuff
	gfxDrawSprite(screen, GFX_LEFT, me->iconData, ENTRY_ICON_WIDTH, ENTRY_ICON_HEIGHT, x+7, y+8);
	gfxDrawTextN(screen, GFX_LEFT, &fontTitle, me->name, 28, x+38, y+66);
	gfxDrawTextN(screen, GFX_LEFT, &fontDescription, me->executablePath, 28, x+26, y+66);
}
