#ifndef BOOT_H
#define BOOT_H

#include <3ds.h>
#include "scanner.h"
#include "titles.h"

extern int targetProcessId;

bool isNinjhax2(void);
void bootSetTargetTitle(titleInfo_s info);
int bootApp(char* executablePath, executableMetadata_s* em, char* arg);

#endif
