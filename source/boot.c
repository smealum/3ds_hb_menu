#include <3ds.h>

#include <stdio.h>
#include <string.h>

#include "boot.h"
#include "netloader.h"
#include "filesystem.h"

extern void (*__system_retAddr)(void);

static Handle hbFileHandle;
static u32 argbuffer[0x100];
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
void (*callBootloaderRunTitle_2x)(u8 mediatype, u32* argbuf, u32 argbuflength, u32 tid_low, u32 tid_high) = (void*)0x00100010;
void (*getBestProcess_2x)(u32 sectionSizes[3], bool* requirements, int num_requirements, processEntry_s* out, int out_size, int* out_len) = (void*)0x0010000C;

int targetProcessId = -1;
titleInfo_s target_title;

static void launchFile_2x(void)
{
	// jump to bootloader
	if(targetProcessId == -1)callBootloader_2x(hbFileHandle, argbuffer, argbuffer_length);
	else if(targetProcessId == -2)callBootloaderRunTitle_2x(target_title.mediatype, argbuffer, argbuffer_length, target_title.title_id & 0xffffffff, (target_title.title_id >> 32) & 0xffffffff);
	else callBootloaderNewProcess_2x(targetProcessId, argbuffer, argbuffer_length);
}

bool isNinjhax2(void)
{
	Result ret = hbInit();
	if(!ret)
	{
		hbExit();
		return false;
	}else return true;
}

int bootApp(char* executablePath, executableMetadata_s* em, char* arg)
{
	// set argv/argc
	argbuffer[0] = 0;
	argbuffer_length = sizeof(argbuffer);

	if (!netloader_boot)
	{
		argbuffer[0] = 1;
		snprintf((char*)&argbuffer[1], sizeof(argbuffer) - 4, "sdmc:%s", executablePath);
	}

	{
		char *ptr = netloaded_commandline;
		char *dst = (char*)&argbuffer[1];
		if (!netloader_boot)
			dst += strlen(dst) + 1; // skip first argument

		if(arg && *arg)
		{

			char c, *pstr, *str=arg, *endarg = arg+strlen(arg);

			do
			{
				do
				{
					c = *str++;
				} while ((c == ' ' || c == '\t') && str < endarg);

				pstr = str-1;

				if (c == '\"')
				{
					pstr++;
					while(*str++ != '\"' && str < endarg);
				}
				else
				{
					do
					{
						c = *str++;
					} while (c != ' ' && c != '\t' && str < endarg);
				}

				if (str<endarg)
				{
					*(--str) = '\0';
					str++;
				}
				strcpy(dst, pstr);
				dst += strlen(dst) + 1;
				argbuffer[0]++;

			} while(str<endarg);

		}
		
		if(netloader_boot)
		{
			while (ptr < netloaded_commandline + netloaded_cmdlen)
			{
				int n = strlen(ptr);
				strcpy(dst, ptr);
				ptr += n + 1;
				dst += n + 1;
				argbuffer[0]++;
			}
		}
		
		argbuffer_length = (int)((void*)dst - (void*)argbuffer);
	}

	// open file that we're going to boot up
	fsInit();
	FSUSER_OpenFileDirectly(&hbFileHandle, sdmcArchive, fsMakePath(PATH_ASCII, executablePath), FS_OPEN_READ, 0);
	fsExit();

	// figure out the preferred way of running the 3dsx
	if(!hbInit())
	{
		// ninjhax 1.x !
		// grab bootloader addresses
		HB_GetBootloaderAddresses((void**)&callBootloader_1x, (void**)&setArgs_1x);
		hbExit();

		// set argv
		setArgs_1x(argbuffer, sizeof(argbuffer));

		// override return address to homebrew booting code
		__system_retAddr = launchFile_1x;
	}else{
		// ninjhax 2.0+
		// override return address to homebrew booting code
		__system_retAddr = launchFile_2x;

		if(em)
		{
			if(em->scanned && targetProcessId == -1)
			{
				// this is a really shitty implementation of what we should be doing
				// i'm really too lazy to do any better right now, but a good solution will come
				// (some day)
				processEntry_s out[4];
				int out_len = 0;
				getBestProcess_2x(em->sectionSizes, (bool*)em->servicesThatMatter, NUM_SERVICESTHATMATTER, out, 4, &out_len);

				// temp : check if we got all the services we want
				if(em->servicesThatMatter[0] <= out[0].capabilities[0] && em->servicesThatMatter[1] <= out[0].capabilities[1] && em->servicesThatMatter[2] <= out[0].capabilities[2] && em->servicesThatMatter[3] <= out[0].capabilities[3] && em->servicesThatMatter[4] <= out[0].capabilities[4])
				{
					targetProcessId = out[0].processId;
				}else{
					// temp : if we didn't get everything we wanted, we search for the candidate that has as many highest-priority services as possible
					int i, j;
					int best_id = 0;
					int best_sum = 0;
					for(i=0; i<out_len; i++)
					{
						int sum = 0;
						for(j=0; j<NUM_SERVICESTHATMATTER; j++)
						{
							sum += (em->servicesThatMatter[j] == 1) && out[i].capabilities[j];
						}

						if(sum > best_sum)
						{
							best_id = i;
							best_sum = sum;
						}
					}
					targetProcessId = out[best_id].processId;
				}

			}else if(targetProcessId != -1) targetProcessId = -2;
		}
	}

	return 0;
}
