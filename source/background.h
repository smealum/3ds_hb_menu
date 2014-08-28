#pragma once
#include <3ds.h>

#define WATERBORDERCOLOR (u8[]){66, 224, 255}
#define WATERCOLOR (u8[]){66, 163, 255}

#define BEERBORDERCOLOR (u8[]){240, 240, 240}
#define BEERCOLOR (u8[]){188, 157, 75}

#define BGCOLOR (u8[]){0, 132, 255}

#define BUBBLE_COUNT 15

typedef struct
{
	s32 x, y;
	u8 fade;
}bubble_t;

void initBackground(void);
void drawBackground(u8 bgColor[3], u8 waterBorderColor[3], u8 waterColor[3]);
