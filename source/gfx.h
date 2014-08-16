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
void gfxDrawText(bool top, char* str, u16 x, u16 y);

//global variables
extern u8* topLeftFramebuffers[2];
extern u8* subFramebuffers[2];

#endif
