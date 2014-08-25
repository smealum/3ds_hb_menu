#ifndef GFX_H
#define GFX_H

#include <3ds/types.h>

typedef enum
{
	GFX_TOP = 0,
	GFX_BOTTOM = 1
}gfxScreen_t;

typedef enum
{
	GFX_LEFT = 0,
	GFX_RIGHT = 1,
	// GFX_BOTTOM = 0
}gfx3dSide_t;

//system stuff
void gfxInit();
void gfxExit();

//control stuff
void gfxSet3D(bool enable);
void gfxFlushBuffers();
void gfxSwapBuffers();

//helper stuff
u8* gfxGetFramebuffer(gfxScreen_t screen, gfx3dSide_t side, u16* width, u16* height);

//rendering stuff
void gfxDrawSprite(gfxScreen_t screen, gfx3dSide_t side, u8* spriteData, u16 width, u16 height, s16 x, s16 y);
void gfxDrawDualSprite(u8* spriteData, u16 width, u16 height, s16 x, s16 y);
void gfxDrawSpriteAlpha(gfxScreen_t screen, gfx3dSide_t side, u8* spriteData, u16 width, u16 height, s16 x, s16 y);
void gfxDrawSpriteAlphaBlend(gfxScreen_t screen, gfx3dSide_t side, u8* spriteData, u16 width, u16 height, s16 x, s16 y);
void gfxDrawSpriteAlphaBlendFade(gfxScreen_t screen, gfx3dSide_t side, u8* spriteData, u16 width, u16 height, s16 x, s16 y, u8 fadeValue);
void gfxDrawText(gfxScreen_t screen, gfx3dSide_t side, char* str, u16 x, u16 y);
void gfxFillColor(gfxScreen_t screen, gfx3dSide_t side, u8 rgbColor[3]);
void gfxDrawRectangle(gfxScreen_t screen, gfx3dSide_t side, u8 rgbColor[3], s16 x, s16 y, u16 width, u16 height);
void gfxDrawWave(gfxScreen_t screen, gfx3dSide_t side, u8 rgbColor[3], u16 level, u16 amplitude, u16 t, u16 width);

//global variables
extern u8* topLeftFramebuffers[2];
extern u8* subFramebuffers[2];

#endif
