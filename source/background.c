#include "background.h"
#include "gfx.h"

void initBackground(void)
{

}

void drawBackground(u8 bgColor[3], u8 waterBorderColor[3], u8 waterColor[3])
{
	static u32 cnt;

	//top screen stuff
	gfxFillColor(GFX_TOP, GFX_LEFT, bgColor);
	gfxDrawWave(GFX_TOP, GFX_LEFT, waterBorderColor, 135, 20, cnt, 5);
	gfxDrawWave(GFX_TOP, GFX_LEFT, waterColor, 130, 20, cnt, 0);

	//sub screen stuff
	gfxFillColor(GFX_BOTTOM, GFX_LEFT, waterColor);

	cnt += 2;
}

