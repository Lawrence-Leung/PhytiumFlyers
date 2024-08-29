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
 * FilePath: fgmac_os.h
 * Date: 2022-02-24 13:42:19
 * LastEditTime: 2022-03-25 09:16:53
 * Description:  This file is for gmac driver.Functions in this file are the minimum required functions for drivers.
 *
 * Modify History:
 *  Ver   Who        Date                   Changes
 * ----- ------    --------     --------------------------------------
 *  1.0  huanghe  2022/11/15    first release
 */

#ifndef FGMAC_OS_H
#define FGMAC_OS_H

#include <FreeRTOS.h>
#include <event_groups.h>
#include <semphr.h>

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

#define FREERTOS_GMAC_INIT_ERROR FT_CODE_ERR(ErrModPort, 0, 0x1)
#define FREERTOS_GMAC_PARAM_ERROR FT_CODE_ERR(ErrModPort, 0, 0x2)
#define FREERTOS_GMAC_NO_VALID_SPACE FT_CODE_ERR(ErrModPort, 0, 0x3)

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

/*irq priority value*/
#define GMAC_OS_IRQ_PRIORITY_VALUE (configMAX_API_CALL_INTERRUPT_PRIORITY+1)
FASSERT_STATIC((GMAC_OS_IRQ_PRIORITY_VALUE <= IRQ_PRIORITY_VALUE_15) && (GMAC_OS_IRQ_PRIORITY_VALUE >= configMAX_API_CALL_INTERRUPT_PRIORITY));

typedef struct
{
    u32 instance_id;
    u32 autonegotiation; /* 1 is autonegotiation ,0 is manually set */
    u32 phy_speed;  /* FXMAC_PHY_SPEED_XXX */
    u32 phy_duplex; /* FXMAC_PHY_XXX_DUPLEX */
} FtOsGmacPhyControl;

typedef struct
{
    FGmac instance;
    FtOsGmacPhyControl mac_config;

    u8 tx_buf[GMAC_TX_DESCNUM * FGMAC_MAX_PACKET_SIZE] __aligned(FGMAC_DMA_MIN_ALIGN);
    u8 rx_buf[GMAC_RX_DESCNUM * FGMAC_MAX_PACKET_SIZE] __aligned(FGMAC_DMA_MIN_ALIGN);
    u8 tx_desc[GMAC_TX_DESCNUM * sizeof(FGmacDmaDesc)] __aligned(FGMAC_DMA_MIN_ALIGN);
    u8 rx_desc[GMAC_RX_DESCNUM * sizeof(FGmacDmaDesc)] __aligned(FGMAC_DMA_MIN_ALIGN);

    u8 is_ready;   /* Ft_Os_Gmac Object first need Init use Ft_Os_GmacObjec_Init */
    SemaphoreHandle_t s_semaphore;    /*   Semaphore to signal incoming packets */
    EventGroupHandle_t s_status_event; /* Event Group to show netif's status ,follow FT_NETIF_XX*/
    struct LwipPort *stack_pointer; /* Docking data stack data structure */
    u8 hwaddr[FGMAX_MAX_HARDWARE_ADDRESS_LENGTH];
} FGmacOs;


FError FGmacOsInit(FGmacOs *instance_p);
FGmacOs *FGmacOsGetInstancePointer(FtOsGmacPhyControl *config_p);
FError FGmacOsConfig(FGmacOs *instance_p, int cmd, void *arg);
void *FGmacOsRx(FGmacOs *instance_p);
FError FGmacOsTx(FGmacOs *instance_p, void *tx_buf);
enum lwip_port_link_status FGmacPhyStatus(struct LwipPort *gmac_netif_p);
void FGmacOsStart(FGmacOs *instance_p);

#ifdef __cplusplus
}
#endif

#endif // ! FT_OS_GMAC_H
