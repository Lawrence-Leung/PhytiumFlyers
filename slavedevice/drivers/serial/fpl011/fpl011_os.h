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
 * FilePath: fpl011_os.h
 * Date: 2022-02-24 13:42:19
 * LastEditTime: 2022-03-21 16:59:58
 * Description:  This file is for providing function related definitions of pl011 driver used in FreeRTOS.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe   2022/04/21   first commit
 */

#ifndef FPL011_OS_H
#define FPL011_OS_H

#include "FreeRTOS.h"
#include "event_groups.h"
#include "semphr.h"
#include "fpl011.h"
#include "fpl011_hw.h"
#include "ftypes.h"
#include "ferror_code.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FREERTOS_UART_SEM_ERROR FT_CODE_ERR(ErrModPort, 0, 0x1)
#define FREERTOS_UART_EVENT_ERROR FT_CODE_ERR(ErrModPort, 0, 0x2)
#define FREERTOS_UART_FIFO_ERROR FT_CODE_ERR(ErrModPort, 0, 0x3)
#define FREERTOS_UART_RECV_ERROR FT_CODE_ERR(ErrModPort, 0, 0x4)
#define FREERTOS_UART_INVAILD_PARAM FT_CODE_ERR(ErrModPort, 0, 0x5)

#define UART0_ID FUART0_ID
#define UART1_ID FUART1_ID
#define UART2_ID FUART2_ID
#define UART3_ID FUART3_ID

/*!
* @cond RTOS_PRIVATE
* @name UART FreeRTOS handler
*
* These are the only valid states for txEvent and rxEvent
*/
/*@{*/
/*! @brief Event flag - transfer complete. */
#define RTOS_UART_COMPLETE 0x1
/*! @brief Event flag - hardware buffer overrun. */
#define RTOS_UART_HARDWARE_BUFFER_OVERRUN 0x2
/*！ @brief Event flag Receive is error */
#define RTOS_UART_RECV_ERROR 0x4

/*@}*/

#define RTOS_UART_ISR_OEIM_MASK FPL011IMSC_OEIM  /* Overrun error interrupt mask.  */
#define RTOS_UART_ISR_BEIM_MASK FPL011IMSC_BEIM  /* Break error interrupt mask  */
#define RTOS_UART_ISR_PEIM_MASK FPL011IMSC_PEIM  /* Parity error interrupt mask.  */
#define RTOS_UART_ISR_FEIM_MASK FPL011IMSC_FEIM   /*  Framing error interrupt mask.  */
#define RTOS_UART_ISR_RTIM_MASK FPL011IMSC_RTIM   /* Receive timeout interrupt mask.   */
#define RTOS_UART_ISR_TXIM_MASK FPL011IMSC_TXIM   /* Transmit interrupt mask.  */
#define RTOS_UART_ISR_RXIM_MASK FPL011IMSC_RXIM   /*  Receive interrupt mask.  */

typedef struct
{
    u32 uart_instance; /* select uart global object */
    u32 isr_priority;  /* irq Priority */
    u32 isr_event_mask; /* followed by RTOS_UART_ISR_XX */
    u32 uart_baudrate;
} FtFreertosUartConfig;

typedef struct
{
    /* Uart Object */
    FPl011 bsp_uart;
    FtFreertosUartConfig config;
    SemaphoreHandle_t rx_semaphore; /*!< RX semaphore for resource sharing */
    SemaphoreHandle_t tx_semaphore; /*!< TX semaphore for resource sharing */
    EventGroupHandle_t rx_event;    /*!< RX completion event */
    EventGroupHandle_t tx_event;    /*!< TX completion event */
} FtFreertosUart;

void FtFreertosUartInit(FtFreertosUart *uart_p, FtFreertosUartConfig *config_p);
FError FtFreertosUartBlcokingSend(FtFreertosUart *uart_p, u8 *buffer, u32 length);
FError FtFreertosUartReceiveBuffer(FtFreertosUart *uart_p, u8 *buffer, u32 length, u32 *received_length);

#ifdef __cplusplus
}
#endif

#endif // !
