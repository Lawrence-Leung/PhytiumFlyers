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
 * FilePath: if_set.c
 * Date: 2022-10-27 16:41:01
 * LastEditTime: 2022-10-27 16:41:02
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who  Date   Changes
 * ----- ------  -------- --------------------------------------
 */
#include "sdkconfig.h"
#include "ftypes.h"
#include "stdio.h"
#include "string.h"
#include "lwip/netif.h"
#include "lwip/dns.h"
#include "lwip/inet.h"
#include "lwip/init.h"
#include "shell.h"
#include "sys_arch.h"
int Max_mtu = -1;
void SetIf(char* netif_name, char* ip_addr, char* gw_addr, char* nm_addr)
{
    sys_prot_t cur;
    cur = sys_arch_protect();

#if LWIP_VERSION_MAJOR == 1U /* v1.x */
    struct ip_addr *ip;
    struct ip_addr addr;
#else /* >= v2.x */
    ip4_addr_t *ip;
    ip4_addr_t addr;
#endif /* LWIP_VERSION_MAJOR == 1U */
    struct netif *netif = netif_list;

    if(strlen(netif_name) > sizeof(netif->name))
    {
        printf("network interface name too long! %d %s \r\n",strlen(netif_name),netif_name);
        goto exit;
    }

    while(netif != NULL)
    {
        if(strncmp(netif_name, netif->name, sizeof(netif->name)) == 0)
            break;

        netif = netif->next;
        if( netif == NULL )
        {
            printf("network interface: %s not found!\r\n", netif_name);
            goto exit;
        }
    }
#if LWIP_VERSION_MAJOR == 1U /* v1.x */
    ip = (struct ip_addr *)&addr;
#else /* >= v2.x */
    ip = (ip4_addr_t *)&addr;
#endif /* LWIP_VERSION_MAJOR == 1U */

    /* set ip address */
    if ((ip_addr != NULL) && inet_aton(ip_addr, &addr))
    {
        netif_set_ipaddr(netif, ip);
    }

    /* set gateway address */
    if ((gw_addr != NULL) && inet_aton(gw_addr, &addr))
    {
        netif_set_gw(netif, ip);
    }

    /* set netmask address */
    if ((nm_addr != NULL) && inet_aton(nm_addr, &addr))
    {
        netif_set_netmask(netif, ip);
    }
exit:
    sys_arch_unprotect(cur);
}


void SetMtu(char * netif_name , char * Mtu_value)
{
    sys_prot_t cur;
    cur = sys_arch_protect();
    struct netif *netif = netif_list;

    if(strlen(netif_name) > sizeof(netif->name))
    {
        printf("network interface name too long! %d %s \r\n",strlen(netif_name),netif_name);
        goto exit;
    }
    while(netif != NULL)
    {
        if(strncmp(netif_name, netif->name, sizeof(netif->name)) == 0)
            break;

        netif = netif->next;
        if( netif == NULL )
        {
            printf("network interface: %s not found!\r\n", netif_name);
            goto exit;
        }
    }
    if(!Mtu_value)
    {
        printf("Input error : Missing max_value parameters! \n");
        goto exit;
    }
    if(Max_mtu == -1)
    {
        Max_mtu = netif->mtu;
    }
    
    int temp = atoi(Mtu_value);
    if (temp <= 0)
    {
        printf("Error: The mtu value input is wrong!\n");
    }
    else if(temp > Max_mtu)
    {
        printf("Error: The mtu value can not exceed %d !\n",Max_mtu);
    }
    else
    {
        netif->mtu = temp;
        printf("Mtu changed ,now is %d \n", netif->mtu);
    }

exit:
    sys_arch_unprotect(cur);
}

