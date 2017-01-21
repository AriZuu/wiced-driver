/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * Copyright (c) 2017, Ari Suutari <ari@stonepile.fi>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include <picoos.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "wiced-driver.h"

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/ethip6.h"
#include "netif/etharp.h"
#include "lwip/mld6.h"

#include "wwd_wifi.h"
#include "wwd_eapol.h"
#include "network/wwd_network_interface.h"
#include "network/wwd_buffer_interface.h"
#include "wwd_assert.h"
#include "wiced_constants.h"
#include "wwd_bus_protocol.h"

/* Define those to better describe your network interface. */
#define IFNAME0 'w'
#define IFNAME1 'l'

#if LWIP_IGMP
static err_t igmp_mac_filter( struct netif *netif, ip_addr_t *group, u8_t action );
#endif

#if LWIP_IPV6 && LWIP_IPV6_MLD
static err_t mld_mac_filter(struct netif *netif, const ip6_addr_t *group, u8_t action);
#endif

/**
 * Low-level initialization for network interface.
 * 
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void
low_level_init(struct netif *netif)
{
  wwd_result_t result;

  /* set MAC hardware address length */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;

  result = wwd_wifi_get_mac_address((wiced_mac_t*)netif->hwaddr, (wwd_interface_t)netif->state);
  P_ASSERT("wlan mac address valid", result != WWD_SUCCESS);
  
  /* maximum transfer unit */
  netif->mtu = WICED_PAYLOAD_MTU;

  /* device capabilities */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

#if LWIP_IPV6 && LWIP_IPV6_MLD

  netif->mld_mac_filter = mld_mac_filter;
  ip6_addr_t ip6_allnodes_ll;
  ip6_addr_set_allnodes_linklocal(&ip6_allnodes_ll);
  netif->mld_mac_filter(netif, &ip6_allnodes_ll, NETIF_ADD_MAC_FILTER);
    
#endif /* LWIP_IPV6 && LWIP_IPV6_MLD */

#if LWIP_IGMP

  netif->flags |= NETIF_FLAG_IGMP;
  netif_set_igmp_mac_filter(netif, igmp_mac_filter);
  
#endif
}

/**
 * Send packet to network.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 */

static err_t
low_level_output(struct netif *netif, struct pbuf *p)
{
  if (((wiced_interface_t)netif->state) == WICED_ETHERNET_INTERFACE ||
      wwd_wifi_is_ready_to_transceive((wwd_interface_t)netif->state) == WWD_SUCCESS) {
    
    pbuf_ref(p);

#if ETH_PAD_SIZE

/*
 * Wiced layer expects that pbuf points to beginning
 * of ethernet frame. When using a padding word for
 * alignment we must drop it (actually the padding word
 * overlaps with the space used by wiced link headers
 * but LwIP never actually touches it so this is not a problem.
 */
    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
    
#endif

    LWIP_ASSERT("Must be single pbuf", ((p->next == NULL) && (( p->tot_len == p->len ))));
    wwd_network_send_ethernet_data(p, (wwd_interface_t)netif->state);

    MIB2_STATS_NETIF_ADD(netif, ifoutoctets, p->tot_len);
    if (((u8_t*)p->payload)[0] & 1) {
    
      /* broadcast or multicast packet*/
      MIB2_STATS_NETIF_INC(netif, ifoutnucastpkts);
    }
    else {
    
      /* unicast packet */
      MIB2_STATS_NETIF_INC(netif, ifoutucastpkts);
    }

    LINK_STATS_INC(link.xmit);
    return ERR_OK;
  }
  else {
  
    return ERR_INPROGRESS;
  }
}

/**
 * WICED stack calls this function when packet has been
 * received from network.
 */
void host_network_process_ethernet_data(wiced_buffer_t p, wwd_interface_t interface)
{
#if ETH_PAD_SIZE

/*
 * LwIP expects that incoming pbuf points to padding word of ethernet header.
 * The padding word actually has data from Wiced link layer but as it
 * is not modified by LwIP it is ok for them to overlap.
 */
  pbuf_header(p, ETH_PAD_SIZE); /* add the padding word */
#endif

  struct netif* netif;
  uint8_t       result;

  for (netif = netif_list; (netif != NULL) && (netif->state != (void*)interface); netif = netif->next) {
  }

  if (netif == NULL) {

    result = pbuf_free(p);
    LWIP_ASSERT("pbuf_free", (result != 0));
    p = NULL;
    return;
  }

  MIB2_STATS_NETIF_ADD(netif, ifinoctets, p->tot_len);
  if (((u8_t*)p->payload)[0] & 1) {

    /* broadcast or multicast packet*/
    MIB2_STATS_NETIF_INC(netif, ifinnucastpkts);
  }
  else {

    /* unicast packet*/
    MIB2_STATS_NETIF_INC(netif, ifinucastpkts);
  }

  LINK_STATS_INC(link.recv);

#warning LWIP_HOOK_UNKNOWN_ETH_PROTOCOL voi hoitaa EAPOL
  if (netif->input(p, netif) != ERR_OK) {

    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
    pbuf_free(p);
    p = NULL;
  }
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t
ethernetif_init(struct netif *netif)
{
  LWIP_ASSERT("netif != NULL", (netif != NULL));

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  /* Check netif for being valid WICED interface */
  if ((wwd_interface_t)netif->state > WWD_ETHERNET_INTERFACE)
    return ERR_ARG;
    
  /*
   * Initialize the snmp variables and counters inside the struct netif.
   * The last argument should be replaced with your link speed, in units
   * of bits per second.
   */
  MIB2_INIT_NETIF(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;
#if LWIP_IPV6
  netif->output_ip6 = ethip6_output;
#endif /* LWIP_IPV6 */
  netif->linkoutput = low_level_output;

  /* initialize the hardware */
  low_level_init(netif);

  return ERR_OK;
}

#if LWIP_IGMP

#define MULTICAST_IP_TO_MAC(ip)       { (uint8_t) 0x01,             \
                                        (uint8_t) 0x00,             \
                                        (uint8_t) 0x5e,             \
                                        (uint8_t) ((ip)[1] & 0x7F), \
                                        (uint8_t) (ip)[2],          \
                                        (uint8_t) (ip)[3]           \
                                      }

static err_t igmp_mac_filter(struct netif *netif, ip_addr_t *group, u8_t action)
{
  wiced_mac_t mac = { MULTICAST_IP_TO_MAC((uint8_t*)group) };

  switch (action) {
  case NETIF_ADD_MAC_FILTER:
    if (wwd_wifi_register_multicast_address(&mac) != WWD_SUCCESS )
      return ERR_VAL;

    break;

  case NETIF_DEL_MAC_FILTER:
    if (wwd_wifi_unregister_multicast_address(&mac) != WWD_SUCCESS )
      return ERR_VAL;

    break;

  default:
    return ERR_VAL;
  }

  return ERR_OK;
}

#endif

#if LWIP_IPV6 && LWIP_IPV6_MLD

static err_t mld_mac_filter(struct netif *netif, const ip6_addr_t *group, u8_t action)
{
  wiced_mac_t mac;
  uint8_t* g = (uint8_t*)&group->addr[3];

  mac.octet[0] = 0x33;
  mac.octet[1] = 0x33;
  mac.octet[2] = g[0];
  mac.octet[3] = g[1];
  mac.octet[4] = g[2];
  mac.octet[5] = g[3];

  switch (action) {
  case NETIF_ADD_MAC_FILTER:
    if (wwd_wifi_register_multicast_address(&mac) != WWD_SUCCESS )
      return ERR_VAL;

    break;

  case NETIF_DEL_MAC_FILTER:
    if (wwd_wifi_unregister_multicast_address(&mac) != WWD_SUCCESS )
      return ERR_VAL;

    break;

  default:
    return ERR_VAL;
  }

  return ERR_OK;
}

#endif

/********************************** End of file ******************************************/
