#include <3ds.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <malloc.h>

#include "filesystem.h"
#include "netloader.h"

#define DATA_BUFFER_SIZE (512*1024)
static u32 dataBuffer[DATA_BUFFER_SIZE/4];

bool netloader_active = false;
bool netloader_boot = false;
int netloader_listenfd = -1;
int netloader_datafd = -1;

unsigned long htonl(unsigned long v)
{
	u8* v2=(u8*)&v;
	return (v2[0]<<24)|(v2[1]<<16)|(v2[2]<<8)|(v2[3]);
}

unsigned short htons(unsigned short v)
{
	u8* v2=(u8*)&v;
	return (v2[0]<<8)|(v2[1]);
}

void netloader_init() {
	SOC_Initialize((u32*)memalign(0x1000, 0x100000), 0x100000);
}

void netloader_activate() {
	// create listening socket on all addresses on NETLOADER_PORT
	struct sockaddr_in serv_addr;

	netloader_listenfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(NETLOADER_PORT);

	bind(netloader_listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	fcntl(netloader_listenfd, F_SETFL, O_NONBLOCK);

	listen(netloader_listenfd, 10);
}

void netloader_deactivate() {
	// close all remaining sockets and allow mainloop to return to main menu
	if(netloader_listenfd>=0)
	{
		closesocket(netloader_listenfd);
		netloader_listenfd=-1;
	}
	if(netloader_datafd>=0)
	{
		closesocket(netloader_datafd);
		netloader_datafd=-1;
	}
}

int netloader_loop() {
	// can i haz client?
	if(netloader_listenfd>=0 && netloader_datafd<0)
	{
		if((netloader_datafd = accept(netloader_listenfd, (struct sockaddr*)NULL, NULL))>0)
		{
			closesocket(netloader_listenfd);
			netloader_listenfd=-1;
		}
	}

	// i can haz client!
	if(netloader_datafd>=0)
	{
		// check if temporary file already exists and remove
		if(fileExists(NETLOADER_TMPFILE, &sdmcArchive))
		{
			FSUSER_DeleteFile(NULL, sdmcArchive, FS_makePath(PATH_CHAR, NETLOADER_TMPFILE));
		}

		// store file to sd
		Handle fileHandle;
		FSUSER_OpenFile(NULL, &fileHandle, sdmcArchive, FS_makePath(PATH_CHAR, NETLOADER_TMPFILE), FS_OPEN_WRITE|FS_OPEN_CREATE, 0);
		int ret;
		u32 totalSize=0;
		// TODO: this seems to fail if the delay between opening the socket and retrieving the first byte is too long,
		// resulting in corrupted 3dsx file.
		while((ret=recv(netloader_datafd, dataBuffer, DATA_BUFFER_SIZE, 0))>0)
		{
			FSFILE_Write(fileHandle, (u32*)&ret, totalSize, (u32*)dataBuffer, ret, 0x10001);totalSize+=ret;
		}
		FSFILE_Close(fileHandle);
		svcCloseHandle(fileHandle);

		closesocket(netloader_datafd);
		return 1;
	}
	return 0;
}

void netloader_exit() {
	SOC_Shutdown();
}
