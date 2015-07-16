#ifndef BOOT_H
#define BOOT_H

#include <3ds.h>
#include "scanner.h"

bool isNinjhax2(void);
int bootApp(char* executablePath, executableMetadata_s* em);

#endif
