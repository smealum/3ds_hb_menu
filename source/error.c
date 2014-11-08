#include <3ds.h>

#include "error.h"
#include "menu.h"
#include "text.h"

u8 roundLutError[]={8, 5, 4, 3, 2, 1, 1, 1, 0};

int countLines(char* str)
{
	if(!str)return 0;
	int cnt; for(cnt=1;*str=='\n'?++cnt:*str;str++);
	return cnt;
}

void initErrors()
{
	//placeholder
}

void drawError(gfxScreen_t screen, char* title, char* body)
{
	int i;

	int numLines=countLines(body);

	int width=numLines*8+32;
	int height=300;
	int x=240-width-12, y=4;

	//main frame
	for(i=0; i<9; i++)gfxDrawRectangle(screen, GFX_LEFT, ENTRY_BGCOLOR, x+roundLutError[i], y+i, width-roundLutError[i]*2, 1);
	gfxDrawRectangle(screen, GFX_LEFT, ENTRY_BGCOLOR, x, y+9, width, height-9*2);
	for(i=0; i<9; i++)gfxDrawRectangle(screen, GFX_LEFT, ENTRY_BGCOLOR, x+roundLutError[i], y+height-1-i, width-roundLutError[i]*2, 1);

	//content
	gfxDrawText(screen, GFX_LEFT, &fontTitle, title, x+width-6-16, y+6);
	gfxDrawText(screen, GFX_LEFT, &fontDescription, body, x+width-5-16-13, y+8);
}
