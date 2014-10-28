#include <3ds.h>
#include "font.h"

font_s fontDefault =
{
	font1Data,
	font1Desc,
	16,
	(u8[]){0xFF,0xFF,0xFF}
};

font_s fontTitle =
{
	font1Data,
	font1Desc,
	16,
	(u8[]){0x76,0x76,0x76}
};

font_s fontDescription =
{
	font2Data,
	font2Desc,
	13,
	(u8[]){0x76,0x76,0x76}
};
