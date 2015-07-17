#include <stdlib.h>
#include <string.h>
#include <3ds.h>

#include "text.h"
#include "menu.h"
#include "error.h"
#include "smdh.h"
#include "regionfree.h"
#include "regionfree_bin.h"

u8 roundLut[]={8, 5, 4, 3, 2, 1, 1, 1, 0};
u8 roundLut2[]={4, 3, 2, 1, 0};
u8 roundLut3[]={0, 1, 2};

#define SCROLLING_SPEED (16) //lower is faster

extern int debugValues[100]; //TEMP

menuEntry_s regionfreeEntry;

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

	if(regionFreeAvailable)
	{
		extractSmdhData((smdh_s*)regionfree_bin, regionfreeEntry.name, regionfreeEntry.description, regionfreeEntry.author, regionfreeEntry.iconData);
		strcpy(regionfreeEntry.executablePath, REGIONFREE_PATH);
		addMenuEntryCopy(m, &regionfreeEntry);
	}
}

static inline s16 getEntryLocationPx(menu_s* m, int px)
{
	return 240-px+fptToInt(m->scrollLocation);
}

static inline s16 getEntryLocation(menu_s* m, int n)
{
	return getEntryLocationPx(m, (n+1)*ENTRY_WIDTH);
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
		gfxDrawRectangle(GFX_BOTTOM, GFX_LEFT, color, 200-scrollBarTotalSize-m->scrollBarPos+38, 308, m->currentScrollBarSize, 7);
		for(i=0; i<3; i++)gfxDrawRectangle(GFX_BOTTOM, GFX_LEFT, color, 200-scrollBarTotalSize-m->scrollBarPos+38+m->currentScrollBarSize+i, 308+roundLut3[i], 1, 7-2*roundLut3[i]);
}

void drawMenu(menu_s* m)
{
	if(!m)return;

	if(!m->numEntries)
	{
		drawError(GFX_BOTTOM,
			"Error",
			"    It seems you don't have any homebrew applications installed on your\n"
			"SD card.\n"
			"    Please take out your SD card, create a folder named \"3ds\" at the root of\n"
			"your card and place homebrew there.\n"
			"    Then, simply insert your SD card back into your 3DS !\n"
			"    The homebrew launcher will take it from there.",
			0);
	}else{
		menuEntry_s* me=m->entries;
		int i=0;
		int h=0;
		while(me)
		{
			h+=drawMenuEntry(me, GFX_BOTTOM, getEntryLocationPx(m,h), 9, i==m->selectedEntry);
			me=me->next;
			i++;
		}
	}

	drawScrollBar(m);
}

void addMenuEntry(menu_s* m, menuEntry_s* me)
{
	if(!m || !me)return;

	// add to the end of the list
	menuEntry_s** l = &m->entries;
	while(*l)l=&(*l)->next;
	*l = me;
	me->next = NULL;
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

void clearMenuEntries(menu_s* m)
{
	if(!m)return;

	m->selectedEntry=0;
	drawScrollBar(m);

	menuEntry_s* me = m->entries;
	menuEntry_s* temp = NULL;
	while(me)
	{
		temp=me->next;
		me->next = NULL;
		free(me);
		me = temp;
	}

	m->numEntries = 0;
	m->entries = NULL;

	if(regionFreeAvailable)
	{
		// should always be available
		addMenuEntryCopy(m, &regionfreeEntry);
	}
}

void createMenuEntry(menu_s* m, char* execPath, char* name, char* description, char* author, u8* iconData)
{
	if(!m || !name || !description || !iconData)return;

	menuEntry_s* me=malloc(sizeof(menuEntry_s));
	if(!me)return;

	initMenuEntry(me, execPath, name, description, author, iconData);
	
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
	if(!m->numEntries)return false;

	//controls
	s8 move=0;
	circlePosition cstick;
	touchPosition touch;
	hidCstickRead(&cstick);
	hidTouchRead(&touch);
	
	cstick.dy=(abs(cstick.dy)<5)?0:cstick.dy;

	if(hidKeysDown()&KEY_DOWN)move++;
	if(hidKeysDown()&KEY_UP)move--;
	if(hidKeysDown()&KEY_RIGHT)move+=4;
	if(hidKeysDown()&KEY_LEFT)move-=4;

	u16 oldEntry=m->selectedEntry;

	if(hidKeysDown()&KEY_TOUCH)
	{
		m->touchTimer=0;
		m->firstTouch=touch;
	}else if((hidKeysUp()&KEY_TOUCH) && m->touchTimer<30 && abs(m->firstTouch.px-m->previousTouch.px)+abs(m->firstTouch.py-m->previousTouch.py)<12){
		menuEntry_s* me=m->entries;
		int i=0;
		int p=0;
		while(me)
		{
			int h=(i==m->selectedEntry)?ENTRY_WIDTH_SELECTED:ENTRY_WIDTH;
			if((240-m->previousTouch.py)>=getEntryLocationPx(m,p)-h && (240-m->previousTouch.py)<getEntryLocationPx(m,p))break;
			p+=h;
			me=me->next;
			i++;
		}
		if(m->selectedEntry==i)return true;
		else m->selectedEntry=i;
	}else if(hidKeysHeld()&KEY_TOUCH){
		//condition to make sure previousTouch is valid
		cstick.dy+=(touch.py-m->previousTouch.py)*16;
		m->touchTimer++;
	}
	if(move+m->selectedEntry<0)m->selectedEntry=0;
	else if(move+m->selectedEntry>=m->numEntries)m->selectedEntry=m->numEntries-1;
	else m->selectedEntry+=move;

	if(m->selectedEntry!=oldEntry)m->atEquilibrium=false;

	if(hidKeysDown()&KEY_A)return true;

	m->previousTouch=touch;

	//scrolling code
	const int maxScroll=240-(m->numEntries)*ENTRY_WIDTH; //cf getEntryLocation

	if(!m->atEquilibrium)
	{
		m->scrollTarget=intToFpt(getEntryLocation(m, m->selectedEntry));
		if(m->scrollTarget>intToFpt(240-ENTRY_WIDTH) || (m->selectedEntry==0 && m->numEntries>3))
			m->scrollVelocity+=(intToFpt(240-ENTRY_WIDTH)-m->scrollTarget)/SCROLLING_SPEED;
		if(m->scrollTarget<0 || (m->selectedEntry==m->numEntries-1 && m->numEntries>3))
			m->scrollVelocity+=(intToFpt(0)-m->scrollTarget)/SCROLLING_SPEED;
	}else if(m->numEntries>3){
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

	m->scrollBarSize=40; //TEMP : make it adaptive to number of menu entries ?
	m->scrollBarPos=-fptToInt(m->scrollLocation*(200-m->scrollBarSize))/maxScroll;
	if(m->scrollBarPos<0)
	{
		m->currentScrollBarSize=m->scrollBarSize+m->scrollBarPos;
		if(m->currentScrollBarSize<10)m->currentScrollBarSize=10;
		m->scrollBarPos=m->currentScrollBarSize-m->scrollBarSize;
	}else if(m->scrollBarPos>=200-m->scrollBarSize)
	{
		m->currentScrollBarSize=-(m->scrollBarPos-200);
		if(m->currentScrollBarSize<10)m->currentScrollBarSize=10;
		debugValues[3]=m->scrollBarPos-200;
		m->scrollBarPos=200-m->scrollBarSize;
	}else m->currentScrollBarSize=m->scrollBarSize;

	if(!m->scrollVelocity)m->atEquilibrium=true;

	// debugValues[0]=m->scrollLocation;
	// debugValues[1]=m->scrollTarget;
	// debugValues[1]=fptToInt(m->scrollLocation);
	// debugValues[2]=intToFpt(maxScroll);
	// debugValues[3]=maxScroll;

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

void initMenuEntry(menuEntry_s* me, char* execPath, char* name, char* description, char* author, u8* iconData)
{
	if(!me)return;

	initEmptyMenuEntry(me);

	strncpy(me->executablePath, execPath, ENTRY_PATHLENGTH);
	strncpy(me->name, name, ENTRY_NAMELENGTH);
	strncpy(me->description, description, ENTRY_DESCLENGTH);
	strncpy(me->author, author, ENTRY_AUTHORLENGTH);
	if(iconData)memcpy(me->iconData, iconData, ENTRY_ICONSIZE);

	initMetadata(&me->metadata);
}

int drawMenuEntry(menuEntry_s* me, gfxScreen_t screen, u16 x, u16 y, bool selected)
{
	if(!me)return 0;
	int i;

	//TODO : proper template sort of thing ?
	//this is all hardcoded and horrible

	const int totalWidth=selected?ENTRY_WIDTH_SELECTED:ENTRY_WIDTH;
	const int actualWidth=(selected?ENTRY_FWIDTH_SELECTED:ENTRY_FWIDTH);
	const int widthOffset=actualWidth-ENTRY_FWIDTH;
	const int actualHeight=selected?ENTRY_HEIGHT_SELECTED:ENTRY_HEIGHT;
	if(selected)y-=ENTRY_HEIGHT_SELECTED-ENTRY_HEIGHT;
	x-=ENTRY_WIDTH+widthOffset;

	//drop shadow
	if(selected)
	{
		const int sw=4;
		const int sx=x-sw;
		for(i=0; i<9; i++)gfxDrawRectangle(screen, GFX_LEFT, ENTRY_BGCOLOR_SHADOW, sx+roundLut[i], y+i, sw, 1);
		gfxDrawRectangle(screen, GFX_LEFT, ENTRY_BGCOLOR_SHADOW, sx, y+9, sw, actualHeight-9*2);
		for(i=0; i<9; i++)gfxDrawRectangle(screen, GFX_LEFT, ENTRY_BGCOLOR_SHADOW, sx+roundLut[i], y+actualHeight-1-i, sw, 1);
	}

	//main frame
	for(i=0; i<9; i++)gfxDrawRectangle(screen, GFX_LEFT, ENTRY_BGCOLOR, x-widthOffset+roundLut[i], y+i, actualWidth-roundLut[i]*2, 1);
	gfxDrawRectangle(screen, GFX_LEFT, ENTRY_BGCOLOR, x-widthOffset, y+9, actualWidth, actualHeight-9*2);
	for(i=0; i<9; i++)gfxDrawRectangle(screen, GFX_LEFT, ENTRY_BGCOLOR, x-widthOffset+roundLut[i], y+actualHeight-1-i, actualWidth-roundLut[i]*2, 1);

	//icon frame
	u8 colorIcon[]={225, 225, 225};
	for(i=0; i<5; i++)gfxDrawRectangle(screen, GFX_LEFT, colorIcon, x+3+roundLut2[i], y+4+i, 56-roundLut2[i]*2, 1);
	gfxDrawRectangle(screen, GFX_LEFT, colorIcon, x+3, y+9, 56, 46);
	for(i=0; i<5; i++)gfxDrawRectangle(screen, GFX_LEFT, colorIcon, x+3+roundLut2[i], y+4+56-1-i, 56-roundLut2[i]*2, 1);

	//app specific stuff
	gfxDrawSprite(screen, GFX_LEFT, me->iconData, ENTRY_ICON_WIDTH, ENTRY_ICON_HEIGHT, x+7, y+8);
	gfxDrawTextN(screen, GFX_LEFT, &fontTitle, me->name, ENTRY_NAMELENGTH, x+38, y+66);
	gfxDrawTextN(screen, GFX_LEFT, &fontDescription, me->description, 56, x+26, y+70);
	if(strlen(me->description) > 56 * 1)
	{
		gfxDrawTextN(screen, GFX_LEFT, &fontDescription, me->description + 56, 56, x+18, y+70);
	}
	else if(strlen(me->description) > 56 * 2)
	{
		gfxDrawTextN(screen, GFX_LEFT, &fontDescription, me->description + 56 * 1, 56, x+18, y+70);
		gfxDrawTextN(screen, GFX_LEFT, &fontDescription, me->description + 56 * 2, 56, x+10, y+70);
	}
	gfxDrawTextN(screen, GFX_LEFT, &fontDescription, me->author, ENTRY_AUTHORLENGTH, x+4, y+ENTRY_HEIGHT-getStringLength(&fontDescription, me->author)-10);

	return totalWidth;
}
