#pragma once

#define CHAR_SIZE_X (8)
#define CHAR_SIZE_Y (8)

void drawCharacter(u8* fb, char c, u16 x, u16 y, u16 w, u16 h);
void drawString(u8* fb, char* str, s16 x, s16 y, u16 w, u16 h, u8 r, u8 g, u8 b);
void drawStringN(u8* fb, char* str, u16 length, s16 x, s16 y, u16 w, u16 h, u8 r, u8 g, u8 b);
