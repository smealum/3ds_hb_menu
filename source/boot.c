#include <3ds.h>

#include <stdio.h>
#include <string.h>

#include "boot.h"
#include "netloader.h"
#include "filesystem.h"

extern void (*__system_retAddr)(void);

static Handle hbFileHandle;
static u32 argbuffer[0x200];

// ninjhax 1.x
void (*callBootloader_1x)(Handle hb, Handle file);
void (*setArgs_1x)(u32* src, u32 length);

static void launchFile_1x(void)
{
	// jump to bootloader
	callBootloader_1x(0x00000000, hbFileHandle);
}

// ninjhax 2.0+
void (*callBootloader_2x)(Handle file, u32* argbuf, u32 arglength) = (void*)0x00100000;

static void launchFile_2x(void)
{
	// jump to bootloader
	callBootloader_2x(hbFileHandle, argbuffer, 0x200*4);
}

int bootApp(char* executablePath)
{
	// open file that we're going to boot up
	fsInit();
	FSUSER_OpenFileDirectly(NULL, &hbFileHandle, sdmcArchive, FS_makePath(PATH_CHAR, executablePath), FS_OPEN_READ, FS_ATTRIBUTE_NONE);
	fsExit();

	// set argv/argc
	argbuffer[0]=0;
	if(netloader_boot) {
		char *ptr = netloaded_commandline;
		char *dst = (char*)&argbuffer[1];
		while (ptr < netloaded_commandline + netloaded_cmdlen) {
			char *arg = ptr;
			strcpy(dst,ptr);
			ptr += strlen(arg) + 1;
			dst += strlen(arg) + 1;
			argbuffer[0]++;
		}
	}else{
		argbuffer[0]=1;
		snprintf((char*)&argbuffer[1], 0x200*4, "sdmc:%s", executablePath);
	}

	// figure out the preferred way of running the 3dsx
	if(!hbInit())
	{
		// ninjhax 1.x !
		// grab bootloader addresses
		HB_GetBootloaderAddresses((void**)&callBootloader_1x, (void**)&setArgs_1x);
		hbExit();

		// set argv
		setArgs_1x(argbuffer, 0x200*4);

		// override return address to homebrew booting code
		__system_retAddr = launchFile_1x;
	}else{
		// ninjhax 2.0+
		// override return address to homebrew booting code
		__system_retAddr = launchFile_2x;
	}

	return 0;
}
