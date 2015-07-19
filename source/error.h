#ifndef ERROR_H
#define ERROR_H

#include <3ds.h>

void initErrors();
void drawError(gfxScreen_t screen, char* title, char* body, int offset);

#endif
