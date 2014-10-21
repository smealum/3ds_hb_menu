#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <3ds.h>
#include "text.h"
#include "font_bin.h"

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

void drawString(u8* fb, char* str, u16 x, u16 y, u16 w, u16 h)
{
	if(!fb || !str)return;
	int k; int dx=0, dy=0;
	for(k=0;k<strlen(str);k++)
	{
		if(str[k]>=32 && str[k]<128)drawCharacter(fb,str[k],x+dx,y+dy,w,h);
		dx+=8;
		if(str[k]=='\n'){dx=0;dy-=8;}
	}
}

void drawStringN(u8* fb, char* str, u16 length, u16 x, u16 y, u16 w, u16 h)
{
	if(!fb || !str)return;
	int k; int dx=0, dy=0;
	for(k=0;k<strlen(str) && k < length;k++)
	{
		if(str[k]>=32 && str[k]<128)drawCharacter(fb,str[k],x+dx,y+dy,w,h);
		dx+=8;
		if(str[k]=='\n'){dx=0;dy-=8;}
	}
}
