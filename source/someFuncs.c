#include <sys/iosupport.h>

// These are supposed to be integrated into ctrulib (and actually implemented properly)

void __ctru_exit(int rc)
{
	for (;;);
}

void initSystem()
{
	__syscalls.exit = __ctru_exit;
}
