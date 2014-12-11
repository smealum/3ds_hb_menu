#include <3ds.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>

#include "error.h"
#include "filesystem.h"
#include "netloader.h"

#define DATA_BUFFER_SIZE (512*1024)
static u8   dataBuffer[DATA_BUFFER_SIZE];
static char errbuf[1024];

static int netloader_listenfd = -1;
static int netloader_datafd   = -1;

static void *SOC_buffer = NULL;

static void netloader_socket_error(const char *func, int err) {
	siprintf(errbuf, "  %s: err=%d", func, err);
	netloader_deactivate();
}

static void netloader_file_error(const char *func, int err) {
	siprintf(errbuf, "  %s: err=0x%08X", func, err);
	netloader_deactivate();
}

int netloader_draw_error(void) {
	drawError(GFX_BOTTOM, "Failure", errbuf);
	return 0;
}

int netloader_init(void) {
	SOC_buffer = memalign(0x1000, 0x100000);
	if(SOC_buffer == NULL)
		return -1;

	Result ret = SOC_Initialize(SOC_buffer, 0x100000);
	if(ret != 0)
	{
		free(SOC_buffer);
		SOC_buffer = NULL;
		return -1;
	}
	return 0;
}

int netloader_activate(void) {
	// create listening socket on all addresses on NETLOADER_PORT
	struct sockaddr_in serv_addr;

	netloader_listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(netloader_listenfd < 0)
	{
		netloader_socket_error("socket", SOC_GetErrno());
		return -1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(NETLOADER_PORT);

	int rc = bind(netloader_listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	if(rc != 0)
	{
		netloader_socket_error("bind", SOC_GetErrno());
		return -1;
	}

	int flags = fcntl(netloader_listenfd, F_GETFL);
	if(flags == -1)
	{
		netloader_socket_error("fcntl", SOC_GetErrno());
		return -1;
	}
	rc = fcntl(netloader_listenfd, F_SETFL, flags | O_NONBLOCK);
	if(rc != 0)
	{
		netloader_socket_error("fcntl", SOC_GetErrno());
		return -1;
	}

	rc = listen(netloader_listenfd, 10);
        if(rc != 0)
	{
		netloader_socket_error("listen", SOC_GetErrno());
		return -1;
	}

	return 0;
}

int netloader_deactivate(void) {
	// close all remaining sockets and allow mainloop to return to main menu
	if(netloader_listenfd >= 0)
	{
		closesocket(netloader_listenfd);
		netloader_listenfd = -1;
	}

	if(netloader_datafd >= 0)
	{
		closesocket(netloader_datafd);
		netloader_datafd = -1;
	}

	return 0;
}

int netloader_loop(void) {
	// can i haz client?
	if(netloader_listenfd >= 0 && netloader_datafd < 0)
	{
		netloader_datafd = accept(netloader_listenfd, (struct sockaddr*)NULL, NULL);
		if(netloader_datafd < 0)
		{
			int err = SOC_GetErrno();
			if(err != -EWOULDBLOCK && err != EWOULDBLOCK)
			{
				netloader_socket_error("accept", err);
				return -1;
			}
		}
		else
		{
			closesocket(netloader_listenfd);
			netloader_listenfd = -1;
			int flags = fcntl(netloader_datafd, F_GETFL);
			if(flags == -1)
			{
				netloader_socket_error("fcntl", SOC_GetErrno());
				return -1;
			}
			int rc = fcntl(netloader_datafd, F_SETFL, flags & ~O_NONBLOCK);
			if(rc != 0)
			{
				netloader_socket_error("fcntl", SOC_GetErrno());
				return -1;
			}
		}
	}

	// i can haz client!
	if(netloader_datafd >= 0)
	{
		// store file to sd
		Handle fileHandle;
		Result ret = FSUSER_OpenFile(NULL, &fileHandle, sdmcArchive,
		                             FS_makePath(PATH_CHAR, NETLOADER_TMPFILE),
		                             FS_OPEN_WRITE|FS_OPEN_CREATE, 0);
		if(ret != 0)
		{
			netloader_file_error("FSUSER_OpenFile", ret);
			return -1;
		}
		ret = FSFILE_SetSize(fileHandle, 0);
		if(ret != 0)
		{
			netloader_file_error("FSFILE_SetSize", ret);
			return -1;
		}

		u64     offset = 0;
		ssize_t rc;
		do
		{
			rc = recv(netloader_datafd, dataBuffer, DATA_BUFFER_SIZE, 0);
			if(rc < 0)
			{
				netloader_socket_error("recv", SOC_GetErrno());
				FSFILE_Close(fileHandle);
				return -1;
			}

			u32    bytes;
			size_t written = 0;
			while(written < rc)
			{
				ret = FSFILE_Write(fileHandle, &bytes, offset,
				                   (char*)dataBuffer + written,
				                   rc - written, FS_WRITE_FLUSH);
				if(ret != 0)
				{
					netloader_file_error("FSFILE_Write", ret);
					FSFILE_Close(fileHandle);
					return -1;
				}
				offset  += bytes;
				written += bytes;
			}
		} while(rc > 0);

		FSFILE_Close(fileHandle);

		netloader_deactivate();
		return 1;
	}

	return 0;
}

int netloader_exit(void) {
	Result ret = SOC_Shutdown();
	if(ret != 0)
		return -1;
	return 0;
}
