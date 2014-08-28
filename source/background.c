#include "background.h"
#include "gfx.h"

#include "logo_bin.h"
#include "bubble_bin.h"

bubble_t bubbles[BUBBLE_COUNT];

void initBackground(void)
{
	int i = 0;
	for(i = 0;i < BUBBLE_COUNT;i += 1)
	{
		bubbles[i].x = rand() % 400;
		bubbles[i].y = rand() % 240;
		bubbles[i].fade = 15;
	}
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
	int i = 0;
	//BUBBLES!!
	for(i = 0;i < BUBBLE_COUNT;i += 1)
	{
		// Update first
		updateBubble(&bubbles[i]);
		// Then draw (no point in separating more because then we go through them all twice).
		gfxDrawSpriteAlphaBlendFade((bubbles[i].y >= 240) ? (GFX_TOP) : (GFX_BOTTOM), GFX_LEFT, (u8*)bubble_bin, 32, 32, 
			((bubbles[i].y >= 240) ? -64 : 0) + bubbles[i].y % 240, 
			((bubbles[i].y >= 240) ? 0 : -40) + bubbles[i].x, bubbles[i].fade);
	}
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

	// Bubbles belong on both screens so they should be drawn second to last.
	drawBubbles();

	// Finally draw the logo.
	gfxDrawSpriteAlphaBlend(GFX_TOP, GFX_LEFT, (u8*)logo_bin, 113, 271, 64, 80);

	cnt += 2;
}

