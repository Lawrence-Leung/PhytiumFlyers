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
 *  Ver   Who  Date   Changes
 * ----- ------  -------- --------------------------------------
 * 1.0   wangxiaodong  2022/6/20  first release
 * 2.0   liuzhihong    2022/1/12  restructure
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
#include "fxmac_os.h"
#include "fdebug.h"


#define FXMAC_LWIP_NET_DEBUG_TAG "FXMAC_LWIP_NET"
#define FXMAC_LWIP_NET_PRINT_E(format, ...) FT_DEBUG_PRINT_E(FXMAC_LWIP_NET_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_LWIP_NET_PRINT_I(format, ...) FT_DEBUG_PRINT_I(FXMAC_LWIP_NET_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_LWIP_NET_PRINT_D(format, ...) FT_DEBUG_PRINT_D(FXMAC_LWIP_NET_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_LWIP_NET_PRINT_W(format, ...) FT_DEBUG_PRINT_W(FXMAC_LWIP_NET_DEBUG_TAG, format, ##__VA_ARGS__)


#if LWIP_IPV6
    #include "lwip/ethip6.h"
#endif

static void ethernetif_input(struct netif *netif);

enum lwip_port_link_status ethernetif_link_detect(struct netif *netif)
{
    struct LwipPort *xmac_netif_p = (struct LwipPort *)(netif->state);
    FXmacOs *instance_p;
    if (xmac_netif_p == NULL)
    {
        return ETH_LINK_UNDEFINED;
    }
    instance_p = (FXmacOs *)xmac_netif_p->state;
    if (instance_p->instance.is_ready != FT_COMPONENT_IS_READY)
    {
        return ETH_LINK_UNDEFINED;
    }

    return  FXmacPhyReconnect(xmac_netif_p);
}

static void ethernetif_start(struct netif *netif)
{
    struct LwipPort *xmac_netif_p = (struct LwipPort *)(netif->state);
    if (xmac_netif_p == NULL)
    {
        FXMAC_LWIP_NET_PRINT_E("%s,xmac_netif_p is NULL\n", __FUNCTION__);
        return;
    }
    FXmacOs *instance_p = (FXmacOs *)(xmac_netif_p->state);
    FXmacOsStart(instance_p);
}

static void ethernetif_poll(struct netif *netif)
{
    struct LwipPort *lwip_port = (struct LwipPort *)(netif->state);
    
    if(lwip_port == NULL)
    {
        FXMAC_LWIP_NET_PRINT_E("%s,lwip_port is NULL\n", __FUNCTION__);
        return;
    }
    FXmacOs *instance_p = (FXmacOs *)(lwip_port->state);
    
    if(instance_p == NULL)
    {
        FXMAC_LWIP_NET_PRINT_E("%s,Fxmac instance_p is NULL\n", __FUNCTION__);
        return;
    }
    FXmacOsRecvHandler(instance_p);
}
static void ethernetif_deinit(struct netif *netif)
{
    struct LwipPort *xmac_netif_p = (struct LwipPort *)(netif->state);
    if (xmac_netif_p == NULL)
    {
        FXMAC_LWIP_NET_PRINT_E("%s,xmac_netif_p is NULL\n", __FUNCTION__);
        return;
    }

    FXmacOs *instance_p = (FXmacOs *)(xmac_netif_p->state);
    FXmacOsStop(instance_p);
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
    FXmacOs *instance_p = NULL;
    FASSERT(netif != NULL);
    FASSERT(netif->state != NULL);
    struct LwipPort *xmac_netif_p = (struct LwipPort *)(netif->state);
    FASSERT(xmac_netif_p != NULL);

    instance_p = (FXmacOs *)(xmac_netif_p->state) ;

    portENTER_CRITICAL();
#if ETH_PAD_SIZE
    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

    ret = FXmacOsTx(instance_p, p);

#if ETH_PAD_SIZE
    pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif
    portEXIT_CRITICAL();

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
    FXmacOs *instance_p = NULL;
    FASSERT(netif != NULL);
    FASSERT(netif->state != NULL);
    struct LwipPort *xmac_netif_p = (struct LwipPort *)(netif->state);
    FASSERT(xmac_netif_p != NULL);
    instance_p = (FXmacOs *)(xmac_netif_p->state);

    return FXmacOsRx(instance_p);
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

    while (1)    
    {
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
    }

    return;
}

static err_t low_level_init(struct netif *netif)
{
    uintptr mac_address = (uintptr)(netif->state);
    struct LwipPort *xmac_netif_p;
    FXmacOs *instance_p;
    FXmac *xmac_p = NULL;
    FError ret;
    u32 dmacrreg;
    FXmacOsControl os_config;
    s32_t status = FT_SUCCESS;
    FASSERT(netif != NULL);
    FASSERT(netif->state != NULL);
    UserConfig *config_p;

    xmac_netif_p = mem_malloc(sizeof * xmac_netif_p);
    if (xmac_netif_p == NULL)
    {
        LWIP_DEBUGF(NETIF_DEBUG, ("xmac_netif_p init: out of memory\r\n"));
        return ERR_MEM;
    }


    sys_sem_new(&xmac_netif_p->sem_rx_data_available, 0);

    /* obtain config of this emac */
    FXMAC_LWIP_NET_PRINT_I("netif->state is %p \r\n ", netif->state);

    config_p = (UserConfig *)netif->state;
    os_config.instance_id = config_p->mac_instance;

    switch (config_p->mii_interface)
    {
        case LWIP_PORT_INTERFACE_RGMII:
            os_config.interface = FXMAC_OS_INTERFACE_RGMII;
            break;
        case LWIP_PORT_INTERFACE_SGMII:
            os_config.interface = FXMAC_OS_INTERFACE_SGMII;
            break;
        default:
            os_config.interface = FXMAC_OS_INTERFACE_RGMII;
            break;
    }

    os_config.autonegotiation = config_p->autonegotiation; /* 1 is autonegotiation ,0 is manually set */
    os_config.phy_speed = config_p->phy_speed;  /* FXMAC_PHY_SPEED_XXX */
    os_config.phy_duplex = config_p->phy_duplex; /* FXMAC_PHY_XXX_DUPLEX */

    instance_p = FXmacOsGetInstancePointer(&os_config);
    if (instance_p == NULL)
    {
        FXMAC_LWIP_NET_PRINT_E("FXmacOsGetInstancePointer is error\r\n");
        return ERR_ARG;
    }

    for (int i = 0; i < 6; i++)
    {
        instance_p->hwaddr[i] = netif->hwaddr[i];
    }

#if LWIP_IPV6
    instance_p->config = FXMAC_OS_CONFIG_COPY_ALL_FRAMES;
#endif

    ret = FXmacOsInit(instance_p);

    if (ret != FT_SUCCESS)
    {
        FXMAC_LWIP_NET_PRINT_E("FXmacOsInit is error\r\n");
        return ERR_ARG;
    }

    xmac_netif_p->state = (void *)instance_p;
    netif->state = (void *)xmac_netif_p; /* update state */
    instance_p->stack_pointer = xmac_netif_p;


    /* maximum transfer unit */
    if (instance_p->config & FXMAC_OS_CONFIG_JUMBO)
    {
        netif->mtu = FXMAC_MTU_JUMBO - FXMAC_HDR_SIZE;
    }
    else
    {
        netif->mtu = FXMAC_MTU ;
    }

    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP |
                   NETIF_FLAG_LINK_UP;

#if LWIP_IPV6 && LWIP_IPV6_MLD
    netif->flags |= NETIF_FLAG_MLD6;
#endif

#if LWIP_IGMP
    netif->flags |= NETIF_FLAG_IGMP;
#endif

    xmac_netif_p->ops.eth_detect = ethernetif_link_detect ;
    xmac_netif_p->ops.eth_input = ethernetif_input;
    xmac_netif_p->ops.eth_deinit = ethernetif_deinit;
    xmac_netif_p->ops.eth_start = ethernetif_start;
    xmac_netif_p->ops.eth_poll= ethernetif_poll; 
    FXMAC_LWIP_NET_PRINT_I("Ready to leave netif \r\n");
    return ERR_OK;
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
    LWIP_DEBUGF(NETIF_DEBUG, ("*******Start init eth\n"));

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
#endif /* LWIP_IPV4 */

    netif->linkoutput = low_level_output;
#if LWIP_IPV6
    netif->output_ip6 = ethip6_output;
#endif

    low_level_init(netif);
    return ERR_OK;
}


