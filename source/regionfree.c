#include <3ds.h>

#include "regionfree.h"

bool regionFreeAvailable = false;
bool regionFreeGamecardIn = false;
Handle nssHandle = 0;

smdh_s gamecardSmdh;
menuEntry_s gamecardMenuEntry;

Result regionFreeInit()
{
	Result ret = srvGetServiceHandle(&nssHandle, "ns:s");

	if(!ret)regionFreeAvailable = true;
	regionFreeGamecardIn = false;

	return ret;
}

Result regionFreeExit()
{
	return svcCloseHandle(nssHandle);
}

void regionFreeUpdate()
{
	if(!regionFreeAvailable)return;

	Result ret = loadGamecardIcon(&gamecardSmdh);

	regionFreeGamecardIn = (ret == 0);

	if(regionFreeGamecardIn)extractSmdhData(&gamecardSmdh, gamecardMenuEntry.name, gamecardMenuEntry.description, gamecardMenuEntry.author, gamecardMenuEntry.iconData);
}

Result loadGamecardIcon(smdh_s* out)
{
	if(!out)return -1;

	Handle fileHandle;
	static const u32 archivePath[] = {0x00000000, 0x00000000, 0x00000002, 0x00000000};
	static const u32 filePath[] = {0x00000000, 0x00000000, 0x00000002, 0x6E6F6369, 0x00000000};	
	Result ret = FSUSER_OpenFileDirectly(&fileHandle, (FS_Archive){ARCHIVE_SAVEDATA_AND_CONTENT, (FS_Path){PATH_BINARY, 0x10, (u8*)archivePath}}, (FS_Path){PATH_BINARY, 0x14, (u8*)filePath}, FS_OPEN_READ, 0);
	if(ret)return ret;

	u32 bytesRead;
	ret = FSFILE_Read(fileHandle, &bytesRead, 0x0, out, sizeof(smdh_s));

	FSFILE_Close(fileHandle);

	return ret;
}

Result NSS_Reboot(u32 pid_low, u32 pid_high, u8 mediatype, u8 flag)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00100180;
	cmdbuf[1] = flag;
	cmdbuf[2] = pid_low;
	cmdbuf[3] = pid_high;
	cmdbuf[4] = mediatype;
	cmdbuf[5] = 0x00000000;
	cmdbuf[6] = 0x00000000;

	if((ret = svcSendSyncRequest(nssHandle))!=0) return ret;

	return (Result)cmdbuf[1];
}

Result regionFreeRun()
{
	Result ret = NSS_Reboot(0x00000000, 0x00000000, 0x2, 0x1);

	regionFreeExit();

	return ret;
}
