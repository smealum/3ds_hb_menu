#include <string.h>
#include <3ds/types.h>

#include "smdh.h"
#include "utils.h"

int extractSmdhData(smdh_s* s, char* name, char* desc, u8* iconData)
{
	if(!s)return -1;
	if(s->header.magic!=0x48444D53)return -2;

	if(name)unicodeToChar(name, s->applicationTitles[1].shortDescription, 0x40);
	if(desc)unicodeToChar(name, s->applicationTitles[1].longDescription, 0x80);
	if(iconData)memcpy(iconData, s->bigIconData, 0x1200); //TODO : support formats + tiling

	return 0;
}
