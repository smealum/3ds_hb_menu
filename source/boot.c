#include <3ds.h>

#include <stdio.h>
#include <string.h>

#include "boot.h"
#include "netloader.h"
#include "filesystem.h"

extern void (*__system_retAddr)(void);

static Handle hbFileHandle;
static u32 argbuffer[0x200];
static u32 argbuffer_length = 0;

// ninjhax 1.x
void (*callBootloader_1x)(Handle hb, Handle file);
void (*setArgs_1x)(u32* src, u32 length);

static void launchFile_1x(void)
{
	// jump to bootloader
	callBootloader_1x(0x00000000, hbFileHandle);
}

// ninjhax 2.0+
typedef struct
{
	int processId;
	bool capabilities[0x10];
}processEntry_s;

void (*callBootloader_2x)(Handle file, u32* argbuf, u32 arglength) = (void*)0x00100000;
void (*callBootloaderNewProcess_2x)(int processId, u32* argbuf, u32 arglength) = (void*)0x00100008;
void (*getBestProcess_2x)(u32 sectionSizes[3], bool* requirements, int num_requirements, processEntry_s* out, int out_size, int* out_len) = (void*)0x0010000C;

int targetProcessId = -1;

static void launchFile_2x(void)
{
	// jump to bootloader
	if(targetProcessId < 0)callBootloader_2x(hbFileHandle, argbuffer, argbuffer_length);
	else callBootloaderNewProcess_2x(targetProcessId, argbuffer, argbuffer_length);
}

int bootApp(char* executablePath, executableMetadata_s* em)
{
	// open file that we're going to boot up
	fsInit();
	FSUSER_OpenFileDirectly(NULL, &hbFileHandle, sdmcArchive, FS_makePath(PATH_CHAR, executablePath), FS_OPEN_READ, FS_ATTRIBUTE_NONE);
	fsExit();

	// set argv/argc
	argbuffer[0] = 0;
	argbuffer_length = 0x200*4;
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
		snprintf((char*)&argbuffer[1], 0x200*4 - 4, "sdmc:%s", executablePath);
		argbuffer_length = strlen((char*)&argbuffer[1]) + 4 + 1; // don't forget null terminator !
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

		targetProcessId = -1;

		if(em && em->scanned)
		{
			processEntry_s out[4];
			int out_len = 0;
			getBestProcess_2x(em->sectionSizes, em->servicesThatMatter, 3, out, 4, &out_len);

			// temp
			targetProcessId = out[0].processId;
		}
	}

	return 0;
}
