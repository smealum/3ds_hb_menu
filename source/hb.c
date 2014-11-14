#include <3ds.h>

#include "hb.h"

static Handle hbHandle;

Result initHb()
{
	return srvGetServiceHandle(&hbHandle, "hb:HB");
}

void exitHb()
{
	svcCloseHandle(hbHandle);
}

Result HB_GetBootloaderAddresses(void** load3dsx, void** setArgv)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00060000;

	if((ret = svcSendSyncRequest(hbHandle))!=0) return ret;

	if(load3dsx)*load3dsx=(void*)cmdbuf[2];
	if(setArgv)*setArgv=(void*)cmdbuf[3];
	
	return (Result)cmdbuf[1];
}
