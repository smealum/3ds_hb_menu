#ifndef CONTROLS_H
#define CONTROLS_H

#include <ctr/types.h>
#include <ctr/HID.h>

//system stuff
void initControls();
void updateControls();

//access stuff
u32 keysDown();
u32 keysHeld();

#endif
