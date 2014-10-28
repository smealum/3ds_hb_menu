#pragma once

#include "font.h"

int drawCharacter(u8* fb, font_s* f, char c, s16 x, s16 y, u16 w, u16 h);
void drawString(u8* fb, font_s* f, char* str, s16 x, s16 y, u16 w, u16 h);
void drawStringN(u8* fb, font_s* f, char* str, u16 length, s16 x, s16 y, u16 w, u16 h);
