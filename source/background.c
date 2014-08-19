#include "background.h"
#include "gfx.h"

void initBackground(void)
{

}

void drawBackground(void)
{
	static u32 cnt;

	//top screen stuff
	gfxFillColor(true, BGCOLOR);
	gfxDrawWave(true, WATERBORDERCOLOR, 135, 20, cnt, 5);
	gfxDrawWave(true, WATERCOLOR, 130, 20, cnt, 0);

	//sub screen stuff
	gfxFillColor(false, WATERCOLOR);

	cnt += 2;
}
void drawBackgroundBrew(void)
{
	static u32 cnt;

	//top screen stuff
	gfxFillColor(true, BGCOLOR);
	gfxDrawWave(true, BEERBORDERCOLOR, 135, 20, cnt, 5);
	gfxDrawWave(true, BEERCOLOR, 130, 20, cnt, 0);

	//sub screen stuff
	gfxFillColor(false, BEERCOLOR);

	cnt += 2;
}
