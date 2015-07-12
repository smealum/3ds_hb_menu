#ifndef REGIONFREE_H
#define REGIONFREE_H

#include <3ds.h>

#define REGIONFREE_PATH "regionfree:/"

extern bool regionFreeAvailable;

Result regionFreeInit();
Result regionFreeExit();
Result regionFreeRun();

#endif
