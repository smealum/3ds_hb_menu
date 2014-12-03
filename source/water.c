#include <string.h>
#include <math.h>
#include <malloc.h>
#include "water.h"

void initWaterEffect(waterEffect_s* we, u16 n, u16 s, float d,  float sf, u16 w, s16 offset)
{
	if(!we)return;

	we->numControlPoints=n;
	we->neighborhoodSize=s;
	we->dampFactor=d;
	we->springFactor=sf;
	we->width=w;
	we->offset=offset;
	we->controlPoints=calloc(n, sizeof(float));
	we->controlPointSpeeds=calloc(n, sizeof(float));
}

//dst shouldn't have been initialized
void copyWaterEffect(waterEffect_s* dst, waterEffect_s* src)
{
	if(!dst || !src)return;

	initWaterEffect(dst, src->numControlPoints, src->neighborhoodSize, src->dampFactor, src->springFactor, src->width, src->offset);
	memcpy(dst->controlPoints, src->controlPoints, sizeof(float)*src->numControlPoints);
	memcpy(dst->controlPointSpeeds, src->controlPointSpeeds, sizeof(float)*src->numControlPoints);
}

void killWaterEffect(waterEffect_s* we)
{
	if(!we)return;

	free(we->controlPoints);
	free(we->controlPointSpeeds);
}

float getNeighborAverage(waterEffect_s* we, int k)
{
	if(!we || k<0 || k>=we->numControlPoints)return 0.0f;

	float sum=0.0f;
	float factors=0.0f;

	int i;
	for(i=k-we->neighborhoodSize; i<k+we->neighborhoodSize; i++)
	{
		if(i==k)continue;
		const int d=i-k;
		const float f=fabs(1.0f/d); // TODO : better function (gauss ?)
		float v=0.0f;
		if(i>=0 && i<we->numControlPoints)v=we->controlPoints[i];
		sum+=f*v;
		factors+=f;
	}

	return sum/factors;
}

float evaluateWater(waterEffect_s* we, u16 x)
{
	if(!we || x>=we->width)return 0.0f;

	const float vx=((float)((x-we->offset)*we->numControlPoints))/we->width;
	const int k=(int)vx;
	const float f=vx-(float)k;

	return we->controlPoints[k]*(1.0f-f)+we->controlPoints[k+1]*f;
}

void exciteWater(waterEffect_s* we, float v, u16 k, bool absolute)
{
	if(!we || k>=we->numControlPoints)return;

	if(absolute)
	{
		we->controlPoints[k]=v;
		we->controlPointSpeeds[k]=0.0f;
	}else we->controlPoints[k]+=v;
}

void updateWaterEffect(waterEffect_s* we)
{
	if(!we)return;

	waterEffect_s tmpwe;
	copyWaterEffect(&tmpwe, we);

	int k;
	for(k=0; k<we->numControlPoints; k++)
	{
		float rest=getNeighborAverage(&tmpwe, k);
		we->controlPointSpeeds[k]*=we->dampFactor;
		we->controlPointSpeeds[k]+=(rest-we->controlPoints[k])*we->springFactor;
		we->controlPoints[k]+=we->controlPointSpeeds[k];
	}

	killWaterEffect(&tmpwe);
}
