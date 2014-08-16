#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctr/types.h>
#include <ctr/GSP.h>
#include <ctr/GX.h>
#include <ctr/svc.h>

#include "gfx.h"
#include "text.h"
#include "costable.h"

//TEMP, should not be here
#define WATERBORDERCOLOR (u8[]){118, 181, 205}
#define WATERCOLOR (u8[]){75, 157, 188}
#define BGCOLOR (u8[]){205, 223, 228}

u8* gspHeap;
u32* gxCmdBuf;

u8 currentBuffer;
u8* topLeftFramebuffers[2];
u8* subFramebuffers[2];

Handle gspEvent, gspSharedMemHandle;

void gfxInit()
{
	gspInit();

	GSPGPU_AcquireRight(NULL, 0x0);
	GSPGPU_SetLcdForceBlack(NULL, 0x0);

	//setup framebuffers
	topLeftFramebuffers[0]=(u8*)0x20000000;
	topLeftFramebuffers[1]=topLeftFramebuffers[0]+0x46500;
	subFramebuffers[0]=topLeftFramebuffers[1]+0x46500;
	subFramebuffers[1]=subFramebuffers[0]+0x38400;

	GSPGPU_WriteHWRegs(NULL, 0x400468, (u32*)&topLeftFramebuffers, 8);
	GSPGPU_WriteHWRegs(NULL, 0x400568, (u32*)&subFramebuffers, 8);
	
	topLeftFramebuffers[0]-=0xC000000;
	topLeftFramebuffers[1]-=0xC000000;
	subFramebuffers[0]-=0xC000000;
	subFramebuffers[1]-=0xC000000;

	//setup our gsp shared mem section
	u8 threadID;
	svc_createEvent(&gspEvent, 0x0);
	if(GSPGPU_RegisterInterruptRelayQueue(NULL, gspEvent, 0x1, &gspSharedMemHandle, &threadID))*(u32*)NULL=0xBABE0006;
	if(svc_mapMemoryBlock(gspSharedMemHandle, 0x10002000, 0x3, 0x10000000))*(u32*)NULL=0xBABE0008;

	//map GSP heap
	if(svc_controlMemory((u32*)&gspHeap, 0x0, 0x0, 0x02000000, 0x10003, 0x3))*(u32*)NULL=0xBABE0007;

	//wait until we can write stuff to it
	svc_waitSynchronization1(gspEvent, 0x55bcb0);

	//GSP shared mem : 0x2779F000
	gxCmdBuf=(u32*)(0x10002000+0x800+threadID*0x200);

	currentBuffer=0;
}

void gfxExit()
{
	GSPGPU_UnregisterInterruptRelayQueue(NULL);

	//unmap GSP shared mem
	svc_unmapMemoryBlock(gspSharedMemHandle, 0x10002000);
	svc_closeHandle(gspSharedMemHandle);
	svc_closeHandle(gspEvent);
	
	gspExit();

	//free GSP heap
	svc_controlMemory((u32*)&gspHeap, (u32)gspHeap, 0x0, 0x02000000, MEMOP_FREE, 0x0);
}

u8* gfxGetFramebuffer(bool top, u16* width, u16* height)
{
	if(width)*width=240;

	if(top)
	{
		if(height)*height=400;
		return topLeftFramebuffers[currentBuffer^1];
	}else{
		if(height)*height=320;
		return subFramebuffers[currentBuffer^1];
	}
}

void gfxSwapBuffers()
{
	u32 regData;

	GSPGPU_ReadHWRegs(NULL, 0x400478, (u32*)&regData, 4);
	regData^=1;
	currentBuffer=regData&1;
	GSPGPU_WriteHWRegs(NULL, 0x400478, (u32*)&regData, 4);
	GSPGPU_WriteHWRegs(NULL, 0x400578, (u32*)&regData, 4);
}

u32 cnt;

void gfxRenderFrame()
{
	u8* topFramebuffer=topLeftFramebuffers[currentBuffer^1];
	u8* subFramebuffer=subFramebuffers[currentBuffer^1];

	//top screen stuff
	gfxFillColor(true, BGCOLOR);
	gfxDrawWave(true, WATERBORDERCOLOR, 135, 15, cnt, 5);
	gfxDrawWave(true, WATERCOLOR, 130, 15, cnt, 0);
	gfxDrawText(true, "hello", 100, 100);

	//sub screen stuff
	gfxFillColor(false, WATERCOLOR);
	static u8 testSprite[48*48*3];
	memset(testSprite, 0x80, 48*48*3);
	gfxDrawSprite(false, testSprite, 48, 48, 100, 20);

	cnt++;

	GSPGPU_FlushDataCache(NULL, topFramebuffer, 0x46500);
	GSPGPU_FlushDataCache(NULL, subFramebuffer, 0x38400);
}

void gfxDrawText(bool top, char* str, u16 x, u16 y)
{
	if(!str)return;

	u16 fbWidth, fbHeight;
	u8* fbAdr=gfxGetFramebuffer(top, &fbWidth, &fbHeight);

	drawString(fbAdr, str, y, x-CHAR_SIZE_Y, fbHeight, fbWidth);
}

void gfxDrawSprite(bool top, u8* spriteData, u16 width, u16 height, u16 x, u16 y)
{
	if(!spriteData)return;

	u16 fbWidth, fbHeight;
	u8* fbAdr=gfxGetFramebuffer(top, &fbWidth, &fbHeight);

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
		memcpy(&fbAdr[((x+xOffset)+(y+j+yOffset)*fbWidth)*3], &spriteData[((xOffset)+(j+yOffset)*width)*3], widthDrawn*3);
	}
}

void gfxDrawDualSprite(u8* spriteData, u16 width, u16 height, u16 x, u16 y)
{
	if(!spriteData)return;

	gfxDrawSprite(true, spriteData, width, height, x-240, y);
	gfxDrawSprite(true, spriteData, width, height, x, y-40);
}

void gfxFillColor(bool top, u8 rgbColor[3])
{
	u16 fbWidth, fbHeight;
	u8* fbAdr=gfxGetFramebuffer(top, &fbWidth, &fbHeight);

	//TODO : optimize; use GX command ?
	int i;
	for(i=0; i<fbWidth*fbHeight; i++)
	{
		*(fbAdr++)=rgbColor[2];
		*(fbAdr++)=rgbColor[1];
		*(fbAdr++)=rgbColor[0];
	}
}

static inline u16 getWaveLevel(u16 j, u16 level, u16 amplitude, u16 t)
{
	return ((pcCos(j*4+t*8)*amplitude/8)/4096)+
			((pcCos(j+t)*amplitude)/4096)+
			level;
}

void gfxDrawWave(bool top, u8 rgbColor[3], u16 level, u16 amplitude, u16 t, u16 width)
{
	u16 fbWidth, fbHeight;
	u8* fbAdr=gfxGetFramebuffer(top, &fbWidth, &fbHeight);

	u8 colorLine[fbWidth*3];

	int j;
	for(j=0; j<fbWidth; j++)
	{
		colorLine[j*3+0]=rgbColor[2];
		colorLine[j*3+1]=rgbColor[1];
		colorLine[j*3+2]=rgbColor[0];
	}

	if(width)
	{
		for(j=0; j<fbHeight; j++)
		{
			u16 waveLevel=getWaveLevel(j, level, amplitude, t);
			memcpy(&fbAdr[(waveLevel-width)*3], colorLine, width*3);
			fbAdr+=fbWidth*3;
		}
	}else{
		for(j=0; j<fbHeight; j++)
		{
			u16 waveLevel=getWaveLevel(j, level, amplitude, t);
			memcpy(fbAdr, colorLine, waveLevel*3);
			fbAdr+=fbWidth*3;
		}
	}
}
