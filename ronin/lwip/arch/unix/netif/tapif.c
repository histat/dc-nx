/*
 * Copyright (c) 2001, Swedish Institute of Computer Science.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: tapif.c,v 1.1 2003/03/17 21:57:54 marcus Exp $
 */

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>


#include "lwip/debug.h"

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/ip.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"

#include "netif/arp.h"

#ifdef linux
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#define DEVTAP "/dev/net/tun"
#else  /* linux */
#define DEVTAP "/dev/tap0"
#endif /* linux */

#define IFNAME0 't'
#define IFNAME1 'p'

static const struct eth_addr ethbroadcast = {{0xff,0xff,0xff,0xff,0xff,0xff}};

struct tapif {
  struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */
  int fd;
};

/* Forward declarations. */
static void  tapif_input(struct netif *netif);
static err_t tapif_output(struct netif *netif, struct pbuf *p,
			       struct ip_addr *ipaddr);

static void tapif_thread(void *data);

/*-----------------------------------------------------------------------------------*/
static void
low_level_init(struct netif *netif)
{
  struct tapif *tapif;
  char buf[100];

  tapif = netif->state;
  
  /* Obtain MAC address from network interface. */

  /* (We just fake an address...) */
  tapif->ethaddr->addr[0] = 0x1;
  tapif->ethaddr->addr[1] = 0x2;
  tapif->ethaddr->addr[2] = 0x3;
  tapif->ethaddr->addr[3] = 0x4;
  tapif->ethaddr->addr[4] = 0x5;
  tapif->ethaddr->addr[5] = 0x6;

  /* Do whatever else is needed to initialize interface. */
  
  tapif->fd = open(DEVTAP, O_RDWR);
  DEBUGF(TAPIF_DEBUG, ("tapif_init: fd %d\n", tapif->fd));
  if(tapif->fd == -1) {
    perror("tapif_init");
    exit(1);
  }

#ifdef linux
  {
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TAP|IFF_NO_PI;
    if (ioctl(tapif->fd, TUNSETIFF, (void *) &ifr) < 0) {
      perror(buf);
      exit(1);
    }
  }
#endif /* Linux */

  snprintf(buf, sizeof(buf), "ifconfig tap0 inet %d.%d.%d.%d",
           ip4_addr1(&(netif->gw)),
           ip4_addr2(&(netif->gw)),
           ip4_addr3(&(netif->gw)),
           ip4_addr4(&(netif->gw)));
  
  DEBUGF(TAPIF_DEBUG, ("tapif_init: system(\"%s\");\n", buf));
  system(buf);
  sys_thread_new(tapif_thread, netif);

}
/*-----------------------------------------------------------------------------------*/
/*
 * low_level_output():
 *
 * Should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 */
/*-----------------------------------------------------------------------------------*/

static err_t
low_level_output(struct tapif *tapif, struct pbuf *p)
{
  struct pbuf *q;
  char buf[1500];
  char *bufptr;
  
  /* initiate transfer(); */
  
  bufptr = &buf[0];
  
  for(q = p; q != NULL; q = q->next) {
    /* Send the data from the pbuf to the interface, one pbuf at a
       time. The size of the data in each pbuf is kept in the ->len
       variable. */    
    /* send data from(q->payload, q->len); */
    bcopy(q->payload, bufptr, q->len);
    bufptr += q->len;
  }

  /* signal that packet should be sent(); */
  if(write(tapif->fd, buf, p->tot_len) == -1) {
    perror("tapif: write");
  }
  return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
/*
 * low_level_input():
 *
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 */
/*-----------------------------------------------------------------------------------*/
static struct pbuf *
low_level_input(struct tapif *tapif)
{
  struct pbuf *p, *q;
  u16_t len;
  char buf[1500];
  char *bufptr;

  /* Obtain the size of the packet and put it into the "len"
     variable. */
  len = read(tapif->fd, buf, sizeof(buf));

  /*  if(((double)rand()/(double)RAND_MAX) < 0.1) {
    printf("drop\n");
    return NULL;
    }*/

  
  /* We allocate a pbuf chain of pbufs from the pool. */
  p = pbuf_alloc(PBUF_LINK, len, PBUF_POOL);
  
  if(p != NULL) {
    /* We iterate over the pbuf chain until we have read the entire
       packet into the pbuf. */
    bufptr = &buf[0];
    for(q = p; q != NULL; q = q->next) {
      /* Read enough bytes to fill this pbuf in the chain. The
         avaliable data in the pbuf is given by the q->len
         variable. */
      /* read data into(q->payload, q->len); */
      bcopy(bufptr, q->payload, q->len);
      bufptr += q->len;
    }
    /* acknowledge that packet has been read(); */
  } else {
    /* drop packet(); */
  }

  return p;  
}
/*-----------------------------------------------------------------------------------*/
static void 
tapif_thread(void *arg)
{
  struct netif *netif;
  struct tapif *tapif;
  fd_set fdset;
  int ret;
  
  netif = arg;
  tapif = netif->state;
  
  while(1) {
    FD_ZERO(&fdset);
    FD_SET(tapif->fd, &fdset);

    /* Wait for a packet to arrive. */
    ret = select(tapif->fd + 1, &fdset, NULL, NULL, NULL);

    if(ret == 1) {
      /* Handle incoming packet. */
      tapif_input(netif);
    } else if(ret == -1) {
      perror("tapif_thread: select");
    }
  }
}
/*-----------------------------------------------------------------------------------*/
/*
 * tapif_output():
 *
 * This function is called by the TCP/IP stack when an IP packet
 * should be sent. It calls the function called low_level_output() to
 * do the actuall transmission of the packet.
 *
 */
/*-----------------------------------------------------------------------------------*/
static err_t
tapif_output(struct netif *netif, struct pbuf *p,
		  struct ip_addr *ipaddr)
{
  struct tapif *tapif;
  struct pbuf *q;
  struct eth_hdr *ethhdr;
  struct eth_addr *dest, mcastaddr;
  struct ip_addr *queryaddr;
  err_t err;
  u8_t i;
  
  tapif = netif->state;

  /* Make room for Ethernet header. */
  if(pbuf_header(p, sizeof(struct eth_hdr)) != 0) {
    /* The pbuf_header() call shouldn't fail, but we allocate an extra
       pbuf just in case. */
    q = pbuf_alloc(PBUF_LINK, sizeof(struct eth_hdr), PBUF_RAM);
    if(q == NULL) {
      return ERR_MEM;
    }
    pbuf_chain(q, p);
    p = q;
  }

  /* Construct Ethernet header. Start with looking up deciding which
     MAC address to use as a destination address. Broadcasts and
     multicasts are special, all other addresses are looked up in the
     ARP table. */
  queryaddr = ipaddr;
  if(ip_addr_isany(ipaddr) ||
     ip_addr_isbroadcast(ipaddr, &(netif->netmask))) {
    dest = (struct eth_addr *)&ethbroadcast;
  } else if(ip_addr_ismulticast(ipaddr)) {
    /* Hash IP multicast address to MAC address. */
    mcastaddr.addr[0] = 0x01;
    mcastaddr.addr[1] = 0x0;
    mcastaddr.addr[2] = 0x5e;
    mcastaddr.addr[3] = ip4_addr2(ipaddr) & 0x7f;
    mcastaddr.addr[4] = ip4_addr3(ipaddr);
    mcastaddr.addr[5] = ip4_addr4(ipaddr);
    dest = &mcastaddr;
  } else {
    if(ip_addr_maskcmp(ipaddr, &(netif->ip_addr), &(netif->netmask))) {
      /* Use destination IP address if the destination is on the same
         subnet as we are. */
      queryaddr = ipaddr;
    } else {
      /* Otherwise we use the default router as the address to send
         the Ethernet frame to. */
      queryaddr = &(netif->gw);
    }
    dest = arp_lookup(queryaddr);
  }


  /* If the arp_lookup() didn't find an address, we send out an ARP
     query for the IP address. */
  if(dest == NULL) {
    q = arp_query(netif, tapif->ethaddr, queryaddr);
    if(q != NULL) {
      printf("Sending ARP after query\n");
      err = low_level_output(tapif, q);
      pbuf_free(q);
      return err;
    }
    return ERR_MEM;
  }
  ethhdr = p->payload;
  
  for(i = 0; i < 6; i++) {
    ethhdr->dest.addr[i] = dest->addr[i];
    ethhdr->src.addr[i] = tapif->ethaddr->addr[i];
  }
  
  ethhdr->type = htons(ETHTYPE_IP);
  
  return low_level_output(tapif, p);

}
/*-----------------------------------------------------------------------------------*/
/*
 * tapif_input():
 *
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface.
 *
 */
/*-----------------------------------------------------------------------------------*/
static void
tapif_input(struct netif *netif)
{
  struct tapif *tapif;
  struct eth_hdr *ethhdr;
  struct pbuf *p;


  tapif = netif->state;
  
  p = low_level_input(tapif);

  if(p == NULL) {
    DEBUGF(TAPIF_DEBUG, ("tapif_input: low_level_input returned NULL\n"));
    return;
  }
  ethhdr = p->payload;

  switch(htons(ethhdr->type)) {
  case ETHTYPE_IP:
    DEBUGF(TAPIF_DEBUG, ("tapif_input: IP packet\n"));
    arp_ip_input(netif, p);
    pbuf_header(p, -14);
    if(ip_lookup(p->payload, netif)) {
      netif->input(p, netif);
    } else {
      printf("tapif_input: lookup failed!\n");
    }
    break;
  case ETHTYPE_ARP:
    DEBUGF(TAPIF_DEBUG, ("tapif_input: ARP packet\n"));
    p = arp_arp_input(netif, tapif->ethaddr, p);
    if(p != NULL) {
      DEBUGF(TAPIF_DEBUG, ("tapif_input: Sending ARP reply\n"));
      low_level_output(tapif, p);
      pbuf_free(p);
    }
    break;
  default:
    pbuf_free(p);
    break;
  }
}
/*-----------------------------------------------------------------------------------*/
static void
arp_timer(void *arg)
{
  arp_tmr();
  sys_timeout(ARP_TMR_INTERVAL, (sys_timeout_handler)arp_timer, NULL);
}
/*-----------------------------------------------------------------------------------*/
/*
 * tapif_init():
 *
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 */
/*-----------------------------------------------------------------------------------*/
void
tapif_init(struct netif *netif)
{
  struct tapif *tapif;
    
  tapif = mem_malloc(sizeof(struct tapif));
  netif->state = tapif;
  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  netif->output = tapif_output;
  netif->linkoutput = low_level_output;
  
  tapif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);
  
  low_level_init(netif);
  arp_init();
  
  sys_timeout(ARP_TMR_INTERVAL, (sys_timeout_handler)arp_timer, NULL);
}
/*-----------------------------------------------------------------------------------*/
