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
 * FilePath: lwip_port.c
 * Date: 2022-10-25 02:18:08
 * LastEditTime: 2022-10-25 02:18:09
 * Description:  This file is part of lwip port. This file comtains the functions to Initialize,input,stop,dhcp lwip stack. 
 * 
 * Modify History: 
 *  Ver     Who           Date                  Changes
 * -----   ------       --------     --------------------------------------
 *  1.0    huanghe     2022/10/20            first release
 *  1.1   liuzhihong   2022/11/7     function and variable naming adjustment  
 */


#include <string.h>

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#include "lwipopts.h"

#include "lwip/mem.h"
#include "lwip/stats.h"
#include "lwip/sys.h"
#include "lwip/ip.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/priv/tcp_priv.h"
#include "lwip/ip_addr.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/dhcp.h"

#include "netif/etharp.h"

#include "lwip_port.h"
#include "fparameters.h"
#include "fprintk.h"
#include "fdebug.h"
#include "fassert.h"
#if !NO_SYS
#include "lwip/tcpip.h"
#endif

#define LWIP_PORT_DEBUG_TAG "LWIP-PORT"
#define LWIP_PORT_ERROR(format, ...)   FT_DEBUG_PRINT_E(LWIP_PORT_DEBUG_TAG, format, ##__VA_ARGS__)
#define LWIP_PORT_INFO(format, ...)    FT_DEBUG_PRINT_I(LWIP_PORT_DEBUG_TAG, format, ##__VA_ARGS__)
#define LWIP_PORT_DEBUG(format, ...)   FT_DEBUG_PRINT_D(LWIP_PORT_DEBUG_TAG, format, ##__VA_ARGS__)
#define LWIP_PORT_WARN(format, ...)    FT_DEBUG_PRINT_W(LWIP_PORT_DEBUG_TAG, format, ##__VA_ARGS__)

#define LWIP_PHY_DETECT_THREAD_NAME "_lwip_phy"
#define LWIP_RX_THREAD_NAME "_lwip_rx"
#define LWIP_MAX_NAME_LENGTH 32


#if !NO_SYS
#define THREAD_STACKSIZE 4096
#define LINK_DETECT_THREAD_INTERVAL_MSEC 1000 

static sys_thread_t dhcp_thread_handle;
static u32 dhcp_thread_created = 0;
void link_detect_thread(void *p);
#else
static u32 dhcp_trans_timeout_msec_cnt = 0 ; 
static u32 dhcp_lease_renewal_msec_cnt = 0 ;

#endif


/* Define those to better describe your network interface. */
#if defined(CONFIG_LWIP_FGMAC)
extern err_t ethernetif_gmac_init(struct netif *netif);
#endif
#if defined(CONFIG_LWIP_FXMAC)
extern err_t ethernetif_xmac_init(struct netif *netif);
#endif
/*
 * LwipPortAdd: this is a wrapper around lwIP's netif_add function.
 * The objective is to provide portability between the different MAC's
 * This function can be used to add both xps_ethernetlite and xps_ll_temac
 * based interfaces
 */
struct netif *LwipPortAdd(struct netif *netif,
	ip_addr_t *ipaddr, ip_addr_t *netmask, ip_addr_t *gw,
	unsigned char *mac_ethernet_address,
	UserConfig *user_config, u32 dhcp_en)
{
	int i;
	FASSERT(netif != NULL);
	FASSERT(user_config != NULL);
	err_t (* fun)(struct netif * netif);
	
	struct netif *netif_p;

	if(user_config->magic_code != LWIP_PORT_CONFIG_MAGIC_CODE)
	{
		LWIP_PORT_ERROR("user_config is illegitmacy");
		return NULL;
	}

	/* select mac type */
	if(user_config->driver_type == LWIP_PORT_TYPE_XMAC)
	{
#if defined(CONFIG_LWIP_FXMAC)
		fun = ethernetif_xmac_init;
#else
		LWIP_PORT_ERROR("LWIP_PORT_TYPE_XMAC is not activated ");
		return NULL;
#endif

	}else if(user_config->driver_type == LWIP_PORT_TYPE_GMAC)
	{
#if defined(CONFIG_LWIP_FGMAC)
		fun = ethernetif_gmac_init;
#else
		LWIP_PORT_ERROR("LWIP_PORT_TYPE_GMAC is not activated ");
		return NULL;
#endif
	}
	else
	{
		LWIP_PORT_ERROR("user_config is illegitmacy");
		return NULL;
	}


	/* set mac address */
	netif->hwaddr_len = 6;
	for (i = 0; i < 6; i++)
		netif->hwaddr[i] = mac_ethernet_address[i];

	netif->name[0] = user_config->name[0];
  	netif->name[1] = user_config->name[1];

#if (LWIP_DHCP==1)
	if(dhcp_en) 
	{
		ip_addr_set_zero(ipaddr);
		ip_addr_set_zero(netmask);
		ip_addr_set_zero(gw);
	}
#endif

	/* initialize based on MAC type */
	netif_p = netif_add(netif, 
#if LWIP_IPV4 && LWIP_IPV6
		&ipaddr->u_addr.ip4, &netmask->u_addr.ip4, &gw->u_addr.ip4,
#elif LWIP_IPV4
		ipaddr, netmask, gw,
#endif
		(void*)(uintptr)user_config,
		fun,
#if NO_SYS
		ethernet_input
#else
		tcpip_input
#endif
		);

	struct LwipPort *lwip_port;
#if !NO_SYS

	char detect_thread_name[LWIP_MAX_NAME_LENGTH] = {0}; /* detect thread name ,name + netif name */
	char rx_thread_name[LWIP_MAX_NAME_LENGTH] = {0}; /* detect thread name ,name + netif name */

	/* Start thread to detect link periodically for Hot Plug autodetect */
	if(netif_p)
	{

	#if defined(CONFIG_LWIP_PORT_USE_LINK_DETECT_THREAD)
			memcpy(detect_thread_name, netif->name, 2);
			strcpy(&detect_thread_name[2], LWIP_PHY_DETECT_THREAD_NAME);

			lwip_port = (struct LwipPort *)netif_p->state;
			lwip_port->detect_thread_handle = sys_thread_new(detect_thread_name, link_detect_thread, netif,
						CONFIG_LWIP_PORT_LINK_DETECT_STACKSIZE, CONFIG_LWIP_PORT_LINK_DETECT_PRIORITY);
	#endif
		/* create semaphore for rx thread */
	#if defined(CONFIG_LWIP_PORT_USE_RECEIVE_THREAD)
			sys_sem_new(&lwip_port->sem_rx_data_available, 0);

			memcpy(rx_thread_name, netif->name, 2);
			strcpy(&rx_thread_name[2], LWIP_RX_THREAD_NAME);

			lwip_port->rx_thread_handle = sys_thread_new(rx_thread_name, (lwip_thread_fn)LwipPortInputThread, netif,
						CONFIG_LWIP_PORT_RECEIVE_THREAD_STACKSIZE, CONFIG_LWIP_PORT_RECEIVE_THREAD_PRIORITY);
	#endif

	if(lwip_port->ops.eth_start)
	{
			/* start mac controller */
			lwip_port->ops.eth_start(netif_p);
	}
	}
	else
	{
		LWIP_PORT_ERROR("netif_add is failed");
	}
	
#else
	if(netif_p)
	{
		lwip_port = (struct LwipPort *)netif_p->state;
		if(lwip_port->ops.eth_start)
		{
			/* start mac controller */
			lwip_port->ops.eth_start(netif_p);
		}
	}
#endif

	return netif_p ;
}

#if !NO_SYS
/*
 * The input thread calls lwIP to process any received packets.
 * This thread waits until a packet is received (sem_rx_data_available),
 * and then calls LwipPortInput which processes 1 packet at a time.
 */
void LwipPortInputThread(struct netif *netif)
{
	struct LwipPort *emac;
	FASSERT(netif != NULL);
	emac = (struct LwipPort *)netif->state;
	vTaskDelay(1000);
	while (1)
	{
		/* sleep until there are packets to process
		 * This semaphore is set by the packet receive interrupt
		 * routine.
		 */
		sys_arch_sem_wait(&emac->sem_rx_data_available,0);
#if defined(CONFIG_LWIP_FXMAC)
		if(emac->ops.eth_poll)
		{
			emac->ops.eth_poll(netif);
		}
		else
		{
			LWIP_PORT_ERROR("emac->ops.eth_poll is null");
		}
#endif
		/* move all received packets to lwIP */
		if(emac->ops.eth_input)
		{
			emac->ops.eth_input(netif);
		}
		else
		{
			LWIP_PORT_ERROR("emac->ops.eth_input is null");
		}
	}
}
#endif

void LwipPortInput(struct netif *netif)
{
	struct LwipPort *emac;
	FASSERT(netif != NULL);
	FASSERT(netif->state != NULL);
	emac = (struct LwipPort *)netif->state;
	if(emac->ops.eth_input)
	{
		emac->ops.eth_input(netif);
	}
	else
	{
		LWIP_PORT_ERROR("emac->ops.eth_input is null");
	}
}


#if !NO_SYS

void link_detect_thread(void *p)
{
	FASSERT(p != NULL);
	struct netif *netif = (struct netif *) p;
	FASSERT(netif->state != NULL);
	struct LwipPort *emac = (struct LwipPort *)netif->state;

	while (1) {
		/* Call eth_link_detect() every second to detect Ethernet link
		 * change.
		 */
		if(emac->ops.eth_detect)
		{			
			switch(emac->ops.eth_detect(netif))
			{
				case ETH_LINK_UP:
					if(netif_is_link_up(netif) == 0)
					{
						LWIP_PORT_INFO("link up"); 
						netif_set_link_up(netif) ;
					}
				break;
				case ETH_LINK_DOWN:
				default:
					if(netif_is_link_up(netif) == 1)
					{
						LWIP_PORT_INFO("link down"); 
						netif_set_link_down(netif) ;
					}
					break;		
			}
		}
		else
		{
			LWIP_PORT_ERROR("emac->ops.eth_detect is null");
		}
	
		sys_arch_delay(LINK_DETECT_THREAD_INTERVAL_MSEC);
	}
}

#else

void LinkDetectLoop(struct netif *netif)
{
	FASSERT(netif != NULL);
	FASSERT(netif->state != NULL);
	struct LwipPort *emac = (struct LwipPort *)netif->state;

	/* Call eth_link_detect() every second to detect Ethernet link
		* change.
		*/
	if(emac->ops.eth_detect)
	{			
		switch(emac->ops.eth_detect(netif))
		{
			case ETH_LINK_UP:
				if(netif_is_link_up(netif) == 0)
				{
					LWIP_PORT_INFO("link up"); 
					netif_set_link_up(netif) ;
				}
			break;
			case ETH_LINK_DOWN:
			default:
				if(netif_is_link_up(netif) == 1)
				{
					LWIP_PORT_INFO("link down"); 
					netif_set_link_down(netif) ;
				}
			break;		
		}
	}
	else
	{
		LWIP_PORT_ERROR("emac->ops.eth_detect is null");
	}
}

#endif


void LwipPortStop(struct netif *netif)
{
	struct LwipPort *emac;
	FASSERT(netif != NULL);
	emac = (struct LwipPort *)netif->state;
	
#if (LWIP_DHCP==1)
	printf("dhcp_stop\n");
	dhcp_stop(netif);
	dhcp_cleanup(netif);
#endif
	if (emac->ops.eth_deinit)
	{
		/* remove mac controler resource */
		printf("eth_deinit\n");
		emac->ops.eth_deinit(netif);
	}
	else
	{
		LWIP_PORT_ERROR("emac->ops.eth_deinit is null");
	}
	printf("netif_remove\n");
	netif_remove(netif);
#if !NO_SYS
	/* delete rx thread */
	sys_thread_delete(emac->rx_thread_handle);
	/* delete detect thread */
	sys_thread_delete(emac->detect_thread_handle);
	sys_sem_free(&emac->sem_rx_data_available);
#endif
	printf("mem_free\n");
	mem_free(emac);
}

#if !NO_SYS
/* gCpuRuntime value from  freertos_configs.c */
extern volatile unsigned int gCpuRuntime;

__attribute__((weak)) u32_t sys_now(void)
{    
	return gCpuRuntime;
}
#else
__attribute__((weak)) u32_t sys_now(void)
{    
	return 0;
}
#endif

struct netif *LwipPortGetByName(const char *name)
{
	struct netif *netif = netif_list;


	if(strlen(name) > sizeof(netif->name))
    {
        LWIP_PORT_ERROR("network interface name too long! %d %s ",strlen(name),name);
        return NULL;
    }

	while(netif != NULL)
    {
        if(strncmp(name, netif->name, sizeof(netif->name)) == 0)
            break;

        netif = netif->next;
        if( netif == NULL )
        {
            LWIP_PORT_ERROR("network interface: %s not found!", name);
            return NULL;
        }
    }
	return netif;
}




#if (LWIP_DHCP==1)

#if !NO_SYS

	void LwipDhcpThread(void *p)
	{
		int mscnt = 0;
		(void *)(p);
		
		while (1)
		{
			sys_arch_delay(DHCP_FINE_TIMER_MSECS);
			dhcp_fine_tmr();
			mscnt += DHCP_FINE_TIMER_MSECS;
			if (mscnt >= DHCP_COARSE_TIMER_SECS*1000)
			{
				dhcp_coarse_tmr();
				mscnt = 0;
			}
		}
	}
	
#else
/**
 * @name: LwipPortDhcpLoop
 * @msg:  
 * @return {*}
 * @note: 
 * @param {u32} loop_freq
 */
void LwipPortDhcpLoop(u32 period_msec_cnt)
{
	dhcp_trans_timeout_msec_cnt += period_msec_cnt;
	dhcp_lease_renewal_msec_cnt += period_msec_cnt;
	if (dhcp_trans_timeout_msec_cnt >= DHCP_FINE_TIMER_MSECS )
	{
		dhcp_fine_tmr();
		dhcp_trans_timeout_msec_cnt = 0;
	}

	if(dhcp_lease_renewal_msec_cnt >= (DHCP_COARSE_TIMER_SECS * 1000))
	{
		dhcp_coarse_tmr();
		dhcp_lease_renewal_msec_cnt = 0;
	}
}

#endif

#endif
void LwipPortDhcpSet(struct netif *netif ,boolean is_enabled) 
{
	#if LWIP_DHCP
	struct LwipPort *emac;
	FASSERT(netif != NULL);
	emac = (struct LwipPort *)netif->state;

    if(TRUE == is_enabled)
    {
        dhcp_start((struct netif *)netif);

		#if !NO_SYS
#if defined(CONFIG_LWIP_PORT_DHCP_THREAD)
			if(dhcp_thread_created == 0)
			{
				dhcp_thread_handle = sys_thread_new("lwip_port_dhcp", LwipDhcpThread, NULL,
				CONFIG_LWIP_PORT_DHCP_STACKSIZE, CONFIG_LWIP_PORT_DHCP_PRIORITY);
				dhcp_thread_created = 1;
			}
#endif
		#endif
	}
    else
    {
        dhcp_stop((struct netif *)netif);
    }
	#endif
}


void lwip_port_init(void)
{
#if !NO_SYS
	dhcp_thread_handle = NULL;
	dhcp_thread_created = 0;
#else
	dhcp_trans_timeout_msec_cnt = 0;
	dhcp_lease_renewal_msec_cnt = 0;
#endif

	
}

void lwip_port_deinit(void)
{
#if !NO_SYS
	if(dhcp_thread_handle)
	{
		sys_thread_delete(dhcp_thread_handle);		
	}
	dhcp_thread_handle = NULL;
#endif
}


void LwipPortDebug(const char *name)
{
	struct netif *netif = LwipPortGetByName(name);
	struct LwipPort *emac;
	if(netif == NULL)
	{
		LWIP_PORT_ERROR("%s was not initialized successfully",name);
		return;
	}
	
	emac = (struct LwipPort *)netif->state;
	if(emac->ops.eth_debug)
	{
		emac->ops.eth_debug(netif);
	}
	else
	{
		LWIP_PORT_ERROR("eth debug function is not registered");
		return;
	}
}

void LwipEthProcessLoop(struct netif *netif)
{
	struct LwipPort *emac;
	FASSERT(netif != NULL);
	FASSERT(netif->state != NULL);
	emac = (struct LwipPort *)netif->state;
	if(emac->ops.eth_poll)
	{
		emac->ops.eth_poll(netif);
	}
	else
	{
		LWIP_PORT_ERROR("emac->ops.eth_poll is null");
	}
}
