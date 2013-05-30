
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <fcntl.h>

#include "tcpstuff.h"
#include "tcpstuff.fdh"

#define uchar		uint8_t
struct sockaddr_in sain;


int senddata(uint32_t sock, const uint8_t *packet, int len)
{
	return send(sock, packet, len, 0);
}

int sendstr(uint32_t sock, const char *str)
{
	return senddata(sock, (const uint8_t *)str, strlen(str));
}

int sendnow(uint32_t sock, const uint8_t *packet, int len)
{
int ok;
	net_nodelay(sock, true);
	ok = senddata(sock, packet, len);
	net_nodelay(sock, false);
	return ok;
}

void net_flush(uint32_t sock)
{
	net_nodelay(sock, true);
	net_nodelay(sock, false);
}

/*
void c------------------------------() {}
*/

int chkread(uint32_t sock)
{
fd_set readfds;
struct timeval poll;

	FD_ZERO(&readfds);
	FD_SET(sock, &readfds);
	
	memset((char *)&poll, 0, sizeof(poll));
	return select(sock+1, &readfds, (fd_set *)0, (fd_set *)0, &poll);
}

int chkwrite(uint32_t sock)
{
fd_set writefds;
struct timeval poll;

	FD_ZERO(&writefds);
	FD_SET(sock, &writefds);
	
	memset((char *)&poll, 0, sizeof(poll));
	return select(sock+1, (fd_set *)0, &writefds, (fd_set *)0, &poll);
}

/*
void c------------------------------() {}
*/

bool net_setnonblock(uint32_t sock, bool enable)
{
long fvalue;

	// Set non-blocking
	if ((fvalue = fcntl(sock, F_GETFL, NULL)) < 0)
	{
		staterr("net_setnonblock: Error fcntl(..., F_GETFL) (%s)", strerror(errno));
		return 1;
	}
	
	if (enable)
		fvalue |= O_NONBLOCK;
	else
		fvalue &= ~O_NONBLOCK;
	
	if (fcntl(sock, F_SETFL, fvalue) < 0)
	{
		staterr("net_setnonblock: Error fcntl(..., F_SETFL) (%s)", strerror(errno));
		return 1;
	}
	
	return 0;
}

bool net_nodelay(uint32_t sock, int flag)
{
	return setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag));
}

/*
void c------------------------------() {}
*/

char *decimalip(uint32_t ip)
{
static char buffer[80];
	sprintf(buffer, "%d.%d.%d.%d",
				(ip>>24)&255, (ip>>16)&255, (ip>>8)&255, ip&255);
	return buffer;
}


uint32_t net_dnslookup(const char *host)
{
struct hostent *hosten;
uint32_t ip;

	// attempt to resolve the hostname via DNS
	hosten = gethostbyname(host);
	if (hosten == NULL)
	{
		staterr("dnslookup: failed to resolve host: '%s'.", host);
		return 0;
	}
	else
	{
		memcpy((char*)&ip, hosten->h_addr_list[0], 4);
		return ntohl(ip);
	}
}

/*
void c------------------------------() {}
*/

// attempts to connect to ip:port, and returns the new socket number if successful,
// else returns 0.
uint32_t net_connect(uint32_t ip, uint16_t port, int timeout_ms)
{
int conn_socket, result;
bool connected;
bool use_timeout;

	if (!(conn_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)))
	{
		staterr("connect_tcp: Failed to create socket.");
		return 1;
	}
	
	net_setnonblock(conn_socket, true);
	
	sain.sin_addr.s_addr = htonl(ip);
	sain.sin_port = htons(port);
	sain.sin_family = AF_INET;
	
	#define TICK_BASE		10
	use_timeout = (timeout_ms != 0);
	connected = false;
	
	for(;;)
	{
		result = connect(conn_socket, (struct sockaddr *)&sain, sizeof(struct sockaddr_in));
		
		if (errno == EISCONN || result != -1)
		{
			connected = true;
			break;
		}
		
		if (errno == EINTR) break;
		
		if (use_timeout)
		{
			if (timeout_ms >= 0)
			{
				usleep(TICK_BASE * 1000);
				timeout_ms -= TICK_BASE;
			}
			else break;
		}
    }

	net_setnonblock(conn_socket, false);

    if (connected)
		return conn_socket;
	
	close(conn_socket);
	return 0;
}


/*
void c------------------------------() {}
*/

// create a socket and have it listen on all available interfaces.
uint32_t net_open_and_listen(uint16_t port, bool reuse_addr)
{
int sock;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (!sock)
	{
		staterr("net_open_and_listen: failed to create socket!");
		return 0;
	}
	
	if (reuse_addr)
	{
		// prevent port from getting blocked if server is shut down unexpectandly
		int	on = 1;
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
	}
	
	if (net_listen_socket(sock, INADDR_ANY, port))
	{
		close(sock);
		return 0;
	}
	
	return sock;
}

// set a socket to listen on the given IP and port
char net_listen_socket(uint32_t sock, uint32_t listen_ip, uint16_t listen_port)
{
sockaddr_in thesock;

	thesock.sin_addr.s_addr = htonl(listen_ip);
	thesock.sin_port = htons(listen_port);
	thesock.sin_family = AF_INET;
	
	if (bind(sock, (struct sockaddr *)&thesock, sizeof(sockaddr_in)))
	{
		staterr("bind failed to %s:%d!", decimalip(listen_ip), listen_port);
		return 1;
	}
	
	if (listen(sock, 50))
	{
		staterr("listen failed!");
		return 1;
	}
	
	stat("bind success to %s:%d", decimalip(listen_ip), listen_port);
	return 0;
}

// accepts an incoming connection on a socket and returns the new socket.
// if remote_ip is non-NULL, it is written with the IP of the connecting host.
uint32_t net_accept(uint32_t s, uint32_t *connected_ip)
{
uint32_t newsocket;
struct sockaddr_in sockinfo;
socklen_t sz = sizeof(sockinfo);

	newsocket = accept(s, (struct sockaddr *)&sockinfo, &sz);
	
	if (connected_ip)
	{
		if (newsocket)
		{
			*connected_ip = ntohl(sockinfo.sin_addr.s_addr);
		}
		else
		{
			*connected_ip = 0;
		}
	}
	
	return newsocket;
}

/*
void c------------------------------() {}
*/


