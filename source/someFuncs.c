#include <sys/iosupport.h>
#include <3ds/types.h>
#include <3ds/svc.h>

// These are supposed to be integrated into ctrulib (and actually implemented properly)

extern void* fake_heap_start;
extern void* fake_heap_end;

void __ctru_exit(int rc)
{
	for (;;);
}

void initSystem()
{
	__syscalls.exit = __ctru_exit;

	u32 blockAddr;
	svcControlMemory(&blockAddr, 0x08000000, 0x0, 0x1910000, MEMOP_ALLOC, 0x3);
	fake_heap_start=(void*)0x08000000;
	fake_heap_end=fake_heap_start+0x01910000;
}
