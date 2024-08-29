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
 * FilePath: fpl011_os_port.c
 * Date: 2022-02-24 21:42:27
 * LastEditTime: 2022-02-24 21:42:27
 * Description:  This file is for letter shell port to serial pl011
 *
 * Modify History:
 *  Ver   Who           Date         Changes
 * ----- ------       --------    --------------------------------------
 * 1.0   huanghe       2022/4/21   first release
 * 1.1   wangxiaodong  2022/6/20   improve functions,adapt E2000
 */


#include <string.h>
#include "fassert.h"
#include "fparameters.h"
#include "shell_port.h"
#include "finterrupt.h"
#include "fpl011_os.h"

FtFreertosUart os_uart1;
extern Shell shell_object;
static char data[64];


#ifdef CONFIG_DEFAULT_LETTER_SHELL_USE_UART1
    #define LETTER_SHELL_UART_ID    UART1_ID
#endif

#ifdef CONFIG_DEFAULT_LETTER_SHELL_USE_UART0
    #define LETTER_SHELL_UART_ID    UART0_ID
#endif

#ifdef CONFIG_DEFAULT_LETTER_SHELL_USE_UART2
    #define LETTER_SHELL_UART_ID    UART2_ID
#endif

extern void FtFreertosUartIntrInit(FtFreertosUart *uart_p);

/**
 * @brief 用户shell写
 *
 * @param data 数据
 */
void LSUserShellWrite(char data)
{
    // FtFreertosUartBlcokingSend(&os_uart1, &data, 1);
    FPl011Send(&os_uart1.bsp_uart, &data, 1);
}

/**
 * @brief 用户shell读
 *
 * @param data 数据
 * @return char 状态
 */
signed char LSUserShellRead(char *data)
{
    u32 received_length;
    FtFreertosUartReceiveBuffer(&os_uart1, data, 1, &received_length);
    return 0;
}

void LSSerialConfig(void)
{
    FtFreertosUartConfig config =
    {
        .uart_instance = LETTER_SHELL_UART_ID, /* select uart global object */
        .isr_priority = IRQ_PRIORITY_VALUE_13,  /* irq Priority */
        .isr_event_mask = (RTOS_UART_ISR_OEIM_MASK | RTOS_UART_ISR_BEIM_MASK | RTOS_UART_ISR_PEIM_MASK | RTOS_UART_ISR_FEIM_MASK | RTOS_UART_ISR_RTIM_MASK | RTOS_UART_ISR_RXIM_MASK),
        .uart_baudrate = 115200
    };
    FtFreertosUartInit(&os_uart1, &config);
}

void LSSerialWaitLoop(void)
{
    u32 recive_length = 0;
    u32 i = 0;

    while (TRUE)
    {
        FtFreertosUartReceiveBuffer(&os_uart1, data, sizeof(data), &recive_length);
        for (i = 0; i < recive_length; i++)
        {
            shellHandler(&shell_object, data[i]);
        }
    }
    vTaskDelete(NULL);
}
