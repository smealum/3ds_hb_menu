#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <3ds.h>
#include "text.h"
#include "font_bin.h"

#include "font.h"

const u8* font=font_bin;

void drawCharacter(u8* fb, char c, u16 x, u16 y, u16 w, u16 h)
{
	if(c<32)return;
	if(x<0 || y<0 || x+CHAR_SIZE_X>=w || y+CHAR_SIZE_Y>=h)return;
	c-=32;
	u8* charData=(u8*)&font_bin[CHAR_SIZE_X*CHAR_SIZE_Y*c];
	fb+=(x*h+y)*3;
	int i, j;
	for(i=0;i<CHAR_SIZE_X;i++)
	{
		for(j=0;j<CHAR_SIZE_Y;j++)
		{
			u8 v=*(charData++);
			if(v)fb[0]=fb[1]=fb[2]=(v==1)?0xFF:0x00;
			fb+=3;
		}
		fb+=(h-CHAR_SIZE_Y)*3;
	}
}

int drawCharacterNew(u8* fb, char c, u16 x, u16 y, u16 w, u16 h)
{
	charDesc_s* cd=&fontDesc[(int)c];
	if(!cd->data)return 0;
	x+=cd->xo;
	if(x<0 || y<0 || x+cd->w>=w || y+cd->h>=h)return 0;
	u8* charData=cd->data;
	fb+=(x*h+y)*3;
	int i, j;
	for(i=0;i<cd->w;i++)
	{
		for(j=0;j<cd->h;j++)
		{
			u8 v=*(charData++);
			if(v)fb[0]=fb[1]=fb[2]=0xff-v;
			fb+=3;
		}
		fb+=(h-cd->h)*3;
	}
	return cd->xa;
}

void drawString(u8* fb, char* str, u16 x, u16 y, u16 w, u16 h)
{
	drawStringN(fb, str, strlen(str), x, y, w, h);
}

void drawStringN(u8* fb, char* str, u16 length, u16 x, u16 y, u16 w, u16 h)
{
	if(!fb || !str)return;
	int k; int dx=0, dy=0;
	k=strlen(str); if(k<length)length=k;
	for(k=0;k<length;k++)
	{
		// if(str[k]>=32 && str[k]<128)drawCharacter(fb,str[k],x+dx,y+dy,w,h);
		// dx+=8;
		dx+=drawCharacterNew(fb,str[k],x+dx,y+dy,w,h);
		if(str[k]=='\n'){dx=0;dy-=8;}
	}
}
