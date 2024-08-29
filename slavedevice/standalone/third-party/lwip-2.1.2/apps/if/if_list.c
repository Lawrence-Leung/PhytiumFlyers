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
 * FilePath: if_list.c
 * Date: 2022-10-27 16:20:55
 * LastEditTime: 2022-10-27 16:20:55
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who  Date   Changes
 * ----- ------  -------- --------------------------------------
 */
#include "sdkconfig.h"
#include "ftypes.h"
#include "stdio.h"
#include "lwip/netif.h"
#include "lwip/dns.h"
#include "lwip/inet.h"
#include "lwip/init.h"
#include "sys_arch.h"


void ListIf(void)
{
    u8 index;
    struct netif * netif;
    sys_prot_t cur;
    cur = sys_arch_protect();

    netif = netif_list;

    while( netif != NULL )
    {
        printf("network interface: %c%c%s\n",
                   netif->name[0],
                   netif->name[1],
                   (netif == netif_default)?" (Default)":"");
        printf("MTU: %d\n", netif->mtu);
        printf("MAC: ");
        for (index = 0; index < netif->hwaddr_len; index ++)
            printf("%02x ", netif->hwaddr[index]);
        printf("\nFLAGS:");
        if (netif->flags & NETIF_FLAG_UP) printf(" UP");
        else printf(" DOWN");
        if (netif->flags & NETIF_FLAG_LINK_UP) printf(" LINK_UP");
        else printf(" LINK_DOWN");
        if (netif->flags & NETIF_FLAG_ETHARP) printf(" ETHARP");
        if (netif->flags & NETIF_FLAG_BROADCAST) printf(" BROADCAST");
        if (netif->flags & NETIF_FLAG_IGMP) printf(" IGMP");
        printf("\n");
        printf("ip address: %s\n", ipaddr_ntoa(&(netif->ip_addr)));
        printf("gw address: %s\n", ipaddr_ntoa(&(netif->gw)));
        printf("net mask  : %s\n", ipaddr_ntoa(&(netif->netmask)));
        printf("\n");
#if LWIP_IPV6
        {
            ip6_addr_t *addr;
            int addr_state;
            int i;

            addr = (ip6_addr_t *)&netif->ip6_addr[0];
            addr_state = netif->ip6_addr_state[0];

            printf("\nipv6 link-local: %s state:%02X %s\n", ip6addr_ntoa(addr),
            addr_state, ip6_addr_isvalid(addr_state)?"VALID":"INVALID");

            for(i=1; i<LWIP_IPV6_NUM_ADDRESSES; i++)
            {
                addr = (ip6_addr_t *)&netif->ip6_addr[i];
                addr_state = netif->ip6_addr_state[i];

                printf("ipv6[%d] address: %s state:%02X %s\n", i, ip6addr_ntoa(addr),
                addr_state, ip6_addr_isvalid(addr_state)?"VALID":"INVALID");
            }
        }
        printf("\r\n");
#endif /* LWIP_IPV6 */
        netif = netif->next;
    }

#if LWIP_DNS
    {
#if LWIP_VERSION_MAJOR == 1U /* v1.x */
        struct ip_addr ip_addr;

        for(index=0; index<DNS_MAX_SERVERS; index++)
        {
            ip_addr = dns_getserver(index);
            printf("dns server #%d: %s\n", index, ipaddr_ntoa(&(ip_addr)));
        }
#else /* >= v2.x */
        const ip_addr_t *ip_addr;

        for(index=0; index<DNS_MAX_SERVERS; index++)
        {
            ip_addr = dns_getserver(index);
            printf("dns server #%d: %s\n", index, inet_ntoa(ip_addr));
        }
#endif /* LWIP_VERSION_MAJOR == 1U */
    }
#endif /**< #if LWIP_DNS */

    sys_arch_unprotect(cur);

}

