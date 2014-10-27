#pragma once
#include <3ds.h>

typedef float (*gfxWaveCallback)(void* p, u16 x);

//rendering stuff
void gfxDrawSprite(gfxScreen_t screen, gfx3dSide_t side, u8* spriteData, u16 width, u16 height, s16 x, s16 y);
void gfxDrawDualSprite(u8* spriteData, u16 width, u16 height, s16 x, s16 y);
void gfxDrawSpriteAlpha(gfxScreen_t screen, gfx3dSide_t side, u8* spriteData, u16 width, u16 height, s16 x, s16 y);
void gfxDrawSpriteAlphaBlend(gfxScreen_t screen, gfx3dSide_t side, u8* spriteData, u16 width, u16 height, s16 x, s16 y);
void gfxDrawSpriteAlphaBlendFade(gfxScreen_t screen, gfx3dSide_t side, u8* spriteData, u16 width, u16 height, s16 x, s16 y, u8 fadeValue);
void gfxDrawText(gfxScreen_t screen, gfx3dSide_t side, char* str, u16 x, u16 y, u8 r, u8 g, u8 b);
void gfxDrawTextN(gfxScreen_t screen, gfx3dSide_t side, char* str, u16 length, u16 x, u16 y, u8 r, u8 g, u8 b);
void gfxFillColor(gfxScreen_t screen, gfx3dSide_t side, u8 rgbColor[3]);
void gfxFillColorGradient(gfxScreen_t screen, gfx3dSide_t side, u8 rgbColorStart[3], u8 rgbColorEnd[3]);
void gfxDrawRectangle(gfxScreen_t screen, gfx3dSide_t side, u8 rgbColor[3], s16 x, s16 y, u16 width, u16 height);
void gfxDrawWave(gfxScreen_t screen, gfx3dSide_t side, u8 rgbColorStart[3], u8 rgbColorEnd[3], u16 level, u16 amplitude, u16 width, gfxWaveCallback cb, void* p);
