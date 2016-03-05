#ifndef MMAP_H
#define MMAP_H

#include <3ds.h>

#ifdef __cplusplus
extern "C" {
#endif

#define size_memmap(m) (sizeof(memorymap_header_t) + sizeof(memorymap_entry_t) * (m).header.num)

typedef struct
{
	u32 num;
	u32 text_end;
	u32 data_address;
	u32 data_size;
	u32 processLinearOffset;
	u32 processHookAddress;
	u32 processAppCodeAddress;
	u32 processHookTidLow, processHookTidHigh;
	u32 mediatype;
	bool capabilities[0x10]; // {socuAccess, csndAccess, qtmAccess, nfcAccess, httpcAccess, reserved...}
} memorymap_header_t;

typedef struct
{
	u32 src, dst, size;
} memorymap_entry_t;

typedef struct {
	memorymap_header_t header;
	memorymap_entry_t map[];
} memorymap_t;

memorymap_t* loadMemoryMap(char* path);

#ifdef __cplusplus
}
#endif

#endif
