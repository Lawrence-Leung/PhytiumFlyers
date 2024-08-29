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
 * FilePath: fpl011_os.c
 * Date: 2022-02-24 13:42:19
 * LastEditTime: 2022-03-21 16:59:51
 * Description:  This file is for required function implementations of pl011 driver used in FreeRTOS.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe   2022/04/21   first commit
 */
#include <stdio.h>
#include <string.h>
#include "fpl011_os.h"
#include "fpl011.h"
#include "fpl011_hw.h"
#include "finterrupt.h"
#include "ftypes.h"
#include "fassert.h"
#include "fdebug.h"
#include "sdkconfig.h"
#include "fcpu_info.h"

#define FPL011_DEBUG_TAG "FFreeRTOSPl001"
#define FPL011_ERROR(format, ...)   FT_DEBUG_PRINT_E(FPL011_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPL011_WARN(format, ...)   FT_DEBUG_PRINT_W(FPL011_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPL011_INFO(format, ...) FT_DEBUG_PRINT_I(FPL011_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPL011_DEBUG(format, ...) FT_DEBUG_PRINT_D(FPL011_DEBUG_TAG, format, ##__VA_ARGS__)

/* Callback events  */
static void FPl011IrqClearReciveTimeOut(FPl011 *uart_p)
{
    u32 reg_temp;
    reg_temp = FPl011GetInterruptMask(uart_p);
    reg_temp &= ~FPL011MIS_RTMIS;
    FPl011SetInterruptMask(uart_p, reg_temp);
}

static void FPl011IrqEnableReciveTimeOut(FPl011 *uart_p)
{
    u32 reg_temp;
    reg_temp = FPl011GetInterruptMask(uart_p);
    reg_temp |= FPL011MIS_RTMIS;
    FPl011SetInterruptMask(uart_p, reg_temp);
}

static void FtFreeRtosUartCallback(void *args, u32 event, u32 event_data)
{

    FtFreertosUart *uart_p = (FtFreertosUart *)args;
    BaseType_t xhigher_priority_task_woken = pdFALSE;
    BaseType_t x_result = pdFALSE;
    if (FPL011_EVENT_RECV_DATA == event || FPL011_EVENT_RECV_TOUT == event)
    {
        x_result = xEventGroupSetBitsFromISR(uart_p->rx_event, RTOS_UART_COMPLETE, &xhigher_priority_task_woken);
    }
    else if (FPL011_EVENT_RECV_ERROR == event)
    {
        x_result = xEventGroupSetBitsFromISR(uart_p->rx_event, RTOS_UART_RECV_ERROR, &xhigher_priority_task_woken);
    }
    else if (FPL011_EVENT_SENT_DATA == event)
    {
        x_result = xEventGroupSetBitsFromISR(uart_p->tx_event, RTOS_UART_COMPLETE, &xhigher_priority_task_woken);
    }
    else if (FPL011_EVENT_PARE_FRAME_BRKE == event)
    {
        x_result = xEventGroupSetBitsFromISR(uart_p->rx_event, RTOS_UART_RECV_ERROR, &xhigher_priority_task_woken);
    }
    else if (FPL011_EVENT_RECV_ORERR == event)
    {
    }

    if (FPL011_EVENT_SENT_DATA == event)
    {
    }
    else
    {
        FPl011IrqClearReciveTimeOut(&uart_p->bsp_uart);
    }

    if (x_result != pdFAIL)
    {
        portYIELD_FROM_ISR(xhigher_priority_task_woken);
    }

}

void FtFreertosUartInit(FtFreertosUart *uart_p, FtFreertosUartConfig *config_p)
{
    FPl011 *bsp_uart_p = NULL;
    FError ret;
    u32 intr_mask;
    u32 cpu_id = 0;
    FPl011Config driver_config;
    FASSERT(uart_p != NULL);
    FASSERT(config_p != NULL);
    bsp_uart_p = &uart_p->bsp_uart;
    uart_p->config = *config_p;
    driver_config = *FPl011LookupConfig(config_p->uart_instance);

    driver_config.baudrate = config_p->uart_baudrate;
    ret = FPl011CfgInitialize(bsp_uart_p, &driver_config);
    FASSERT(FT_SUCCESS == ret);

    FPl011SetHandler(bsp_uart_p, FtFreeRtosUartCallback, uart_p);
    FASSERT((uart_p->rx_semaphore = xSemaphoreCreateMutex()) != NULL);
    FASSERT((uart_p->tx_semaphore = xSemaphoreCreateMutex()) != NULL);
    FASSERT((uart_p->tx_event = xEventGroupCreate()) != NULL);
    FASSERT((uart_p->rx_event = xEventGroupCreate()) != NULL);

    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(bsp_uart_p->config.irq_num, cpu_id);

    FPl011SetRxFifoThreadhold(bsp_uart_p, FPL011IFLS_RXIFLSEL_1_4);
    FPl011SetTxFifoThreadHold(bsp_uart_p, FPL011IFLS_TXIFLSEL_1_2);
    intr_mask = config_p->isr_event_mask;
    FPl011SetInterruptMask(bsp_uart_p, intr_mask);
    FPl011SetOptions(bsp_uart_p, FPL011_OPTION_UARTEN | FPL011_OPTION_RXEN | FPL011_OPTION_TXEN | FPL011_OPTION_FIFOEN);

    InterruptSetPriority(bsp_uart_p->config.irq_num, config_p->isr_priority);
    InterruptInstall(bsp_uart_p->config.irq_num, FPl011InterruptHandler, bsp_uart_p, "uart1");
    InterruptUmask(bsp_uart_p->config.irq_num);
}

FError FtFreertosUartReceiveBuffer(FtFreertosUart *uart_p, u8 *buffer, u32 length, u32 *received_length)
{
    // f_printk("FtFreertosUartReceiveBuffer");
    u32 get_length;
    FError ret = FT_SUCCESS;
    FPl011 *bsp_uart_p = NULL;
    EventBits_t ev;
    FASSERT(NULL != uart_p);
    FASSERT(NULL != buffer);
    bsp_uart_p = &uart_p->bsp_uart;

    if (length == 0)
    {
        *received_length = 0;
        return FT_SUCCESS;
    }

    /* New transfer can be performed only after current one is finished */
    if (pdFALSE == xSemaphoreTake(uart_p->rx_semaphore, portMAX_DELAY))
    {
        /* We could not take the semaphore, exit with 0 data received */
        ret = FREERTOS_UART_SEM_ERROR;
    }



    if (uart_p->config.isr_event_mask & (RTOS_UART_ISR_RTIM_MASK | RTOS_UART_ISR_RXIM_MASK))
    {
        get_length = FPl011Receive(bsp_uart_p, buffer, length);
        // f_printk("get_length:%d",get_length);
        if (get_length > 0)
        {
            *received_length = get_length;
            ret = FT_SUCCESS;
        }
        else
        {
            FPl011IrqEnableReciveTimeOut(bsp_uart_p);
            ev = xEventGroupWaitBits(uart_p->rx_event,
                                     RTOS_UART_COMPLETE | RTOS_UART_HARDWARE_BUFFER_OVERRUN | RTOS_UART_RECV_ERROR,
                                     pdTRUE, pdFALSE, portMAX_DELAY);
            if (ev & RTOS_UART_HARDWARE_BUFFER_OVERRUN)
            {
                ret = FREERTOS_UART_FIFO_ERROR;
                *received_length = 0;
            }
            else if (ev & RTOS_UART_COMPLETE)
            {
                ret = FT_SUCCESS;
                *received_length = bsp_uart_p->receive_buffer.requested_bytes - bsp_uart_p->receive_buffer.remaining_bytes;
            }
            else if (ev & RTOS_UART_RECV_ERROR)
            {
                ret = FREERTOS_UART_RECV_ERROR;
                *received_length = 0;
            }
            else
            {
                ret = FREERTOS_UART_INVAILD_PARAM;
                *received_length = 0;
            }
        }
    }
    else
    {
        buffer[0] = FPl011BlockReceive(bsp_uart_p);
        *received_length = 1;
        ret = FT_SUCCESS;
    }

    /* Enable next transfer. Current one is finished */
    if (pdFALSE == xSemaphoreGive(uart_p->rx_semaphore))
    {
        /* We could not post the semaphore, exit with error */
        FPL011_ERROR("FST_FAILURE xSemaphoreGive.");
        ret = FREERTOS_UART_RECV_ERROR;
    }

    return ret;
}


FError FtFreertosUartBlcokingSend(FtFreertosUart *uart_p, u8 *buffer, u32 length)
{
    FError ret = FT_SUCCESS;
    FPl011 *bsp_uart_p = NULL;
    EventBits_t ev;
    u32 send_length;
    FASSERT(NULL != uart_p);
    FASSERT(NULL != buffer);
    bsp_uart_p = &uart_p->bsp_uart;

    if (pdFALSE == xSemaphoreTake(uart_p->tx_semaphore, portMAX_DELAY))
    {
        return FREERTOS_UART_SEM_ERROR;
    }

    if (uart_p->config.isr_event_mask & RTOS_UART_ISR_TXIM_MASK)
    {
        send_length = FPl011Send(bsp_uart_p, buffer, length);
        if (send_length != length)
        {
            ev = xEventGroupWaitBits(uart_p->tx_event, RTOS_UART_COMPLETE, pdTRUE, pdFALSE, portMAX_DELAY);
            if (!(ev & RTOS_UART_COMPLETE))
            {
                ret = FREERTOS_UART_EVENT_ERROR;
            }
        }
    }
    else
    {
        FPl011BlockSend(bsp_uart_p, buffer, length);
    }

    if (pdFALSE == xSemaphoreGive(uart_p->tx_semaphore))
    {
        /* We could not post the semaphore, exit with error */
        ret = FREERTOS_UART_SEM_ERROR;
    }

    return ret;
}
