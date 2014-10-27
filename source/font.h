#ifndef FONT_H
#define FONT_H

typedef struct {char c; int x, y, w, h, xo, yo, xa; u8* data;}charDesc_s;
extern u8 fontData[];
extern charDesc_s fontDesc[];

#endif
