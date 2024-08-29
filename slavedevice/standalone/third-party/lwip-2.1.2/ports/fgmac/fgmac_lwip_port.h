/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc. 
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
 * FilePath: fgmac_lwip_port.h
 * Date: 2022-02-24 13:42:19
 * LastEditTime: 2022-03-25 09:16:53
 * Description:  This file is gmac portable code for lwip port input,outpu,status check.
 * 
 * Modify History: 
 *  Ver     Who           Date                  Changes
 * -----   ------       --------     --------------------------------------
 *  1.0    huanghe      2022/11/2             first release
 */


#ifndef FGMAC_LWIP_PORT_H
#define FGMAC_LWIP_PORT_H


#include "fgmac.h"
#include "fgmac_hw.h"
#include "fgmac_phy.h"
#include "fparameters.h"
#include "lwip/netif.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* error code */
#define FGMAC_INIT_ERROR FT_CODE_ERR(ErrModPort, 0, 0x1)
#define FGMAC_PARAM_ERROR FT_CODE_ERR(ErrModPort, 0, 0x2)
#define FGMAC_NO_VALID_SPACE FT_CODE_ERR(ErrModPort, 0, 0x3)

#define FGMAX_MAX_HARDWARE_ADDRESS_LENGTH 6

#define FT_OS_GMAC0_ID FGMAC0_ID
#define FT_OS_GMAC1_ID FGMAC1_ID

#define FT_NETIF_LINKUP 0x1U
#define FT_NETIF_DOWN 0x2U

/** @defgroup ENET_Buffers_setting
  * @{
  */



#define GMAC_MTU            1500U

/* Common PHY Registers (AR8035) */
#define PHY_INTERRUPT_ENABLE_OFFSET ((u16)0x12)
#define PHY_INTERRUPT_ENABLE_LINK_FAIL BIT(11)  /* Link fail interrupt, 0  Interrupt disable , 1 Interrupt enable */


/* Phy */
#define FGMAC_PHY_SPEED_10M    10
#define FGMAC_PHY_SPEED_100M    100
#define FGMAC_PHY_SPEED_1000M    1000

#define FGMAC_PHY_HALF_DUPLEX   0
#define FGMAC_PHY_FULL_DUPLEX   1


/* dma */

#define GMAC_RX_DESCNUM     128
#define GMAC_TX_DESCNUM     128

typedef struct
{
    u32 instance_id;
    u32 autonegotiation; /* 1 is autonegotiation ,0 is manually set */
    u32 phy_speed;  /* FXMAC_PHY_SPEED_XXX */
    u32 phy_duplex; /* FXMAC_PHY_XXX_DUPLEX */
} FtLwipPortGmacPhyControl;

typedef struct
{
    FGmac instance;
    FtLwipPortGmacPhyControl mac_config;

    u8 tx_buf[GMAC_TX_DESCNUM * FGMAC_MAX_PACKET_SIZE] __aligned(FGMAC_DMA_MIN_ALIGN);
    u8 rx_buf[GMAC_RX_DESCNUM * FGMAC_MAX_PACKET_SIZE] __aligned(FGMAC_DMA_MIN_ALIGN);
    u8 tx_desc[GMAC_TX_DESCNUM * sizeof(FGmacDmaDesc)] __aligned(FGMAC_DMA_MIN_ALIGN);
    u8 rx_desc[GMAC_RX_DESCNUM * sizeof(FGmacDmaDesc)] __aligned(FGMAC_DMA_MIN_ALIGN);

    u8 is_ready;   /* Ft_LwipPort_Gmac Object first need Init use Ft_LwipPort_GmacObjec_Init */
    struct LwipPort *stack_pointer; /* Docking data stack data structure */
    u8 hwaddr[FGMAX_MAX_HARDWARE_ADDRESS_LENGTH];
} FGmacLwipPort;


FError FGmacLwipPortInit(FGmacLwipPort *instance_p);
FGmacLwipPort *FGmacLwipPortGetInstancePointer(FtLwipPortGmacPhyControl *config_p);
void *FGmacLwipPortRx(FGmacLwipPort *instance_p);
FError FGmacLwipPortTx(FGmacLwipPort *instance_p,void *tx_buf);
enum lwip_port_link_status FGmacPhyStatus(struct LwipPort *gmac_netif_p);
void FGmacLwipPortStart(FGmacLwipPort *instance_p);

#ifdef __cplusplus
}
#endif


#endif // !FGMAC_LWIP_PORT_H
