#include <math.h>

#include "background.h"
#include "water.h"
#include "gfx.h"

#include "logo_bin.h"
#include "bubble_bin.h"

#define BG_WATER_CONTROLPOINTS (80)
#define BG_WATER_NEIGHBORHOODS (3)
#define BG_WATER_DAMPFACTOR (0.7f)
#define BG_WATER_SPRINGFACTOR (0.8f)
#define BG_WATER_WIDTH (400)

static bubble_t bubbles[BUBBLE_COUNT];
static waterEffect_s waterEffect;
static int backgroundCnt;

void initBackground(void)
{
	int i = 0;
	for(i = 0;i < BUBBLE_COUNT;i += 1)
	{
		bubbles[i].x = rand() % 400;
		bubbles[i].y = rand() % 240;
		bubbles[i].fade = 15;
	}

	initWaterEffect(&waterEffect, BG_WATER_CONTROLPOINTS, BG_WATER_NEIGHBORHOODS, BG_WATER_DAMPFACTOR, BG_WATER_SPRINGFACTOR, BG_WATER_WIDTH);
	backgroundCnt = 0;
}

void updateBubble(bubble_t* bubble)
{
	// Float up the screen.
	bubble->y += 2;

	// Check if faded away, then reset if gone.
	if(bubble->fade < 10)
	{
		bubble->x = rand() % 400;
		bubble->y = rand() % 10;
		bubble->fade = 15;
	}
	// Check if too far up screen and start fizzling away.
	else if(bubble->y >= 240 && bubble->y % 240 > 100)
	{
		bubble->fade -= 10;
	}
	// Otherwise make sure the bubble is visible.
	else if(bubble->fade < 255)
	{
		bubble->fade += 10;
	}
}

void drawBubbles(void)
{
	int i;
	//BUBBLES!!
	for(i = 0;i < BUBBLE_COUNT;i += 1)
	{
		// Then draw (no point in separating more because then we go through them all twice).
		gfxDrawSpriteAlphaBlendFade((bubbles[i].y >= 240) ? (GFX_TOP) : (GFX_BOTTOM), GFX_LEFT, (u8*)bubble_bin, 32, 32, 
			((bubbles[i].y >= 240) ? -64 : 0) + bubbles[i].y % 240, 
			((bubbles[i].y >= 240) ? 0 : -40) + bubbles[i].x, bubbles[i].fade);
	}
}

void updateBackground(void)
{
	int i;
	for(i = 0;i < BUBBLE_COUNT;i += 1)
	{
		// Update first
		updateBubble(&bubbles[i]);
	}

	exciteWater(&waterEffect, sin(backgroundCnt*0.1f), 0);

	//TEMP TEST
	if(hidKeysDown()&KEY_UP)exciteWater(&waterEffect, 1.0f, 0);
	if(hidKeysDown()&KEY_DOWN)exciteWater(&waterEffect, 2.0f, 0);
	if(hidKeysDown()&KEY_LEFT)exciteWater(&waterEffect, 5.0f, 0);
	if(hidKeysDown()&KEY_RIGHT)exciteWater(&waterEffect, -10.0f, 0);
	if(hidKeysDown()&KEY_R)exciteWater(&waterEffect, 20.0f, 0);
	if(hidKeysDown()&KEY_L)exciteWater(&waterEffect, 50.0f, 0);

	updateWaterEffect(&waterEffect);

	backgroundCnt++;
}

void drawBackground(u8 bgColor[3], u8 waterBorderColor[3], u8 waterColor[3])
{
	//top screen stuff
	//gfxFillColorGradient(GFX_TOP, GFX_LEFT, waterBorderColor, waterColor);
	gfxFillColor(GFX_TOP, GFX_LEFT, bgColor);
	gfxDrawWave(GFX_TOP, GFX_LEFT, waterBorderColor, waterColor, 135, 20, 5, (gfxWaveCallback)&evaluateWater, &waterEffect);
	gfxDrawWave(GFX_TOP, GFX_LEFT, waterColor, waterBorderColor, 130, 20, 0, (gfxWaveCallback)&evaluateWater, &waterEffect);

	//sub screen stuff
	gfxFillColorGradient(GFX_BOTTOM, GFX_LEFT, waterColor, waterBorderColor);

	// Bubbles belong on both screens so they should be drawn second to last.
	drawBubbles();

	// Finally draw the logo.
	gfxDrawSpriteAlphaBlend(GFX_TOP, GFX_LEFT, (u8*)logo_bin, 113, 271, 64, 80);
}

