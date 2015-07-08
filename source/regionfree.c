#include <3ds.h>

#include "regionfree.h"

bool regionFreeAvailable = false;
Handle nssHandle = 0;

Result regionFreeInit()
{
	Result ret = srvGetServiceHandle(&nssHandle, "ns:s");

	if(!ret)regionFreeAvailable=true;

	return ret;
}

Result regionFreeExit()
{
	svcCloseHandle(nssHandle);
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
