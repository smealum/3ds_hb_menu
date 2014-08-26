#pragma once

#define CHAR_SIZE_X (8)
#define CHAR_SIZE_Y (8)

void drawCharacter(u8* fb, char c, u16 x, u16 y, u16 w, u16 h);
void drawString(u8* fb, char* str, u16 x, u16 y, u16 w, u16 h);
