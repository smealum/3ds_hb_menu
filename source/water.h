#ifndef WATER_H
#define WATER_H

#include <3ds/types.h>

typedef struct
{
	u16 numControlPoints;
	u16 neighborhoodSize;
	u16 width;
	s16 offset;
	float springFactor;
	float dampFactor;
	float* controlPoints;
	float* controlPointSpeeds;
}waterEffect_s;

void initWaterEffect(waterEffect_s* we, u16 n, u16 s, float d, float sf, u16 w, s16 offset);
void killWaterEffect(waterEffect_s* we);

float evaluateWater(waterEffect_s* we, u16 x);

void exciteWater(waterEffect_s* we, float v, u16 k, bool absolute);
void updateWaterEffect(waterEffect_s* we);

#endif
