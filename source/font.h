#ifndef FONT_H
#define FONT_H

typedef struct {char c; int x, y, w, h, xo, yo, xa; u8* data;}charDesc_s;
typedef struct
{
	u8* data;
	charDesc_s* desc;
	u8 height;
	u8 color[3];
}font_s;

extern u8 font1Data[];
extern charDesc_s font1Desc[];

extern u8 font2Data[];
extern charDesc_s font2Desc[];

extern font_s fontDefault;
extern font_s fontTitle;
extern font_s fontDescription;

#endif
