#pragma once

#include <3ds/types.h>

extern const s32 costable[];

static inline s32 pcCos(u16 v)
{
	return costable[v&0x1FF];
}
