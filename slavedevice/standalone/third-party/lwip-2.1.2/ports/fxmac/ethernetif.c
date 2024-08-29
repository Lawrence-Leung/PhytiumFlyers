/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc. 
 * All Rights Reserved.
 *  
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it  
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,  
 * either version 1.0 of the License, or (at your option) any later version. 
 *  
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;  
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details. 
 *  
 * 
 * FilePath: ethernetif.c
 * Date: 2022-07-21 19:18:46
 * LastEditTime: 2022-07-21 19:18:46
 * Description:  This file is the function file of the xmac adaptation to lwip stack.
 * 
 * Modify History: 
 *  Ver     Who           Date                  Changes
 * -----   ------       --------     --------------------------------------
 *  1.0    huanghe     2022/11/3            first release
 */


#include <stdio.h>
#include <string.h>

#include "lwipopts.h"
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "lwip/igmp.h"
#include "netif/etharp.h"
#include "lwip_port.h"
#include "fxmac_lwip_port.h"
#include "fdebug.h"


#define FXMAC_LWIP_NET_DEBUG_TAG "FXMAC_LWIP_NET"
#define FXMAC_LWIP_NET_PRINT_E(format, ...) FT_DEBUG_PRINT_E(FXMAC_LWIP_NET_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_LWIP_NET_PRINT_I(format, ...) FT_DEBUG_PRINT_I(FXMAC_LWIP_NET_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_LWIP_NET_PRINT_D(format, ...) FT_DEBUG_PRINT_D(FXMAC_LWIP_NET_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_LWIP_NET_PRINT_W(format, ...) FT_DEBUG_PRINT_W(FXMAC_LWIP_NET_DEBUG_TAG, format, ##__VA_ARGS__)


#if LWIP_IPV6
#include "lwip/ethip6.h"
#endif

#if LWIP_IGMP
static err_t xmac_filter_update (struct netif *netif,const ip_addr_t *group, enum netif_mac_filter_action action);
static u8_t xmac_multicast_entry_mask = 0;
#endif

static void ethernetif_input(struct netif *netif);

enum lwip_port_link_status ethernetif_link_detect(struct netif *netif)
{
	struct LwipPort *lwip_port = (struct LwipPort *)(netif->state);
	FXmacLwipPort *instance_p;
	if(lwip_port == NULL)
	{
		return ETH_LINK_UNDEFINED;
	}
	instance_p = (FXmacLwipPort *)lwip_port->state;
	if (instance_p->instance.is_ready != FT_COMPONENT_IS_READY)
	{
		return ETH_LINK_UNDEFINED;
	}

	return  FXmacPhyReconnect(lwip_port);
}

void ethernetif_debug(struct netif *netif)
{
    struct LwipPort *lwip_port = (struct LwipPort *)(netif->state);
    FXmacLwipPort *instance_p;
	if(lwip_port == NULL)
	{
        FXMAC_LWIP_NET_PRINT_E("lwip_port is an NULL pointer");
        return;
    }
	instance_p = (FXmacLwipPort *)lwip_port->state;
	if (instance_p->instance.is_ready != FT_COMPONENT_IS_READY)
	{
        FXMAC_LWIP_NET_PRINT_E("The drive is not ready");
		return ;
	}
    
    FXmacDebugTxPrint(&instance_p->instance);
    FXmacDebugRxPrint(&instance_p->instance);
    FXmacDebugUsxPrint(&instance_p->instance);
}

static void ethernetif_start(struct netif *netif)
{
    struct LwipPort *lwip_port = (struct LwipPort *)(netif->state);
    if(lwip_port == NULL)
    {
        FXMAC_LWIP_NET_PRINT_E("%s,lwip_port is NULL\n", __FUNCTION__);
        return;
    }
    FXmacLwipPort *instance_p = (FXmacLwipPort *)(lwip_port->state);
    FXmacLwipPortStart(instance_p);
}

static void ethernetif_deinit(struct netif *netif)
{
    struct LwipPort *lwip_port = (struct LwipPort *)(netif->state);
    if(lwip_port == NULL)
    {
        FXMAC_LWIP_NET_PRINT_E("%s,lwip_port is NULL\n", __FUNCTION__);
        return;
    }

    FXmacLwipPort *instance_p = (FXmacLwipPort *)(lwip_port->state);

    FXmacLwipPortStop(instance_p);
}

static void ethernetif_poll(struct netif *netif)
{
    struct LwipPort *lwip_port = (struct LwipPort *)(netif->state);
    
    if(lwip_port == NULL)
    {
        FXMAC_LWIP_NET_PRINT_E("%s,lwip_port is NULL\n", __FUNCTION__);
        return;
    }
    FXmac *instance_p = (FXmac *)(lwip_port->state);
    
    if(instance_p == NULL)
    {
        FXMAC_LWIP_NET_PRINT_E("%s,Fxmac instance_p is NULL\n", __FUNCTION__);
        return;
    }
    FXmacRecvHandler(instance_p->recv_args);
}

/*
 * low_level_output():
 *
 * Should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 */
static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
    FError ret ;
    FXmacLwipPort *instance_p = NULL;
    FASSERT(netif != NULL);
    FASSERT(netif->state != NULL);
    struct LwipPort *lwip_port = (struct LwipPort *)(netif->state);
    FASSERT(lwip_port != NULL);

    instance_p = (FXmacLwipPort *)(lwip_port->state) ;

#if ETH_PAD_SIZE
    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif
    
    ret = FXmacLwipPortTx(instance_p, p);

#if ETH_PAD_SIZE
    pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

    if (ret != FT_SUCCESS)
    {
        return ERR_MEM; 
    }

    return ERR_OK;
}

/*
 * low_level_input():
 *
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 */
static struct pbuf *low_level_input(struct netif *netif)
{
    FXmacLwipPort *instance_p = NULL;
    FASSERT(netif != NULL);
    FASSERT(netif->state != NULL);
    struct LwipPort *lwip_port = (struct LwipPort *)(netif->state);
    FASSERT(lwip_port != NULL);
    instance_p = (FXmacLwipPort *)(lwip_port->state) ;

    return FXmacLwipPortRx(instance_p);
}

/*
 * ethernetif_input():
 *
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface.
 *
 * Returns the number of packets read (max 1 packet on success,
 * 0 if there are no packets)
 *
 */

static void ethernetif_input(struct netif *netif)
{
    struct eth_hdr *ethhdr;
    struct pbuf *p;
    SYS_ARCH_DECL_PROTECT(lev);

    /* move received packet into a new pbuf */
    SYS_ARCH_PROTECT(lev);
    p = low_level_input(netif);
    SYS_ARCH_UNPROTECT(lev);

    /* no packet could be read, silently ignore this */
    if (p == NULL)
    {
        return;
    }

    /* points to packet payload, which starts with an Ethernet header */
    ethhdr = p->payload;
#if LINK_STATS
    lwip_stats.link.recv++;
#endif /* LINK_STATS */
    switch (htons(ethhdr->type))
    {
    /* IP or ARP packet? */
        case ETHTYPE_IP:
        case ETHTYPE_ARP:
    #if LWIP_IPV6
        /*IPv6 Packet?*/
        case ETHTYPE_IPV6:
    #endif
    #if PPPOE_SUPPORT
            /* PPPoE packet? */
        case ETHTYPE_PPPOEDISC:
        case ETHTYPE_PPPOE:
    #endif /* PPPOE_SUPPORT */

        /* full packet send to tcpip_thread to process */
            if (netif->input(p, netif) != ERR_OK)
            {
                LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\r\n"));
                pbuf_free(p);
                p = NULL;
            }
            break;

        default:
            pbuf_free(p);
            p = NULL;
            break;
    }
    
    
    return;
}

static err_t low_level_init(struct netif *netif)
{
    uintptr mac_address = (uintptr)(netif->state);
    struct LwipPort *lwip_port;
    FXmacLwipPort *instance_p;
    FXmac *xmac_p = NULL;
    FError ret;
    u32 dmacrreg;
    FXmacLwipPortControl mac_lwip_port_config;
    s32_t status = FT_SUCCESS;
    FASSERT(netif != NULL);
    FASSERT(netif->state != NULL);
    UserConfig *config_p;
    
    /* step 1：malloc lwip port object */
    lwip_port = mem_malloc(sizeof *lwip_port);
    if (lwip_port == NULL)
    {
        LWIP_DEBUGF(NETIF_DEBUG, ("lwip_port init: out of memory\r\n"));
        return ERR_MEM;
    }


    /* obtain config of this emac */
    FXMAC_LWIP_NET_PRINT_I("netif->state is %p \r\n ",netif->state);

    config_p = (UserConfig *)netif->state;
    if(config_p == NULL)
    {
        FXMAC_LWIP_NET_PRINT_E("UserConfig is NULL");
        goto failed;
    }

    mac_lwip_port_config.instance_id = config_p->mac_instance;

    switch (config_p->mii_interface)
    {
        case LWIP_PORT_INTERFACE_RGMII:
            mac_lwip_port_config.interface = FXMAC_LWIP_PORT_INTERFACE_RGMII;
            break;
        case LWIP_PORT_INTERFACE_SGMII:
            mac_lwip_port_config.interface = FXMAC_LWIP_PORT_INTERFACE_SGMII;
            break;
        case LWIP_PORT_INTERFACE_USX:
            mac_lwip_port_config.interface = FXMAC_LWIP_PORT_INTERFACE_USXGMII;
            break;
        default:
            mac_lwip_port_config.interface = FXMAC_LWIP_PORT_INTERFACE_RGMII;
            break;
    }

    mac_lwip_port_config.autonegotiation = config_p->autonegotiation; /* 1 is autonegotiation ,0 is manually set */
    mac_lwip_port_config.phy_speed = config_p->phy_speed;  /* FXMAC_PHY_SPEED_XXX */
    mac_lwip_port_config.phy_duplex = config_p->phy_duplex; /* FXMAC_PHY_XXX_DUPLEX */

    instance_p = FXmacLwipPortGetInstancePointer(&mac_lwip_port_config);
    if(instance_p == NULL)
    {
        FXMAC_LWIP_NET_PRINT_E("FXmacLwipPortGetInstancePointer is error\r\n");
        return ERR_ARG;
    }

    for (int i = 0; i < 6; i++)
    {
        instance_p->hwaddr[i] = netif->hwaddr[i];
    }

#if LWIP_IPV6
    instance_p->config = FXMAC_LWIP_PORT_CONFIG_COPY_ALL_FRAMES;
#endif

    ret = FXmacLwipPortInit(instance_p);

    if (ret != FT_SUCCESS)
    {
        FXMAC_LWIP_NET_PRINT_E("FXmacLwipPortInit is error\r\n");
        return ERR_ARG;
    }

    lwip_port->state = (void *)instance_p;
    netif->state = (void *)lwip_port; /* update state */
    instance_p->stack_pointer = lwip_port;

    /* maximum transfer unit */
    if(instance_p->config & FXMAC_LWIP_PORT_CONFIG_JUMBO) 
    {
        netif->mtu = FXMAC_MTU_JUMBO;
    }
    else
    {
        netif->mtu = FXMAC_MTU;
    }

    if(instance_p->config & FXMAC_LWIP_PORT_CONFIG_UNICAST_ADDRESS_FILITER) 
    {
       LWIP_DEBUGF(NETIF_DEBUG, ("Set unicast hash table!!\n"));
       FXmac_SetHash(&instance_p->instance, netif->hwaddr);
    }

    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP |
                   NETIF_FLAG_LINK_UP;


    #if LWIP_IPV6 && LWIP_IPV6_MLD
        netif->flags |= NETIF_FLAG_MLD6;
    #endif

    #if LWIP_IGMP
        netif->flags |= NETIF_FLAG_IGMP;
    #endif

    lwip_port->ops.eth_detect = ethernetif_link_detect ;
    lwip_port->ops.eth_input = ethernetif_input;
    lwip_port->ops.eth_deinit = ethernetif_deinit;
    lwip_port->ops.eth_start = ethernetif_start;
    lwip_port->ops.eth_debug = ethernetif_debug;
    lwip_port->ops.eth_poll = ethernetif_poll;
    FXMAC_LWIP_NET_PRINT_I("ready to leave netif \r\n");
    return ERR_OK;
failed:
    mem_free(lwip_port);
    return ERR_MEM;
}

#if !LWIP_ARP
/**
 * This function has to be completed by user in case of ARP OFF.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if ...
 */
static err_t low_level_output_arp_off(struct netif *netif, struct pbuf *q, const ip4_addr_t *ipaddr)
{  
  	err_t errval;
	errval = ERR_OK;
    
    
	return errval;
  
}
#endif /* LWIP_ARP */ 

/*
 * ethernetif_xmac_init():
 *
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 */

err_t ethernetif_xmac_init(struct netif *netif)
{
    LWIP_DEBUGF(NETIF_DEBUG, ("*******start init eth\n"));

#if LWIP_NETIF_HOSTNAME
	/* Initialize interface hostname */
  	netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

#if LWIP_IPV4
#if LWIP_ARP || LWIP_ETHERNET
#if LWIP_ARP
	netif->output = etharp_output;
#else
  	/* The user should write ist own code in low_level_output_arp_off function */
	netif->output = low_level_output_arp_off;
#endif /* LWIP_ARP */
#endif /* LWIP_ARP || LWIP_ETHERNET */

#if LWIP_IGMP
	netif_set_igmp_mac_filter(netif,xmac_filter_update);
#endif

#endif /* LWIP_IPV4 */

    netif->linkoutput = low_level_output;
#if LWIP_IPV6
    netif->output_ip6 = ethip6_output;
#endif

    return low_level_init(netif);
}


#if LWIP_IGMP
static void xmac_hash_update (struct netif *netif, u8_t *ip_addr,
		u8_t action)
{
	u8_t multicast_mac_addr[6];
    struct LwipPort *lwip_port;
    FXmacLwipPort *instance_p;

	lwip_port = (struct LwipPort *) (netif->state);
    instance_p = (FXmacLwipPort *)lwip_port->state;
	FXmacBdRing *txring;
	txring = &(FXMAC_GET_TXRING(instance_p->instance));

	multicast_mac_addr[0] = 0x01;
	multicast_mac_addr[1] = 0x00;
	multicast_mac_addr[2] = 0x5E;
	multicast_mac_addr[3] = ip_addr[1] & 0x7F;
	multicast_mac_addr[4] = ip_addr[2];
	multicast_mac_addr[5] = ip_addr[3];

	/* Wait till all sent packets are acknowledged from HW */
	while(txring->hw_cnt) ;
   
	SYS_ARCH_DECL_PROTECT(lev);

	SYS_ARCH_PROTECT(lev);

	/* Stop Ethernet */
	FXmacStop(&instance_p->instance);

	if (action == IGMP_ADD_MAC_FILTER) 
    {
		/* Set Mulitcast mac address in hash table */
		FXmac_SetHash(&instance_p->instance, multicast_mac_addr);

	} 
    else if (action == IGMP_DEL_MAC_FILTER) 
    {
		/* Remove Mulitcast mac address in hash table */
		FXmac_DeleteHash(&instance_p->instance, multicast_mac_addr);
	}

	/* Reset DMA */
    ResetDma(instance_p);

	/* Start Ethernet */
	FXmacStart(&instance_p->instance);

	SYS_ARCH_UNPROTECT(lev);
}

static err_t xmac_filter_update (struct netif *netif, const ip_addr_t *group,enum netif_mac_filter_action action)
{
	u8_t temp_mask;
	unsigned int i;
	u8_t * ip_addr = (u8_t *) group;

	if ((ip_addr[0] < 224) && (ip_addr[0] > 239)) 
    {
		LWIP_DEBUGF(NETIF_DEBUG,
				("%s: The requested MAC address is not a multicast address.\r\n", __func__));
		LWIP_DEBUGF(NETIF_DEBUG,
				("Multicast address add operation failure !!\r\n"));

		return ERR_ARG;
	}
	if (action == IGMP_ADD_MAC_FILTER) 
    {

		for (i = 0; i < FXMAC_MAX_MAC_ADDR; i++) 
        {
			temp_mask = (0x01) << i;
			if ((xmac_multicast_entry_mask & temp_mask) == temp_mask) 
            {
				continue;
			}
			xmac_multicast_entry_mask |= temp_mask;

			/* Update mac address in hash table */
			xmac_hash_update(netif, ip_addr, action);

			LWIP_DEBUGF(NETIF_DEBUG,
					("%s: Multicast MAC address successfully added.\r\n", __func__));

			return ERR_OK;
		}
		if (i == FXMAC_MAX_MAC_ADDR) 
        {
			LWIP_DEBUGF(NETIF_DEBUG,
					("%s: No multicast address registers left.\r\n", __func__));
			LWIP_DEBUGF(NETIF_DEBUG,
					("Multicast MAC address add operation failure !!\r\n"));

		}
			return ERR_MEM;

	} 
    else if (action == IGMP_DEL_MAC_FILTER) 
    {
		for (i = 0; i < FXMAC_MAX_MAC_ADDR; i++) 
        {
			temp_mask = (0x01) << i;
			if ((xmac_multicast_entry_mask & temp_mask) != temp_mask) 
            {
				continue;
			}
			xmac_multicast_entry_mask &= (~temp_mask);

			/* Update mac address in hash table */
			xmac_hash_update(netif, ip_addr, action);

			LWIP_DEBUGF(NETIF_DEBUG,
					("%s: Multicast MAC address successfully removed.\r\n", __func__));

			return ERR_OK;
		}
		if (i == FXMAC_MAX_MAC_ADDR) 
        {
			LWIP_DEBUGF(NETIF_DEBUG,
					("%s: No multicast address registers present with\r\n", __func__));
			LWIP_DEBUGF(NETIF_DEBUG,
					("the requested Multicast MAC address.\r\n"));
			LWIP_DEBUGF(NETIF_DEBUG,
					("Multicast MAC address removal failure!!.\r\n"));

			return ERR_MEM;
		}
	}
	return ERR_OK;
}
#endif
