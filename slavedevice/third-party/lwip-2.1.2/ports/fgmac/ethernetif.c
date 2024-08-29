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
 * Date: 2022-12-09 16:42:31
 * LastEditTime: 2022-12-14 10:05:50
 * Description:  This file is the function file of the gmac adaptation to lwip stack.
 *
 * Modify History:
 *  Ver   Who            Date      Changes
 * ----- ------        --------   --------------------------------------
 * 1.0   wangxiaodong  2022/6/20  first release
 * 2.0   liuzhihong    2022/1/12  restructure
 */


#include <string.h>
#include "lwip/opt.h"
#include "lwip/mem.h"
#include "lwip/timeouts.h"
#include "../include/netif/ethernet.h"
#include "../include/lwip/etharp.h"
#include "../include/lwip/debug.h"
#include "sdkconfig.h"
#include "fparameters.h"
#include "lwip_port.h"
#if LWIP_IPV6
    #include "lwip/ethip6.h"
#endif

#include "fgmac_os.h"
#include "fgmac.h"
#include "fgmac_hw.h"
#include "fgmac_phy.h"
#include "fdebug.h"
#include "fassert.h"
#include "finterrupt.h"
#include "fgeneric_timer.h"

#ifndef SDK_CONFIG_H__
    #error "Please include sdkconfig.h first"
#endif

/* The time to block waiting for input. */
#define TIME_WAITING_FOR_INPUT (portMAX_DELAY)

#define ETHNETIF_DEBUG_TAG "ETHNETIF"

#define ETHNETIF_DEBUG_I(format, ...) FT_DEBUG_PRINT_I(ETHNETIF_DEBUG_TAG, format, ##__VA_ARGS__)
#define ETHNETIF_DEBUG_E(format, ...) FT_DEBUG_PRINT_E(ETHNETIF_DEBUG_TAG, format, ##__VA_ARGS__)
#define ETHNETIF_DEBUG_W(format, ...) FT_DEBUG_PRINT_W(ETHNETIF_DEBUG_TAG, format, ##__VA_ARGS__)

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become available since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
    FError ret ;
    FGmacOs *instance_p = NULL;
    FASSERT(netif != NULL);
    FASSERT(netif->state != NULL);
    struct LwipPort *gmac_netif_p = (struct LwipPort *)(netif->state);
    FASSERT(gmac_netif_p != NULL);

    instance_p = (FGmacOs *)(gmac_netif_p->state) ;

    portENTER_CRITICAL();
#if ETH_PAD_SIZE
    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

    ret = FGmacOsTx(instance_p, p);

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



static struct pbuf *low_level_input(struct netif *netif)
{
    FGmacOs *instance_p = NULL;
    FASSERT(netif != NULL);
    FASSERT(netif->state != NULL);
    struct LwipPort *gmac_netif_p = (struct LwipPort *)(netif->state);
    FASSERT(gmac_netif_p != NULL);
    instance_p = (FGmacOs *)(gmac_netif_p->state) ;

    return FGmacOsRx(instance_p);
}


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


static void ethernetif_start(struct netif *netif)
{
    struct LwipPort *gmac_netif_p = (struct LwipPort *)(netif->state);
    if (gmac_netif_p == NULL)
    {
        ETHNETIF_DEBUG_E("%s,gmac_netif_p is NULL\n", __FUNCTION__);
        return;
    }
    FGmacOs *instance_p = (FGmacOs *)(gmac_netif_p->state);
    FGmacOsStart(instance_p);
}


static enum lwip_port_link_status ethernetif_link_detect(struct netif *netif)
{
    struct LwipPort *gmac_netif_p = (struct LwipPort *)(netif->state);
    FGmacOs *instance_p;
    if (gmac_netif_p == NULL)
    {
        return ETH_LINK_UNDEFINED;
    }
    instance_p = (FGmacOs *)gmac_netif_p->state;
    if (instance_p->instance.is_ready != FT_COMPONENT_IS_READY)
    {
        return ETH_LINK_UNDEFINED;
    }

    return FGmacPhyStatus(gmac_netif_p);
}

static void ethernetif_deinit(struct netif *netif)
{
    struct LwipPort *gmac_netif_p = (struct LwipPort *)(netif->state);
    if (gmac_netif_p == NULL)
    {
        ETHNETIF_DEBUG_E("%s,gmac_netif_p is NULL\n", __FUNCTION__);
        return;
    }

    FGmacOs *instance_p = (FGmacOs *)(gmac_netif_p->state);

    FGmacStopTrans(&instance_p->instance);

}

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static err_t low_level_init(struct netif *netif)
{
    uintptr mac_address = (uintptr)(netif->state);
    struct LwipPort *gmac_netif_p;
    FGmacOs *instance_p;
    FGmac *gmac_p = NULL;
    FError ret;
    u32 dmacrreg;
    FtOsGmacPhyControl os_config;
    s32_t status = FT_SUCCESS;
    FASSERT(netif != NULL);
    FASSERT(netif->state != NULL);
    UserConfig *config_p;

    gmac_netif_p = mem_malloc(sizeof * gmac_netif_p);
    if (gmac_netif_p == NULL)
    {
        LWIP_DEBUGF(NETIF_DEBUG, ("gmac_netif_p init: out of memory\r\n"));
        return ERR_MEM;
    }

    /* obtain config of this emac */
    ETHNETIF_DEBUG_I("netif->state is %p \r\n ", netif->state);

    config_p = (UserConfig *)netif->state;
    os_config.instance_id = config_p->mac_instance;

    os_config.autonegotiation = config_p->autonegotiation; /* 1 is autonegotiation ,0 is manually set */
    os_config.phy_speed = config_p->phy_speed;  /* FGMAC_PHY_SPEED_XXX */
    os_config.phy_duplex = config_p->phy_duplex; /* FGMAC_PHY_XXX_DUPLEX */

    instance_p = FGmacOsGetInstancePointer(&os_config);
    if (instance_p == NULL)
    {
        ETHNETIF_DEBUG_E("FGmacOsGetInstancePointer is error\r\n");
        return ERR_ARG;
    }

    for (int i = 0; i < 6; i++)
    {
        instance_p->hwaddr[i] = netif->hwaddr[i];
    }

    ret = FGmacOsInit(instance_p);

    if (ret != FT_SUCCESS)
    {
        ETHNETIF_DEBUG_E("FGmacOsInit is error\r\n");
        return ERR_ARG;
    }

    gmac_netif_p->state = (void *)instance_p;
    netif->state = (void *)gmac_netif_p; /* update state */
    instance_p->stack_pointer = gmac_netif_p;

    /* maximum transfer unit */
    netif->mtu = GMAC_MTU;

    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP |
                   NETIF_FLAG_LINK_UP;

#if LWIP_IPV6 && LWIP_IPV6_MLD
    netif->flags |= NETIF_FLAG_MLD6;
#endif

#if LWIP_IGMP
    netif->flags |= NETIF_FLAG_IGMP;
#endif

    gmac_netif_p->ops.eth_detect = ethernetif_link_detect ;
    gmac_netif_p->ops.eth_input = ethernetif_input;
    gmac_netif_p->ops.eth_deinit = ethernetif_deinit;
    gmac_netif_p->ops.eth_start = ethernetif_start;
    ETHNETIF_DEBUG_I("ready to leave netif \r\n");
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



err_t ethernetif_gmac_init(struct netif *netif)
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


