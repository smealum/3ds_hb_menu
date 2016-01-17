#include <3ds.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>

#include <zlib.h>

#define ZLIB_CHUNK (16 * 1024)

#include "error.h"
#include "filesystem.h"
#include "netloader.h"

char *netloadedPath = NULL;
char *netloaded_commandline = NULL;
bool netloader_boot = false;
int netloaded_cmdlen = 0;

static char errbuf[1024];

static int netloader_listenfd = -1;
static int netloader_datafd   = -1;
static int netloader_udpfd = -1;


static void *SOC_buffer = NULL;


unsigned char in[ZLIB_CHUNK];
unsigned char out[ZLIB_CHUNK];

static void netloader_socket_error(const char *func, int err) {
	siprintf(errbuf, "  %s: err=%d", func, err);
	netloader_deactivate();
}

static char progress[256];

static int netloader_draw_progress(void) {
	char info[1024];
	sprintf(info, "Transferring: %s\n\n%s",netloadedPath,progress);
	drawError(GFX_BOTTOM, "NetLoader", info, 0);
	gfxFlushBuffers();
	gfxSwapBuffers();

	gspWaitForVBlank();

	return 0;
}

//---------------------------------------------------------------------------------
static int recvall(int sock, void *buffer, int size, int flags) {
//---------------------------------------------------------------------------------
	int len, sizeleft = size;

	while (sizeleft) {

		len = recv(sock,buffer,sizeleft,flags);

		if (len == 0) {
			size = 0;
			break;
		};

		if (len == -1) {

			if (errno != EAGAIN && errno != EWOULDBLOCK) {
				netloader_socket_error("recv", errno);
				break;
			}
		} else {
			sizeleft -=len;
			buffer +=len;
		}
	}
	return size;
}


//---------------------------------------------------------------------------------
static int decompress(int sock, FILE *fh, size_t filesize) {
//---------------------------------------------------------------------------------
	int ret;
	unsigned have;
	z_stream strm;
	size_t chunksize;

	/* allocate inflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit(&strm);
	if (ret != Z_OK) {
		netloader_socket_error("inflateInit failed.",ret);
		return ret;
	}

	size_t total = 0;
	/* decompress until deflate stream ends or end of file */
	do {

		int len = recvall(sock, &chunksize, 4, 0);

		if (len != 4) {
			(void)inflateEnd(&strm);
			netloader_socket_error("Error getting chunk size",len);
			return Z_DATA_ERROR;
		}

	strm.avail_in = recvall(sock,in,chunksize,0);

	if (strm.avail_in == 0) {
		(void)inflateEnd(&strm);
		netloader_socket_error("remote closed socket.",0);
		return Z_DATA_ERROR;
	}

	strm.next_in = in;

	/* run inflate() on input until output buffer not full */
	do {
		strm.avail_out = ZLIB_CHUNK;
		strm.next_out = out;
		ret = inflate(&strm, Z_NO_FLUSH);

		switch (ret) {

			case Z_NEED_DICT:
			ret = Z_DATA_ERROR;     /* and fall through */

			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
			case Z_STREAM_ERROR:
				(void)inflateEnd(&strm);
				netloader_socket_error("inflate error",ret);
				return ret;
		}

		have = ZLIB_CHUNK - strm.avail_out;

		if (fwrite(out, 1, have, fh) != have || ferror(fh)) {
			(void)inflateEnd(&strm);
			netloader_socket_error("file write error",0);
			return Z_ERRNO;
		}

		total += have;
		sprintf(progress,"%zu (%d%%)",total, (100 * total) / filesize);
		netloader_draw_progress();
	} while (strm.avail_out == 0);

	/* done when inflate() says it's done */
	} while (ret != Z_STREAM_END);

	/* clean up and return */
	(void)inflateEnd(&strm);
	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}


int netloader_draw_error(void) {
	drawError(GFX_BOTTOM, "Failure", errbuf, 0);
	return 0;
}

int netloader_init(void) {
	SOC_buffer = memalign(0x1000, 0x100000);
	if(SOC_buffer == NULL)
		return -1;

	Result ret = socInit(SOC_buffer, 0x100000);
	if(ret != 0)
	{
		// need to free the shared memory block if something goes wrong
		socExit();
		free(SOC_buffer);
		SOC_buffer = NULL;
		return -1;
	}
	return 0;
}

static int set_socket_nonblocking(int sock) {

	int flags = fcntl(sock, F_GETFL);

	if(flags == -1) return -1;

	int rc = fcntl(sock, F_SETFL, flags | O_NONBLOCK);

	if(rc != 0) return -1;

	return 0;
}

int netloader_activate(void) {
	struct sockaddr_in serv_addr;
	// create udp socket for broadcast ping
	netloader_udpfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (netloader_udpfd < 0)
	{
		netloader_socket_error("udp socket", errno );
		return -1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(NETLOADER_PORT);

	if(bind(netloader_udpfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
		netloader_socket_error("bind udp socket", errno );
		return -1;
	}

	if (set_socket_nonblocking(netloader_udpfd) == -1)
	{
		netloader_socket_error("listen fcntl", errno);
		return -1;
	}

	// create listening socket on all addresses on NETLOADER_PORT

	netloader_listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(netloader_listenfd < 0)
	{
		netloader_socket_error("socket", errno );
		return -1;
	}

	int rc = bind(netloader_listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	if(rc != 0)
	{
		netloader_socket_error("bind", errno);
		return -1;
	}

	if (set_socket_nonblocking(netloader_listenfd) == -1)
	{
		netloader_socket_error("listen fcntl", errno);
		return -1;
	}

	rc = listen(netloader_listenfd, 10);
		if(rc != 0)
	{
		netloader_socket_error("listen", errno);
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

	if(netloader_udpfd >= 0)
	{
		closesocket(netloader_udpfd);
		netloader_udpfd = -1;
	}

	return 0;
}

//---------------------------------------------------------------------------------
int load3DSX(int sock, u32 remote) {
//---------------------------------------------------------------------------------
	int len, namelen, filelen;
	char filename[256];
	len = recvall(sock, &namelen, 4, 0);

	if (len != 4) {
		netloader_socket_error("Error getting name length", errno);
		return -1;
	}

	len = recvall(sock, filename, namelen, 0);

	if (len != namelen) {
		netloader_socket_error("Error getting filename", errno);
		return -1;
	}

	filename[namelen] = 0;

	len = recvall(sock, &filelen, 4, 0);

	if (len != 4) {
		netloader_socket_error("Error getting file length",errno);
		return -1;
	}

	int response = 0;

	chdir("sdmc:/3ds/");

	int fd = open(filename,O_CREAT|O_WRONLY,ACCESSPERMS);

	if (fd < 0) {
		response = -1;
	} else {
		if (ftruncate(fd,filelen) == -1) {
			response = -2;
			netloader_socket_error("ftruncate",errno);
		}
	}

	send(sock,(int *)&response,sizeof(response),0);
	close(fd);

	netloadedPath=getcwd(NULL,0);
	strcat(netloadedPath,filename);

	FILE *file = fopen(filename,"wb");
	char *writebuffer=malloc(65536);
	setvbuf(file,writebuffer,_IOFBF, 65536);

	if (response == 0) {
		//printf("transferring %s\n%d bytes.\n", filename, filelen);

		if (decompress(sock,file,filelen)==Z_OK) {
			send(sock,(int *)&response,sizeof(response),0);
			//printf("\ntransferring command line\n");
			len = recvall(sock,(char*)&netloaded_cmdlen,4,0);
			if (netloaded_cmdlen) {
				netloaded_commandline = malloc(netloaded_cmdlen);
				len = recvall(sock, netloaded_commandline, netloaded_cmdlen,0);
			}
		} else {
			response = 1;
		}
	}

	free(netloadedPath);
	free(writebuffer);
	ftruncate(fileno(file), ftell(file));
	fclose(file);

	if (response == 0) {
		netloadedPath=getcwd(NULL,0);
		strcat(netloadedPath,filename);
		netloadedPath = strchr(netloadedPath,'/');
	}
	return response;
}

int netloader_loop(void) {

	struct sockaddr_in sa_udp_remote;
	char recvbuf[256];
	socklen_t fromlen = sizeof(sa_udp_remote);

	int len = recvfrom(netloader_udpfd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr*) &sa_udp_remote, &fromlen);

	if (len!=-1) {
		if (strncmp(recvbuf,"3dsboot",strlen("3dsboot")) == 0) {
			sa_udp_remote.sin_family=AF_INET;
			sa_udp_remote.sin_port=htons(17491);
			sendto(netloader_udpfd, "boot3ds", strlen("boot3ds"), 0, (struct sockaddr*) &sa_udp_remote,sizeof(sa_udp_remote));
		}
	}

	if(netloader_listenfd >= 0 && netloader_datafd < 0) {
		netloader_datafd = accept(netloader_listenfd, (struct sockaddr*)NULL, NULL);
		if(netloader_datafd < 0)
		{
			if(errno != -EWOULDBLOCK && errno != EWOULDBLOCK)
			{
				netloader_socket_error("accept", errno);
				return -1;
			}
		}
		else
		{
			closesocket(netloader_listenfd);
			netloader_listenfd = -1;
		}
	}

	if(netloader_datafd >= 0)
	{
		int result = load3DSX(netloader_datafd,0);
		netloader_deactivate();
		if (result== 0) return 1;
	}

	return 0;
}

int netloader_exit(void) {
	Result ret = socExit();
	if(ret != 0)
		return -1;
	return 0;
}
