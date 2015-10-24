#ifndef SHORTCUT_H
#define SHORTCUT_H

#include <3ds.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	char* executable;
	char* descriptor;
	char* icon;
	char* arg;
}shortcut_s;

void initShortcut(shortcut_s* d);
void freeShortcut(shortcut_s* d);
Result loadShortcut(shortcut_s* d, char* path);
Result createShortcut(shortcut_s* d, char* path);

#ifdef __cplusplus
}
#endif

#endif
