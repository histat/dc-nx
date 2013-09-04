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
 * $Id: cs8900if.c,v 1.1 2003/03/17 21:57:53 marcus Exp $
 */

/*
 * This is a device driver for the Crystal Semiconductor CS8900
 * chip. The CS8900 specific code in this file is written by Mikael
 * Lundberg for the EIS2001 student project at Luleå University,
 * Sweden.
 */

#include "lwip/debug.h"

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/stats.h"
#include "lwip/sys.h"

#include "netif/arp.h"

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 't'

struct cs8900if {
  struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */
};

static const struct eth_addr ethbroadcast = {{0xff,0xff,0xff,0xff,0xff,0xff}};

/* Forward declarations. */
static void  cs8900if_input(struct netif *netif);
static err_t cs8900if_output(struct netif *netif, struct pbuf *p,
			       struct ip_addr *ipaddr);

#include "rtxcapi.h"
#include "periphal.h" 
#include "enable.h"
#include "iom16c.h"
#include "cqueue.h"   /* SIO0OQ   */
#include "cres.h"     /* SIO0RES  */
#include "csema.h"    /* LANEVENT LANSEMA */
#include "cclock.h"   /* CLKTICK */
#include "ctask.h"    /* FPUTASK1 and FPUTASK2 */
#include "siocomm.h"

#define MEM_BASE 0x6000
#define IO_BASE 0x300
#define INT_NR 0x04

#define TXCMD    *(u16_t *)(MEM_BASE + IO_BASE + 0x04)
#define TXLENGTH *(u16_t *)(MEM_BASE + IO_BASE + 0x06)
#define ISQ      *(u16_t *)(MEM_BASE + IO_BASE + 0x08)
#define PACKETPP *(u16_t *)(MEM_BASE + IO_BASE + 0x0A)
#define PPDATA   *(u16_t *)(MEM_BASE + IO_BASE + 0x0C)
#define RXTXREG  *(u16_t *)(MEM_BASE + IO_BASE)

#define ETHADDR0 0x00
#define ETHADDR1 0xE0
#define ETHADDR2 0xFE
#define ETHADDR3 0x48
#define ETHADDR4 0xC8
#define ETHADDR5 0x28

static struct netif *cs8900if_netif;

/*-----------------------------------------------------------------------------------*/
static void
low_level_init(struct netif *netif)
{
  struct cs8900if *cs8900if;
  u16_t dummy;
  
  cs8900if = netif->state;
  
  /* Obtain MAC address from network interface. */

  /* XXX: make up an address. */
  cs8900if->ethaddr->addr[0] = ETHADDR0;
  cs8900if->ethaddr->addr[1] = ETHADDR1;
  cs8900if->ethaddr->addr[2] = ETHADDR2;
  cs8900if->ethaddr->addr[3] = ETHADDR3;
  cs8900if->ethaddr->addr[4] = ETHADDR4;
  cs8900if->ethaddr->addr[5] = ETHADDR5;

  /* Do whatever else is needed to initialize interface. */
#ifdef USE_ETH_INT
#ifdef VECTORS_IN_RAM
  /* claim the ethernet Interrrupt vector */
   setvect((unsigned long far *)INTB_ADDRESS, ETH_INT_NUM, ETH_ISR_ADDR);
#endif /* VECTORS_IN_RAM */
   INT0IC = ETH_IPL ;          /* set Interrupt control register */
#endif /* USE_ETH_INT */

   PRCR.2 = 1;
   P9D.4 = 1;
   PRCR.2 = 1;
   P9D.3 = 1;
   PRCR.2 = 1;
   P9D.6 = 1;
   P9.6 = 1;
  
   KS_lockw(CS8900_R);   
   PACKETPP = 0x0114 ;
   PPDATA = 0x0055 ;        /* Reset the chip */
   dummy = *(u8_t *)(MEM_BASE + IO_BASE + 0x0D) ;
   /* Dummy read, put chip in 16bit-mode */
   dummy = *(u16_t *)(MEM_BASE + IO_BASE + 0x0D) ;



   PACKETPP = 0x0020;   /* I/O-base addr */
   PPDATA = 0x0300;     /* set I/O-addr = 300H */
   
   PACKETPP = 0x0022;   /* Interrupt Number */
   PPDATA = 0x0004;     /* no interrupt channel */
   
   PACKETPP = 0x0024;   /* DMA Channel Number */
   PPDATA = 0x0003;     /* No DMA */
   
   PACKETPP = 0x0102;   /* Reciever Configuration    REG 3 */
   PPDATA = 0x0003;     /*  Make no interrupt = x0xx, interrupt = x1xx */
   
   PACKETPP = 0x0104;   /* Reciever Control    REG 5 */
   PPDATA = 0x0D05;     /* PromiscuousA accept all frames = 0x85 else 0x05 */

   /* Accept Broadcast, Individual & Valid Frame */
   
   PACKETPP = 0x0106;    /* Transmit configuration   REG 7 */
   PPDATA = 0x0007;      /* same as reset  ***** */
    
   PACKETPP = 0x010A;    /* Buffer Configuration   REG B */
   PPDATA = 0x000B;      /* same as reset  ***** */
    
   PACKETPP = 0x0112;    /* Line control     REG 13 */
   PPDATA = 0x00D3;      /* TX and Rx on   ****** */
    
   PACKETPP = 0x0114;    /* Self control     REG 15 */
   PPDATA = 0x0015;      /* same as reset  *****  and no reset =) */

         
   PACKETPP = 0x0116;    /* Bus Control     REG 17 */
   PPDATA = 0x0017;      /*   8xxx = generate IRQ */
   
   PACKETPP = 0x0118;    /* Test Control */
   PPDATA = 0x0099;      /* same as reset + DisableLT  XXX OBS  */
   
   PACKETPP = 0x0150;    /* Address Filter Register */
   PPDATA = 0x0000;      /* */
   // plus 3 till  ...
   
   /* Set MAC address. */
   PACKETPP = 0x0158;
   PPDATA = (ETHADDR0 | (ETHADDR1 << 8));
   PACKETPP = 0x015a;
   PPDATA = (ETHADDR2 | (ETHADDR3 << 8));
   PACKETPP = 0x015c;
   PPDATA = (ETHADDR4 | (ETHADDR5 << 8));

   /* Hack - pass the netif structure to the recieve thread through a global variable. */
   cs8900if_netif = netif;

   
   KS_unlock(CS8900_R);
   KS_signal(CS8900S);

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
low_level_output(struct cs8900if *cs8900if, struct pbuf *p)
{
  struct pbuf *q;
  u16_t i;
  u16_t *ptr;
  u16_t event;
  
  KS_lockw(CS8900_R);
  TXCMD = 0x00C9 ;   /* transmit command */
  TXLENGTH = p->tot_len;
    
  for(i = 0; i < 8 ; i++) {
    PACKETPP = 0x0138; /*  Bus Status Register   -
			   bit7 TxBidErr, bit 8 Rdy4TxNow */			   
    event = PPDATA;
    if(event & 0x0100)
      break ;

    /* Wait for buffer space to become avaliable. */
    KS_unlock(CS8900_R);
    KS_delay ((TASK)0,(TICKS)100/CLKTICK);
    KS_lockw(CS8900_R);
  }
  if (i == 8){
    KS_unlock(CS8900_R);
    return ERR_MEM;
  }
    
  
  for(q = p; q != NULL; q = q->next) {
    /* Send the data from the pbuf to the interface, one pbuf at a
       time. The size of the data in each pbuf is kept in the ->len
       variable. */
    ptr = (u16_t *)q->payload;
    for(i = 0; i < q->len; i += 2) {
      RXTXREG = *ptr++;
    }
  }
  
  KS_unlock(CS8900_R);

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
low_level_input(struct cs8900if *cs8900if)
{
  struct pbuf *p, *q;
  u16_t len;
  u16_t event_type;
  u16_t i;
  u16_t *ptr;
  
  
  /* Obtain the size of the packet and put it into the "len"
     variable. */
  KS_lockw(CS8900_R);
  event_type = RXTXREG; /* read RX-status reg */
  len = RXTXREG;        /* read the packetlength. */
  KS_unlock(CS8900_R);

  /* We allocate a pbuf chain of pbufs from the pool. */
  p = pbuf_alloc(PBUF_LINK, len, PBUF_POOL);

  KS_lockw(CS8900_R);
  if(p != NULL) {
    /* We iterate over the pbuf chain until we have read the entire
       packet into the pbuf. */
    for(q = p; q != NULL; q = q->next) {
      /* Read enough bytes to fill this pbuf in the chain. The
         avaliable data in the pbuf is given by the q->len
         variable. */
      /* read data into(q->payload, q->len); */
      ptr = q->payload;
      for(i = 0; i < (q->len + 1)/ 2; i++) {
	*ptr = RXTXREG;
	ptr++;
      }
    }
  } else {
    /* Discard packet in buffer. */
    PACKETPP = 0x102;
    PPDATA |= 0x0040;
  }

  KS_unlock(CS8900_R);

  return p;  
}
/*-----------------------------------------------------------------------------------*/
/*
 * cs8900rx():
 *
 * The receive thread. Works by polling the network device.
 */
/*-----------------------------------------------------------------------------------*/
ks_nosaveregs void
cs8900rx(void)
{
  u16_t event;

  /* Wait to be started by low_level_init() function. */
  KS_wait(CS8900S);

  
  while(1) {
    KS_delay ((TASK)0,(TICKS)50/CLKTICK);
    
    KS_lockw(CS8900_R);
    PACKETPP = 0x0124; /* Receive Event Register  -
			  bit8 RxOK, bitA Indl Addr, bitB broadc,
			  bit 7,C,D,E = bad frame */
    event = PPDATA;
    KS_unlock(CS8900_R);
    
    while(event & 0xFFC0) {
      if(event & 0x0100) { /* OK frame */
        if(event & (0x0400 | 0x0800)) {  /* Frame to us or a broadcast. */
          cs8900if_input(cs8900if_netif);
        } else {
#ifdef LINK_STATS
          stats.link.drop++;
#endif /* LINK_STATS */
	  /* Discard packet in buffer. */
	  PACKETPP = 0x102;
	  PPDATA |= 0x0040;
        }
      } else {
#ifdef LINK_STATS
        stats.link.drop++;
        if(event & 0x1080) {
          stats.link.err++;  /* Alignment error */
        } else if(event & 0x1000) {
          stats.link.chkerr++; /* CRC error */
        }
#endif /* LINK_STATS */        
      }
      KS_lockw(CS8900_R);
      
      PACKETPP = 0x0124; /* Receive Event Register -
			    bit8 RxOK, bitA Indl Addr, bitB broadc,
			    bit 7,C,D,E = bad frame */
      event = PPDATA;
      KS_unlock(CS8900_R);
    }
  }
}
/*-----------------------------------------------------------------------------------*/
/*
 * cs8900if_output():
 *
 * This function is called by the TCP/IP stack when an IP packet
 * should be sent. It calls the function called low_level_output() to
 * do the actuall transmission of the packet.
 *
 */
/*-----------------------------------------------------------------------------------*/
static err_t
cs8900if_output(struct netif *netif, struct pbuf *p,
		  struct ip_addr *ipaddr)
{
  struct cs8900if *cs8900if;
  struct pbuf *q;
  struct eth_hdr *ethhdr;
  struct eth_addr *dest, mcastaddr;
  struct ip_addr *queryaddr;
  err_t err;
  u8_t i;
  
  cs8900if = netif->state;

  /* Make room for Ethernet header. */
  if(pbuf_header(p, 14) != 0) {
    /* The pbuf_header() call shouldn't fail, but we allocate an extra
       pbuf just in case. */
    q = pbuf_alloc(PBUF_LINK, 14, PBUF_RAM);
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
    /* Handle multicast MAC address hashing. */
    mcastaddr.addr[0] = 0x01;
    mcastaddr.addr[1] = 0x00;
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
    q = arp_query(netif, cs8900if->ethaddr, queryaddr);
    if(q != NULL) {
      err = low_level_output(cs8900if, q);
      pbuf_free(q);
      return err;
    }
    return ERR_MEM;
  }
  ethhdr = p->payload;
  for(i = 0; i <  6; i++) {
    ethhdr->dest.addr[i] = dest->addr[i];
    ethhdr->src.addr[i] = cs8900if->ethaddr->addr[i];
  }
  ethhdr->type = htons(ETHTYPE_IP);
  
  return low_level_output(cs8900if, p);

}
/*-----------------------------------------------------------------------------------*/
/*
 * cs8900if_input():
 *
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface.
 *
 */
/*-----------------------------------------------------------------------------------*/
static void
cs8900if_input(struct netif *netif)
{
  struct cs8900if *cs8900if;
  struct eth_hdr *ethhdr;
  struct pbuf *p;


  cs8900if = netif->state;
  
  p = low_level_input(cs8900if);

  if(p == NULL) {
    return;
  }
  ethhdr = p->payload;

  switch(htons(ethhdr->type)) {
  case ETHTYPE_IP:
    arp_ip_input(netif, p);
    pbuf_header(p, -14);
    netif->input(p, netif);
    break;
  case ETHTYPE_ARP:
    p = arp_arp_input(netif, cs8900if->ethaddr, p);
    if(p != NULL) {
      low_level_output(cs8900if, p);
      pbuf_free(p);
    }
    break;
  default:
    pbuf_free(p);
    break;
  }
}
/*-----------------------------------------------------------------------------------*/
/*
 * cs8900if_init():
 *
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 */
/*-----------------------------------------------------------------------------------*/
void
cs8900if_init(struct netif *netif)
{
  struct cs8900if *cs8900if;
    
  cs8900if = mem_malloc(sizeof(struct cs8900if));
  netif->state = cs8900if;
  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  netif->output = cs8900if_output;
  
  cs8900if->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);
  
  low_level_init(netif);
  
  arp_init();  
}
/*-----------------------------------------------------------------------------------*/
