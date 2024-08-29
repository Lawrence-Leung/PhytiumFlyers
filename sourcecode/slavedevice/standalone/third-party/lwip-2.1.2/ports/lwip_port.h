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
 * FilePath: lwip_port.h
 * Date: 2022-10-25 02:18:02
 * LastEditTime: 2022-10-25 02:18:03
 * Description:  This file is part of lwip port. This file comtains the functions to Initialize,input,stop,dhcp lwip stack. 
 * 
 * Modify History: 
 *  Ver     Who           Date                  Changes
 * -----   ------       --------     --------------------------------------
 *  1.0    huanghe     2022/10/20            first release
 *  1.1   liuzhihong   2022/11/7     function and variable naming adjustment
 */

#ifndef __LWIP_PORT_H_
#define __LWIP_PORT_H_


#include "ftypes.h"
#include "lwipopts.h"

#include "lwip/netif.h"
#include "lwip/ip.h"

#ifdef __cplusplus
extern "C" {
#endif

/* network interface device status */

enum lwip_port_link_status {
	ETH_LINK_UNDEFINED = 0,
	ETH_LINK_UP,
	ETH_LINK_DOWN,
	ETH_LINK_NEGOTIATING
};


/* config */

/* driver type */
#define LWIP_PORT_TYPE_XMAC 0
#define LWIP_PORT_TYPE_GMAC 1

/* Mii interface */
#define LWIP_PORT_INTERFACE_RGMII 0
#define LWIP_PORT_INTERFACE_SGMII 1
#define LWIP_PORT_INTERFACE_USX	2

/* Phy speed */
#define LWIP_PORT_SPEED_10M    10
#define LWIP_PORT_SPEED_100M    100
#define LWIP_PORT_SPEED_1000M    1000
#define LWIP_PORT_SPEED_10G    10000


/* Duplex */
#define LWIP_PORT_HALF_DUPLEX    0
#define LWIP_PORT_FULL_DUPLEX    1

#define LWIP_PORT_CONFIG_MAGIC_CODE 0x616b6200

typedef struct
{
	u32 magic_code;		/* LWIP_PORT_CONFIG_MAGIC_CODE */
	char name[2];		/* Used to name netif */
	u32 driver_type;	/* driver type */
	u32 mac_instance;	/* mac controler id */
	u32 mii_interface;  /* LWIP_PORT_INTERFACE_XXX */
    u32 autonegotiation; /* 1 is autonegotiation ,0 is manually set */
    u32 phy_speed;  /* LWIP_PORT_SPEED_XXX */
    u32 phy_duplex; /* LWIP_PORT_XXX_DUPLEX */
} UserConfig;

#define LWIP_PORT_CONFIG_DEFAULT_INIT(config)           \
	do                                                    \
	{                                                     \
		config.magic_code = LWIP_PORT_CONFIG_MAGIC_CODE;  \
		config.driver_type = LWIP_PORT_TYPE_XMAC;		  \
		config.mac_instance = 3;                          \
		config.mii_interface = LWIP_PORT_INTERFACE_RGMII; \
		config.autonegotiation = 1;                       \
		config.phy_speed = LWIP_PORT_SPEED_1000M;         \
		config.phy_duplex = LWIP_PORT_FULL_DUPLEX; \
}while(0)


typedef struct
{
	void (*eth_input)(struct netif *netif);/*LwipTestLoop call*/
	enum lwip_port_link_status (*eth_detect)(struct netif *netif);/*LwipPortInput call*/
	void (*eth_deinit)(struct netif *netif);/*LwipPortStop call*/
	void (*eth_start)(struct netif *netif); /*LwipPortAdd call*/
	void (*eth_debug)(struct netif *netif); 
	void (*eth_poll)(struct netif *netif); 
} LwipPortOps;



struct LwipPort {
	void *state; /* mac controler */
#if !NO_SYS
	sys_sem_t sem_rx_data_available;
	sys_thread_t detect_thread_handle;
	sys_thread_t rx_thread_handle;
#endif
	LwipPortOps ops;
};

struct netif *LwipPortAdd(struct netif *netif,
							ip_addr_t *ipaddr, ip_addr_t *netmask, ip_addr_t *gw,
							unsigned char *mac_ethernet_address,
							UserConfig *user_config, u32 dhcp_en);
void LwipPortInput(struct netif *netif);

#if !NO_SYS
void LwipPortInputThread(struct netif *netif);
#else
void LwipEthProcessLoop(struct netif *netif);
void LinkDetectLoop(struct netif *netif);
#endif

void LwipPortStop(struct netif *netif);
struct netif *LwipPortGetByName(const char *name);
void LwipPortDhcpSet(struct netif *netif, boolean is_enabled);


#if !NO_SYS
void LwipDhcpThread(void *p);
#else
void LwipPortDhcpLoop(u32 period_msec_cnt) ;
#endif

void LwipPortDebug(const char *name);

#ifdef __cplusplus
}
#endif

#endif
