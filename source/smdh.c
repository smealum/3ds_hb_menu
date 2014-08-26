#include <string.h>
#include <3ds.h>

#include "smdh.h"
#include "utils.h"

//shamelessly stolen from bch2obj.py
u8 tileOrder[]={0,1,8,9,2,3,10,11,16,17,24,25,18,19,26,27,4,5,12,13,6,7,14,15,20,21,28,29,22,23,30,31,32,33,40,41,34,35,42,43,48,49,56,57,50,51,58,59,36,37,44,45,38,39,46,47,52,53,60,61,54,55,62,63};

static inline void putPixel565(u8* dst, u8 x, u8 y, u16 v)
{
	dst[((47-y)+x*48)*3+0]=(v&0x1F)<<3;
	dst[((47-y)+x*48)*3+1]=((v>>5)&0x3F)<<2;
	dst[((47-y)+x*48)*3+2]=((v>>11)&0x1F)<<3;
}

int extractSmdhData(smdh_s* s, char* name, char* desc, u8* iconData)
{
	if(!s)return -1;
	if(s->header.magic!=0x48444D53)return -2;

	if(name)unicodeToChar(name, s->applicationTitles[1].shortDescription, 0x40);
	if(desc)unicodeToChar(name, s->applicationTitles[1].longDescription, 0x80);
	if(iconData)
	{
		u16* data=s->bigIconData;
		//convert RGB565 to RGB24
		int i, j, k;
		for(j=0; j<48; j+=8)
		{
			for(i=0; i<48; i+=8)
			{
				//parse tiling...
				for(k=0; k<8*8; k++)
				{
					u8 x=tileOrder[k]&0x7;
					u8 y=tileOrder[k]>>3;
					putPixel565(iconData, i+x, j+y, *data++);
				}
			}
		}
	}

	return 0;
}
