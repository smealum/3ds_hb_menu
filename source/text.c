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

//this code is not meant to be readable
int drawCharacterNew(u8* fb, char c, s16 x, s16 y, u16 w, u16 h, u8 r, u8 g, u8 b)
{
	charDesc_s* cd=&fontDesc[(int)c];
	if(!cd->data)return 0;
	x+=cd->xo;
	if(x<0 || x+cd->w>=w || y<-cd->h || y>=h+cd->h)return 0;
	u8* charData=cd->data;
	int i, j;
	s16 cy=y, ch=cd->h, cyo=0;
	if(y<0){cy=0;cyo=-y;ch=cd->h-cyo;}
	else if(y+ch>h)ch=h-y;
	fb+=(x*h+cy)*3;
	for(i=0;i<cd->w;i++)
	{
		charData+=cyo;
		for(j=0;j<ch;j++)
		{
			u8 v=*(charData++);
			if(v)
			{
				fb[0]=(fb[0]*(0xFF-v)+(b*v))>>8;
				fb[1]=(fb[1]*(0xFF-v)+(g*v))>>8;
				fb[2]=(fb[2]*(0xFF-v)+(r*v))>>8;
			}
			fb+=3;
		}
		charData+=(cd->h-(cyo+ch));
		fb+=(h-ch)*3;
	}
	return cd->xa;
}

void drawString(u8* fb, char* str, s16 x, s16 y, u16 w, u16 h, u8 r, u8 g, u8 b)
{
	drawStringN(fb, str, strlen(str), x, y, w, h, r, g, b);
}

void drawStringN(u8* fb, char* str, u16 length, s16 x, s16 y, u16 w, u16 h, u8 r, u8 g, u8 b)
{
	if(!fb || !str)return;
	int k; int dx=0, dy=0;
	k=strlen(str); if(k<length)length=k;
	for(k=0;k<length;k++)
	{
		// if(str[k]>=32 && str[k]<128)drawCharacter(fb,str[k],x+dx,y+dy,w,h);
		// dx+=8;
		dx+=drawCharacterNew(fb,str[k],x+dx,y+dy,w,h,r,g,b);
		if(str[k]=='\n'){dx=0;dy-=8;}
	}
}
