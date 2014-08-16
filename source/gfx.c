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
}

u32 cnt;

void gfxRenderFrame()
{
	u8* topFramebuffer=topLeftFramebuffers[currentBuffer^1];
	u8* subFramebuffer=subFramebuffers[currentBuffer^1];

	int i, j;
	for(i=1;i<400;i++)
	{
		for(j=1;j<240;j++)
		{
			u32 v=(j+i*240)*3;
			topFramebuffer[v]=(pcCos(i+cnt)+4096)/32;
			topFramebuffer[v+1]=(pcCos(j-256+cnt)+4096)/64;
			topFramebuffer[v+2]=(pcCos(i+128-cnt)+4096)/32;
		}
	}
	cnt++;

	char str[256];
	sprintf(str, "hello\n");
	gfxDrawText(true, str, 0, 0);

	static u8 testSprite[48*48*3];
	memset(testSprite, 0x80, 48*48*3);
	gfxDrawSprite(false, testSprite, 48, 48, 100, 20);

	GSPGPU_FlushDataCache(NULL, topFramebuffer, 0x46500);
	GSPGPU_FlushDataCache(NULL, subFramebuffer, 0x38400);
}

void gfxDrawText(bool top, char* str, u16 x, u16 y)
{
	if(!str)return;

	u16 fbWidth, fbHeight;
	u8* fbAdr=gfxGetFramebuffer(top, &fbWidth, &fbHeight);

	drawString(fbAdr, str, y, x, fbHeight, fbWidth);
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
