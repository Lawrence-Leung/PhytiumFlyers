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
 * FilePath: fxmac_os.h
 * Date: 2022-07-15 16:33:19
 * LastEditTime: 2022-07-15 16:33:19
 * Description:  This file is for xmac driver.Functions in this file are the minimum required functions for drivers.
 *
 * Modify History:
 *  Ver   Who        Date                   Changes
 * ----- ------    --------     --------------------------------------
 *  1.0  huanghe  2022/11/15            first release
 */

#ifndef FXMAC_OS_H
#define FXMAC_OS_H

#include <FreeRTOS.h>
#include <event_groups.h>
#include <semphr.h>
#include "fxmac.h"
#include "fkernel.h"
#include "ferror_code.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FREERTOS_XMAC_INIT_ERROR FT_CODE_ERR(ErrModPort, 0, 0x1)
#define FREERTOS_XMAC_PARAM_ERROR FT_CODE_ERR(ErrModPort, 0, 0x2)
#define FREERTOS_XMAC_NO_VALID_SPACE FT_CODE_ERR(ErrModPort, 0, 0x3)

#define FXMAX_RX_BDSPACE_LENGTH    0x20000 /* default set 128KB*/
#define FXMAX_TX_BDSPACE_LENGTH    0x20000 /* default set 128KB*/

#define FXMAX_RX_PBUFS_LENGTH       128
#define FXMAX_TX_PBUFS_LENGTH       128

#define FXMAX_MAX_HARDWARE_ADDRESS_LENGTH 6

/* configuration */
#define FXMAC_OS_CONFIG_JUMBO  BIT(0)
#define FXMAC_OS_CONFIG_MULTICAST_ADDRESS_FILITER  BIT(1) /* Allow multicast address filtering  */
#define FXMAC_OS_CONFIG_COPY_ALL_FRAMES BIT(2) /* enable copy all frames */
#define FXMAC_OS_CONFIG_CLOSE_FCS_CHECK BIT(3) /* close fcs check */
#define FXMAC_OS_CONFIG_RX_POLL_RECV BIT(4)  /* select poll mode */
/* Phy */
#define FXMAC_PHY_SPEED_10M    10
#define FXMAC_PHY_SPEED_100M    100
#define FXMAC_PHY_SPEED_1000M    1000

#define FXMAC_PHY_HALF_DUPLEX   0
#define FXMAC_PHY_FULL_DUPLEX   1

#define MAX_FRAME_SIZE_JUMBO (FXMAC_MTU_JUMBO + FXMAC_HDR_SIZE + FXMAC_TRL_SIZE)

/* Byte alignment of BDs */
#define BD_ALIGNMENT (FXMAC_DMABD_MINIMUM_ALIGNMENT*2)

/*  frame queue */
#define PQ_QUEUE_SIZE 4096

/*irq priority value*/
#define XMAC_OS_IRQ_PRIORITY_VALUE (configMAX_API_CALL_INTERRUPT_PRIORITY+1)
FASSERT_STATIC((XMAC_OS_IRQ_PRIORITY_VALUE <= IRQ_PRIORITY_VALUE_15)&&(XMAC_OS_IRQ_PRIORITY_VALUE >= configMAX_API_CALL_INTERRUPT_PRIORITY));

typedef struct 
{
    uintptr data[PQ_QUEUE_SIZE];
    int head, tail, len;
} PqQueue;

typedef enum
{
    FXMAC_OS_INTERFACE_SGMII = 0,
    FXMAC_OS_INTERFACE_RMII,
    FXMAC_OS_INTERFACE_RGMII,
    FXMAC_OS_INTERFACE_LENGTH
} FXmacFreeRtosInterface;


typedef struct
{
    u8 rx_bdspace[FXMAX_RX_BDSPACE_LENGTH] __attribute__((aligned(128))); /* 接收bd 缓冲区 */
    u8 tx_bdspace[FXMAX_RX_BDSPACE_LENGTH] __attribute__((aligned(128))); /* 发送bd 缓冲区 */

    uintptr rx_pbufs_storage[FXMAX_RX_PBUFS_LENGTH];
    uintptr tx_pbufs_storage[FXMAX_TX_PBUFS_LENGTH];

} FXmacNetifBuffer;

typedef struct
{
    u32 instance_id;
    FXmacFreeRtosInterface interface;
    u32 autonegotiation; /* 1 is autonegotiation ,0 is manually set */
    u32 phy_speed;  /* FXMAC_PHY_SPEED_XXX */
    u32 phy_duplex; /* FXMAC_PHY_XXX_DUPLEX */
} FXmacOsControl;

typedef struct
{
    FXmac instance;
    FXmacOsControl mac_config;

    FXmacNetifBuffer buffer;

    /* queue to store overflow packets */
    PqQueue recv_q;
    PqQueue send_q;

    /* configuration */
    u32 config;

    struct LwipPort *stack_pointer; /* Docking data stack data structure */
    u8 hwaddr[FXMAX_MAX_HARDWARE_ADDRESS_LENGTH];
} FXmacOs;

FXmacOs *FXmacOsGetInstancePointer(FXmacOsControl *config_p);
FError FXmacOsInit(FXmacOs *instance_p);
FError FXmacOsConfig(FXmacOs *instance_p, int cmd, void *arg);
void *FXmacOsRx(FXmacOs *instance_p);
FError FXmacOsTx(FXmacOs *instance_p, void *tx_buf);
void FXmacOsStop(FXmacOs *instance_p);
void FXmacOsStart(FXmacOs *instance_p);
void FXmacOsRecvHandler(FXmacOs *instance_p);
enum lwip_port_link_status FXmacPhyReconnect(struct LwipPort *xmac_netif_p);

#ifdef __cplusplus
}
#endif

#endif // !
