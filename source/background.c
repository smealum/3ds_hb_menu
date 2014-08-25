#include "background.h"
#include "gfx.h"

void initBackground(void)
{

}

void drawBackground(void)
{
	static u32 cnt;

	//top screen stuff
	gfxFillColor(GFX_TOP, GFX_LEFT, BGCOLOR);
	gfxDrawWave(GFX_TOP, GFX_LEFT, WATERBORDERCOLOR, 135, 20, cnt, 5);
	gfxDrawWave(GFX_TOP, GFX_LEFT, WATERCOLOR, 130, 20, cnt, 0);

	//sub screen stuff
	gfxFillColor(GFX_BOTTOM, GFX_LEFT, WATERCOLOR);

	cnt += 2;
}

void drawBackgroundBrew(void)
{
	static u32 cnt;

	//top screen stuff
	gfxFillColor(GFX_TOP, GFX_LEFT, BGCOLOR);
	gfxDrawWave(GFX_TOP, GFX_LEFT, BEERBORDERCOLOR, 135, 20, cnt, 5);
	gfxDrawWave(GFX_TOP, GFX_LEFT, BEERCOLOR, 130, 20, cnt, 0);

	//sub screen stuff
	gfxFillColor(GFX_BOTTOM, GFX_LEFT, BEERCOLOR);

	cnt += 2;
}
