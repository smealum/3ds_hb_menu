#ifndef CONTROLS_H
#define CONTROLS_H

#include <3ds/types.h>
#include <3ds/HID.h>

//system stuff
void initControls();
void updateControls();

//access stuff
u32 keysDown();
u32 keysHeld();

#endif
