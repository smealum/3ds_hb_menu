#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <3ds.h>

#include "gfx.h"
#include "text.h"
#include "costable.h"

void gfxDrawText(gfxScreen_t screen, gfx3dSide_t side, char* str, u16 x, u16 y)
{
	if(!str)return;

	u16 fbWidth, fbHeight;
	u8* fbAdr=gfxGetFramebuffer(screen, side, &fbWidth, &fbHeight);

	drawString(fbAdr, str, y, x-CHAR_SIZE_Y, fbHeight, fbWidth);
}

void gfxDrawSprite(gfxScreen_t screen, gfx3dSide_t side, u8* spriteData, u16 width, u16 height, s16 x, s16 y)
{
	if(!spriteData)return;

	u16 fbWidth, fbHeight;
	u8* fbAdr=gfxGetFramebuffer(screen, side, &fbWidth, &fbHeight);

	if(x+width<0 || x>=fbWidth)return;
	if(y+height<0 || y>=fbHeight)return;

	u16 xOffset=0, yOffset=0;
	u16 widthDrawn=width, heightDrawn=height;

	if(x<0)xOffset=-x;
	if(y<0)yOffset=-y;
	if(x+width>=fbWidth)widthDrawn=fbWidth-x;
	if(y+height>=fbHeight)heightDrawn=fbHeight-y;
	widthDrawn-=xOffset;
	heightDrawn-=yOffset;

	int j;
	for(j=yOffset; j<yOffset+heightDrawn; j++)
	{
		memcpy(&fbAdr[((x+xOffset)+(y+j)*fbWidth)*3], &spriteData[((xOffset)+(j)*width)*3], widthDrawn*3);
	}
}

void gfxDrawDualSprite(u8* spriteData, u16 width, u16 height, s16 x, s16 y)
{
	if(!spriteData)return;

	gfxDrawSprite(GFX_TOP, GFX_LEFT, spriteData, width, height, x-240, y);
	gfxDrawSprite(GFX_BOTTOM, GFX_LEFT, spriteData, width, height, x, y-40);
}

void gfxDrawSpriteAlpha(gfxScreen_t screen, gfx3dSide_t side, u8* spriteData, u16 width, u16 height, s16 x, s16 y)
{
	if(!spriteData)return;

	u16 fbWidth, fbHeight;
	u8* fbAdr=gfxGetFramebuffer(screen, side, &fbWidth, &fbHeight);

	if(x+width<0 || x>=fbWidth)return;
	if(y+height<0 || y>=fbHeight)return;

	u16 xOffset=0, yOffset=0;
	u16 widthDrawn=width, heightDrawn=height;

	if(x<0)xOffset=-x;
	if(y<0)yOffset=-y;
	if(x+width>=fbWidth)widthDrawn=fbWidth-x;
	if(y+height>=fbHeight)heightDrawn=fbHeight-y;
	widthDrawn-=xOffset;
	heightDrawn-=yOffset;

	//TODO : optimize
	fbAdr+=(y+yOffset)*fbWidth*3;
	spriteData+=yOffset*width*4;
	int j, i;
	for(j=yOffset; j<yOffset+heightDrawn; j++)
	{
		u8* fbd=&fbAdr[(x+xOffset)*3];
		u8* data=&spriteData[(xOffset)*4];
		for(i=xOffset; i<xOffset+widthDrawn; i++)
		{
			if(data[3])
			{
				fbd[0]=data[0];
				fbd[1]=data[1];
				fbd[2]=data[2];
			}
			fbd+=3;
			data+=4;
		}
		fbAdr+=fbWidth*3;
		spriteData+=width*4;
	}
}
void gfxDrawSpriteAlphaBlend(gfxScreen_t screen, gfx3dSide_t side, u8* spriteData, u16 width, u16 height, s16 x, s16 y)
{
	if(!spriteData)return;

	u16 fbWidth, fbHeight;
	u8* fbAdr=gfxGetFramebuffer(screen, side, &fbWidth, &fbHeight);

	if(x+width<0 || x>=fbWidth)return;
	if(y+height<0 || y>=fbHeight)return;

	u16 xOffset=0, yOffset=0;
	u16 widthDrawn=width, heightDrawn=height;

	if(x<0)xOffset=-x;
	if(y<0)yOffset=-y;
	if(x+width>=fbWidth)widthDrawn=fbWidth-x;
	if(y+height>=fbHeight)heightDrawn=fbHeight-y;
	widthDrawn-=xOffset;
	heightDrawn-=yOffset;

	//TODO : optimize
	fbAdr+=(y+yOffset)*fbWidth*3;
	spriteData+=yOffset*width*4;
	int j, i;
	for(j=yOffset; j<yOffset+heightDrawn; j++)
	{
		u8* fbd=&fbAdr[(x+xOffset)*3];
		u8* data=&spriteData[(xOffset)*4];
		for(i=xOffset; i<xOffset+widthDrawn; i++)
		{
			if(data[3])
			{
				u8 alphaSource = data[3];
				fbd[0]=((data[0] * alphaSource) / 256)+((fbd[0] * (255 - alphaSource)) / 256);
				fbd[1]=((data[1] * alphaSource) / 256)+((fbd[1] * (255 - alphaSource)) / 256);
				fbd[2]=((data[2] * alphaSource) / 256)+((fbd[2] * (255 - alphaSource)) / 256);
			}
			fbd+=3;
			data+=4;
		}
		fbAdr+=fbWidth*3;
		spriteData+=width*4;
	}
}
void gfxDrawSpriteAlphaBlendFade(gfxScreen_t screen, gfx3dSide_t side, u8* spriteData, u16 width, u16 height, s16 x, s16 y, u8 fadeValue)
{
	if(!spriteData)return;

	u16 fbWidth, fbHeight;
	u8* fbAdr=gfxGetFramebuffer(screen, side, &fbWidth, &fbHeight);

	if(x+width<0 || x>=fbWidth)return;
	if(y+height<0 || y>=fbHeight)return;

	u16 xOffset=0, yOffset=0;
	u16 widthDrawn=width, heightDrawn=height;

	if(x<0)xOffset=-x;
	if(y<0)yOffset=-y;
	if(x+width>=fbWidth)widthDrawn=fbWidth-x;
	if(y+height>=fbHeight)heightDrawn=fbHeight-y;
	widthDrawn-=xOffset;
	heightDrawn-=yOffset;

	//TODO : optimize
	fbAdr+=(y+yOffset)*fbWidth*3;
	spriteData+=yOffset*width*4;
	int j, i;
	for(j=yOffset; j<yOffset+heightDrawn; j++)
	{
		u8* fbd=&fbAdr[(x+xOffset)*3];
		u8* data=&spriteData[(xOffset)*4];
		for(i=xOffset; i<xOffset+widthDrawn; i++)
		{
			if(data[3])
			{
				u8 alphaSource = (fadeValue * data[3]) / 256;
				fbd[0]=((data[0] * alphaSource) / 256)+((fbd[0] * (255 - alphaSource)) / 256);
				fbd[1]=((data[1] * alphaSource) / 256)+((fbd[1] * (255 - alphaSource)) / 256);
				fbd[2]=((data[2] * alphaSource) / 256)+((fbd[2] * (255 - alphaSource)) / 256);
			}
			fbd+=3;
			data+=4;
		}
		fbAdr+=fbWidth*3;
		spriteData+=width*4;
	}
}

void gfxFillColor(gfxScreen_t screen, gfx3dSide_t side, u8 rgbColor[3])
{
	u16 fbWidth, fbHeight;
	u8* fbAdr=gfxGetFramebuffer(screen, side, &fbWidth, &fbHeight);

	//TODO : optimize; use GX command ?
	int i;
	for(i=0; i<fbWidth*fbHeight; i++)
	{
		*(fbAdr++)=rgbColor[2];
		*(fbAdr++)=rgbColor[1];
		*(fbAdr++)=rgbColor[0];
	}
}
void gfxFillColorGradient(gfxScreen_t screen, gfx3dSide_t side, u8 rgbColorStart[3], u8 rgbColorEnd[3])
{
	u16 fbWidth, fbHeight;
	u8* fbAdr=gfxGetFramebuffer(screen, side, &fbWidth, &fbHeight);
	u8 colorLine[fbWidth*3];

	//TODO : optimize; use GX command ?
	int i;
	float n;
	float total = (float)(fbWidth - 1);
	// make slightly bigger to prevent gradients from blending around.  SHould be removed and have the gradient color be better later.
	total *= 1.5f;
	for(i=0; i<fbWidth; i++)
	{
		n = (float)i / total;
		colorLine[i*3+0]=(float)rgbColorStart[2] * (1.0f-n) + (float)rgbColorEnd[2] * n;
		colorLine[i*3+1]=(float)rgbColorStart[1] * (1.0f-n) + (float)rgbColorEnd[1] * n;
		colorLine[i*3+2]=(float)rgbColorStart[0] * (1.0f-n) + (float)rgbColorEnd[0] * n;
	}

	for(i=0; i<fbHeight; i++)
	{
		memcpy(fbAdr, colorLine, fbWidth*3);
		fbAdr+=fbWidth*3;
	}
}

void gfxDrawRectangle(gfxScreen_t screen, gfx3dSide_t side, u8 rgbColor[3], s16 x, s16 y, u16 width, u16 height)
{
	u16 fbWidth, fbHeight;
	u8* fbAdr=gfxGetFramebuffer(screen, side, &fbWidth, &fbHeight);

	if(x+width<0 || x>=fbWidth)return;
	if(y+height<0 || y>=fbHeight)return;

	if(x<0){width+=x; x=0;}
	if(y<0){height+=y; y=0;}
	if(x+width>=fbWidth)width=fbWidth-x;
	if(y+height>=fbHeight)height=fbHeight-y;

	u8 colorLine[width*3];

	int j;
	for(j=0; j<width; j++)
	{
		colorLine[j*3+0]=rgbColor[2];
		colorLine[j*3+1]=rgbColor[1];
		colorLine[j*3+2]=rgbColor[0];
	}

	fbAdr+=fbWidth*3*y;
	for(j=0; j<height; j++)
	{
		memcpy(&fbAdr[x*3], colorLine, width*3);
		fbAdr+=fbWidth*3;
	}
}

void gfxDrawWave(gfxScreen_t screen, gfx3dSide_t side, u8 rgbColorStart[3], u8 rgbColorEnd[3], u16 level, u16 amplitude, u16 width, gfxWaveCallback cb, void* p)
{
	u16 fbWidth, fbHeight;
	u8* fbAdr=gfxGetFramebuffer(screen, side, &fbWidth, &fbHeight);

	u8 colorLine[fbWidth*3];

	int j;

	if(width)
	{
		for(j=0; j<fbWidth; j++)
		{
			colorLine[j*3+0]=rgbColorStart[2];
			colorLine[j*3+1]=rgbColorStart[1];
			colorLine[j*3+2]=rgbColorStart[0];
		}
		for(j=0; j<fbHeight; j++)
		{
			u16 waveLevel=level+cb(p, j)*amplitude;
			memcpy(&fbAdr[(waveLevel-width)*3], colorLine, width*3);
			fbAdr+=fbWidth*3;
		}
	}else{
		int i;
		float n;
		float total = (float)(fbWidth - 1);
		// make slightly bigger to prevent gradients from blending around.  SHould be removed and have the gradient color be better later.
		total *= 1.5f;
		for(i=0; i<fbWidth; i++)
		{
			n = (float)i / total;
			colorLine[i*3+0]=(float)rgbColorStart[2] * (1.0f-n) + (float)rgbColorEnd[2] * n;
			colorLine[i*3+1]=(float)rgbColorStart[1] * (1.0f-n) + (float)rgbColorEnd[1] * n;
			colorLine[i*3+2]=(float)rgbColorStart[0] * (1.0f-n) + (float)rgbColorEnd[0] * n;
		}

		for(j=0; j<fbHeight; j++)
		{
			u16 waveLevel=level+cb(p, j)*amplitude;
			memcpy(fbAdr, colorLine, waveLevel*3);
			fbAdr+=fbWidth*3;
		}
	}
}
