#ifndef REGIONFREE_H
#define REGIONFREE_H

#include <3ds.h>

extern bool regionFreeAvailable;

Result regionFreeInit();
Result regionFreeExit();
Result regionFreeRun();

#endif
