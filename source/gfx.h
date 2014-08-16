#ifndef GFX_H
#define GFX_H

#include <ctr/types.h>

//system stuff
void gfxInit();
void gfxExit();

//control stuff
void gfxRenderFrame();
void gfxSwapBuffers();

//helper stuff
u8* gfxGetFramebuffer(bool top, u16* width, u16* height);

//rendering stuff
void gfxDrawSprite(bool top, u8* spriteData, u16 width, u16 height, u16 x, u16 y);
void gfxDrawDualSprite(u8* spriteData, u16 width, u16 height, u16 x, u16 y);
void gfxDrawText(bool top, char* str, u16 x, u16 y);
void gfxFillColor(bool top, u8 rgbColor[3]);
void gfxDrawWave(bool top, u8 rgbColor[3], u16 level, u16 amplitude, u16 t, u16 width);

//global variables
extern u8* topLeftFramebuffers[2];
extern u8* subFramebuffers[2];

#endif
