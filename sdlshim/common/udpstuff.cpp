
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>

#include "basics.h"
#include "tcpstuff.h"
#include "udpstuff.h"
#include "udpstuff.fdh"


uint32_t udp_createsocket(uint16_t port)
{
uint s;
struct sockaddr_in host_address;
	
	// create the socket
	s = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s <= 0)
	{
		staterr("udp_create_socket: failed to create UDP socket on port %d", port);
		return 0;
	}
	
	// init the host_address the socket is beeing bound to
	memset((void*)&host_address, 0, sizeof(host_address));
	
	host_address.sin_family = PF_INET;
	host_address.sin_addr.s_addr = INADDR_ANY;
	host_address.sin_port = htons(port);
	
	if (bind(s, (struct sockaddr*)&host_address, sizeof(host_address)) < 0)
	{
		staterr("udp_create_socket: failed to bind UDP socket to port %d", port);
		close(s);
		return 0;
	}
	
	return s;
}

uint32_t udp_createsocket_anyport(uint16_t *port_out)
{
uint16_t port = 65530;
uint32_t sock;

	while(port > 10000)
	{
		sock = udp_createsocket(port);
		if (sock)
		{
			if (port_out) *port_out = port;
			return sock;
		}
		
		port--;
	}
	
	if (port_out) *port_out = 0;
	return 0;
}

/*
void c------------------------------() {}
*/

void udp_send(uint32_t sock, const uint8_t *data, int len, uint32_t ip, uint16_t port)
{
struct sockaddr_in sain;

	if (chkwrite(sock))		// prevent potential lock-ups if outgoing buffer is full
	{
		sain.sin_addr.s_addr = htonl(ip);
		sain.sin_port = htons(port);
		sain.sin_family = AF_INET;
		
		sendto(sock, data, len, 0, \
			(struct sockaddr *)&sain, sizeof(struct sockaddr));
	}
}


int udp_recv(uint32_t sock, uint8_t *buffer, int buffer_size, \
			uint32_t *ip_out, uint16_t *port_out)
{
struct sockaddr_in sain;
socklen_t sain_size = sizeof(sain);
int len;

	len = recvfrom(sock, buffer, buffer_size, 0,
			(struct sockaddr *)&sain, &sain_size);
	
	if (ip_out)
		*ip_out = ntohl(sain.sin_addr.s_addr);
	
	if (port_out)
		*port_out = ntohs(sain.sin_port);
	
	return len;
}




