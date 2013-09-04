#include <stdlib.h>

#include "lwip/debug.h"

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"

#include "netif/arp.h"

#include "dc_time.h"


#define IFNAME0 'b'
#define IFNAME1 'b'

struct bbaif {
  struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */
};

static const struct eth_addr ethbroadcast = {{0xff,0xff,0xff,0xff,0xff,0xff}};

/* Forward declarations. */
static void  bbaif_input(struct netif *netif, int len, char *ringbuf,
			 int ring_offs, int ring_size);
static err_t bbaif_output(struct netif *netif, struct pbuf *p,
			       struct ip_addr *ipaddr);

/*-----------------------------------------------------------------------------------*/
static void bbaif_thread(void *netif)
{
  extern void ether_check_events(void (*)(struct netif *, int, char *,
					  int, int), void *);
  for(;;) {
    ether_check_events(bbaif_input, netif);
    sys_thread_yield(YIELD_MODE_RUN);
  }
}

static void
low_level_init(struct netif *netif)
{
  extern unsigned char ether_MAC[6];
  extern int ether_setup();
  int pci_setup();

  struct bbaif *bbaif;

  bbaif = netif->state;
  
  if(pci_setup()<0 || ether_setup()<0)
    exit(1);

  /* Obtain MAC address from network interface. */
  bbaif->ethaddr->addr[0] = ether_MAC[0];
  bbaif->ethaddr->addr[1] = ether_MAC[1];
  bbaif->ethaddr->addr[2] = ether_MAC[2];
  bbaif->ethaddr->addr[3] = ether_MAC[3];
  bbaif->ethaddr->addr[4] = ether_MAC[4];
  bbaif->ethaddr->addr[5] = ether_MAC[5];

  /* Do whatever else is needed to initialize interface. */  
  usleep(2000000);
  sys_thread_new(bbaif_thread, netif);
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
low_level_output(struct bbaif *bbaif, struct pbuf *p)
{
  extern char *ether_send_packet_1(void);
  extern void ether_send_packet_2(char *buf, int size);

  struct pbuf *q;

  char *buf = ether_send_packet_1();
  int len = 0;

  for(q = p; q != NULL; q = q->next) {
    /* Send the data from the pbuf to the interface, one pbuf at a
       time. The size of the data in each pbuf is kept in the ->len
       variable. */
    memcpy(buf+len, q->payload, q->len);
    len += q->len;
  }

  ether_send_packet_2(buf, len);
  
#ifdef LINK_STATS
  stats.link.xmit++;
#endif /* LINK_STATS */      

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
low_level_input(struct bbaif *bbaif, int len, char *ringbuf,
		int ring_offs, int ring_size)
{
  struct pbuf *p, *q;

  /* We allocate a pbuf chain of pbufs from the pool. */
  p = pbuf_alloc(PBUF_LINK, len, PBUF_POOL);
  
  if(p != NULL) {
    /* We iterate over the pbuf chain until we have read the entire
       packet into the pbuf. */
    for(q = p; q != NULL; q = q->next) {
      /* Read enough bytes to fill this pbuf in the chain. The
         avaliable data in the pbuf is given by the q->len
         variable. */
      int i;
      for(i=0; i<q->len; i++, ring_offs++)
	((char *)q->payload)[i] = ringbuf[ring_offs % ring_size];
    }
    /* acknowledge that packet has been read(); */
#ifdef LINK_STATS
    stats.link.recv++;
#endif /* LINK_STATS */      
  } else {
    /* drop packet(); */
#ifdef LINK_STATS
    stats.link.memerr++;
    stats.link.drop++;
#endif /* LINK_STATS */      
  }

  return p;  
}
/*-----------------------------------------------------------------------------------*/
/*
 * bbaif_output():
 *
 * This function is called by the TCP/IP stack when an IP packet
 * should be sent. It calls the function called low_level_output() to
 * do the actuall transmission of the packet.
 *
 */
/*-----------------------------------------------------------------------------------*/
static err_t
bbaif_output(struct netif *netif, struct pbuf *p,
		  struct ip_addr *ipaddr)
{
  struct bbaif *bbaif;
  struct pbuf *q;
  struct eth_hdr *ethhdr;
  struct eth_addr *dest, mcastaddr;
  struct ip_addr *queryaddr;
  err_t err;
  u8_t i;
  
  bbaif = netif->state;

  /* Make room for Ethernet header. */
  if(pbuf_header(p, 14) != 0) {
    /* The pbuf_header() call shouldn't fail, but we allocate an extra
       pbuf just in case. */
    q = pbuf_alloc(PBUF_LINK, 14, PBUF_RAM);
    if(q == NULL) {
#ifdef LINK_STATS
      stats.link.drop++;
      stats.link.memerr++;
#endif /* LINK_STATS */      
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
    q = arp_query(netif, bbaif->ethaddr, queryaddr);
    if(q != NULL) {
      err = low_level_output(bbaif, q);
      pbuf_free(q);
      return err;
    }
#ifdef LINK_STATS
    stats.link.drop++;
    stats.link.memerr++;
#endif /* LINK_STATS */          
    return ERR_MEM;
  }
  ethhdr = p->payload;

  for(i = 0; i < 6; i++) {
    ethhdr->dest.addr[i] = dest->addr[i];
    ethhdr->src.addr[i] = bbaif->ethaddr->addr[i];
  }
  
  ethhdr->type = htons(ETHTYPE_IP);
  
  return low_level_output(bbaif, p);

}
/*-----------------------------------------------------------------------------------*/
/*
 * bbaif_input():
 *
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface.
 *
 */
/*-----------------------------------------------------------------------------------*/
static void
bbaif_input(struct netif *netif, int len, char *ringbuf,
	    int ring_offs, int ring_size)
{
  struct bbaif *bbaif;
  struct eth_hdr *ethhdr;
  struct pbuf *p;


  bbaif = netif->state;
  
  p = low_level_input(bbaif, len, ringbuf, ring_offs, ring_size);

  if(p != NULL) {

#ifdef LINK_STATS
    stats.link.recv++;
#endif /* LINK_STATS */

    ethhdr = p->payload;
    
    switch(htons(ethhdr->type)) {
    case ETHTYPE_IP:
      arp_ip_input(netif, p);
      pbuf_header(p, -14);
      netif->input(p, netif);
      break;
    case ETHTYPE_ARP:
      p = arp_arp_input(netif, bbaif->ethaddr, p);
      if(p != NULL) {
	low_level_output(bbaif, p);
	pbuf_free(p);
      }
      break;
    default:
      pbuf_free(p);
      break;
    }
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
 * bbaif_init():
 *
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 */
/*-----------------------------------------------------------------------------------*/
void
bbaif_init(struct netif *netif)
{
  struct bbaif *bbaif;
    
  bbaif = mem_malloc(sizeof(struct bbaif));
  netif->state = bbaif;
  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  netif->output = bbaif_output;
  netif->linkoutput = low_level_output;
  
  bbaif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);
  
  low_level_init(netif);
  arp_init();

  sys_timeout(ARP_TMR_INTERVAL, (sys_timeout_handler)arp_timer, NULL);
}
/*-----------------------------------------------------------------------------------*/
