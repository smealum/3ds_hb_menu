#include <stdlib.h>
#include <3ds/types.h>
#include <3ds/HID.h>

#include "controls.h"

u32 previousPAD;
u32 currentPAD;

void initControls()
{
	hidInit(NULL);
	previousPAD=currentPAD=0;
}

void exitControls()
{
	hidExit();
}

void updateControls()
{
	previousPAD=currentPAD;
	currentPAD=hidSharedMem[7];
}

u32 keysDown()
{
	return currentPAD&~previousPAD;
}

u32 keysHeld()
{
	return currentPAD&previousPAD;
}
