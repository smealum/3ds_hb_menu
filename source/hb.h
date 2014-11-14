#ifndef HB_H
#define HB_H

#include <3ds.h>

Result initHb();
void exitHb();

Result HB_GetBootloaderAddresses(void** load3dsx, void** setArgv);

#endif
