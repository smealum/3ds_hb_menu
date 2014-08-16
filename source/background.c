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
	gfxDrawWave(true, WATERBORDERCOLOR, 135, 15, cnt, 5);
	gfxDrawWave(true, WATERCOLOR, 130, 15, cnt, 0);

	//sub screen stuff
	gfxFillColor(false, WATERCOLOR);

	cnt++;
}
