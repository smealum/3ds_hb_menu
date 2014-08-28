#ifndef WATER_H
#define WATER_H

#include <3ds/types.h>

typedef struct
{
	u16 numControlPoints;
	u16 neighborhoodSize;
	u16 width;
	float dampFactor;
	float* controlPoints;
	float* controlPointSpeeds;
}waterEffect_s;

void initWaterEffect(waterEffect_s* we, u16 n, u16 s, float d, u16 w);
void killWaterEffect(waterEffect_s* we);

float evaluateWater(waterEffect_s* we, u16 x);

void exciteWater(waterEffect_s* we, float v, u16 k);
void updateWaterEffect(waterEffect_s* we);

#endif
