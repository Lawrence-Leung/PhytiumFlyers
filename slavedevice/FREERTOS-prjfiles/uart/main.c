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
 * FilePath: main.c
 * Date: 2022-02-24 13:42:19
 * LastEditTime: 2022-03-21 17:02:53
 * Description:  This file is for providing a template main.c file when creating new freertos examples.
 *
 * Modify History:
 *  Ver   Who            Date         Changes
 * ----- ------        --------    --------------------------------------
 *  1.0  wangxiaodong  2022/11/1    init commit
 */
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "ftypes.h"
// #include "shell.h"
// #include "shell_port.h"
#include "fpl011.h"
#include "fpl011_hw.h"
#include "fpl011_os.h"
#include "gps.h"

#define USART_REC_LEN  			200  	//定义最大接收字节数 200
#define EN_USART1_RX 			1		//使能（1）/禁止（0）串口1接收


#define TX_BUFFER   10000
#define RX_BUFFER   10000
static u8 send_buffer_i[TX_BUFFER] = "hello world!\r\n"; /*intr Buffer for Transmitting Data */
static u8 recv_buffer_i[RX_BUFFER]; /*intr Buffer for Receiving Data */
char rx[10];
u16 cnt = 0;
_SaveData Save_Data;

char USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
u16 USART_RX_STA;         		//接收状态标记	

FtFreertosUart uart;
FtFreertosUartConfig uart_config =
{
    .uart_instance = UART2_ID,              //TX pin8，RX pin10
    .isr_priority = IRQ_PRIORITY_VALUE_13,  /* irq Priority */
    .isr_event_mask = (RTOS_UART_ISR_OEIM_MASK | RTOS_UART_ISR_BEIM_MASK | RTOS_UART_ISR_PEIM_MASK | RTOS_UART_ISR_FEIM_MASK | RTOS_UART_ISR_RTIM_MASK | RTOS_UART_ISR_RXIM_MASK),
    .uart_baudrate = 9600
};

FPl011Format format =
{
    .baudrate = FPL011_BAUDRATE,
    .data_bits = FPL011_FORMAT_WORDLENGTH_8BIT,
    .parity = FPL011_FORMAT_NO_PARITY,
    .stopbits = FPL011_FORMAT_1_STOP_BIT
};


void UartWaitLoop(void)
{
    u32 recive_length = 0;
    u32 i,j = 0;

    while (TRUE)
    {
        FtFreertosUartReceiveBuffer(&uart, recv_buffer_i, sizeof(recv_buffer_i), &recive_length);
        for (i = 0; i < recive_length; i++)
        {
            // f_printk("%c",recv_buffer_i[i]);
            if(recv_buffer_i[i] == '$')
            {
                cnt = 0;
                // f_printk("/r/n");
            }
            USART_RX_BUF[cnt++] = recv_buffer_i[i];

            if(USART_RX_BUF[0] == '$' && USART_RX_BUF[4] == 'M' && USART_RX_BUF[5] == 'C')			//确定是否收到"GPRMC/GNRMC"这一帧数据
            {
                if(recv_buffer_i[i] == '\n')									   
                {
                    // for(j = 0;j<sizeof(USART_RX_BUF);j++)
                    // {
                    //     f_printk("%c",USART_RX_BUF[j]);
                    // }
                    // f_printk("\r\n");
                    memset(Save_Data.GPS_Buffer, 0, GPS_Buffer_Length);      //清空
                    // f_printk("cnt:%d\r\n",cnt);
                    memcpy(Save_Data.GPS_Buffer, USART_RX_BUF, cnt); 	//保存数据
                    // f_printk("Save_Data:");
                    // for(j = 0;j<sizeof(sizeof(Save_Data.GPS_Buffer));j++)
                    // {
                    //     f_printk("%c",Save_Data.GPS_Buffer[j]);
                    // }
                    // f_printk("\r\n");
                    Save_Data.isGetData = true;
                    cnt = 0;
                    memset(USART_RX_BUF, 0, USART_REC_LEN);      //清空	
                    
                }	
                        
            }
        }
        // f_printk("log:\r\n");
        parseGpsBuffer();
        printGpsBuffer();

    }
    vTaskDelete(NULL);
}


void UartTaskCreate(void *args)
{
    BaseType_t ret;
    FtFreertosUartInit(&uart,&uart_config);


    ret = xTaskCreate((TaskFunction_t)UartWaitLoop,  /* 任务入口函数 */
                      (const char *)"UartWaitLoop",/* 任务名字 */
                      (uint16_t)1024,  /* 任务栈大小 */
                      (void *)NULL,/* 任务入口函数参数 */
                      (UBaseType_t)2,  /* 任务的优先级 */
                      NULL); /* 任务控制块指针 */

    FASSERT_MSG(ret == pdPASS, "UartWaitLoop create is failed");

    vTaskDelete(NULL);
}

BaseType_t AppTask(void)
{
    return xTaskCreate((TaskFunction_t)UartTaskCreate,  /* 任务入口函数 */
                       (const char *)"LSUserShellTaskCreate",/* 任务名字 */
                       (uint16_t)1024,  /* 任务栈大小 */
                       (void *)NULL,/* 任务入口函数参数 */
                       (UBaseType_t)2,  /* 任务的优先级 */
                       NULL); /* 任务控制块指针 */
}

int main()
{
    printf("Hello main func,FT Date: %s, Time: %s\n", __DATE__, __TIME__);
    BaseType_t xReturn = pdPASS;
    // FError err = FT_SUCCESS;
    // // memset(&recv_buffer_i, 0, sizeof(recv_buffer_i));
    // FtFreertosUartInit(&uart,&uart_config);         //串口初始化
    // // send_buffer_i= "hello world!\r\n";
    // u32 rx_length = 0;
    // // err = FtFreertosUartReceiveBuffer(&uart,recv_buffer_i,1,&rx_length);
    // while (1)
    // {
    //     /* code */
    //     // FtFreertosUartBlcokingSend(&uart,"a",1);
    //     FPl011Send(&uart.bsp_uart,(u8*)"ok\n",4);
    //     // FtFreertosUartBlcokingSend(&uart,(u8*)send_buffer_i,sizeof(send_buffer_i));
    //     f_printk("recv:");
    //     FPl011Receive(&uart.bsp_uart,recv_buffer_i,RX_BUFFER);
    //     f_printk("freertos:1");
    //     err = FtFreertosUartReceiveBuffer(&uart,recv_buffer_i,sizeof(recv_buffer_i),&rx_length);
        
        
    // }
    
    
    // FSerialIntrInit(UART2_ID,&format,)

    xReturn = AppTask();
    if (xReturn != pdPASS)
    {
        goto FAIL_EXIT;
    }

    vTaskStartScheduler(); /* 启动任务，开启调度 */

    while (1); /* 正常不会执行到这里 */

FAIL_EXIT:
    printf("Failed,the xReturn value is 0x%x. \r\n", xReturn);
    return 0;
}


// void FPl011InterruptHandler(s32 vector, void *param)
// {
//     FPl011 *uart_p = (FPl011 *)param;
//     u32 reg_value = 0;
//     FASSERT(uart_p != NULL);
//     FASSERT(uart_p->is_ready == FT_COMPONENT_IS_READY);

//     reg_value = FUART_READREG32(uart_p->config.base_address, FPL011IMSC_OFFSET);
//     reg_value &= FUART_READREG32(uart_p->config.base_address, FPL011MIS_OFFSET);

//     if ((reg_value & ((u32)FPL011MIS_RXMIS)) != (u32)0)
//     {
//         /* Received data interrupt */
//         FPl011ReceiveDataHandler(uart_p);
//     }

//     if ((reg_value & ((u32)FPL011MIS_TXMIS)) != (u32)0)
//     {
//         /* Transmit data interrupt */
//         FPl011SendDataHandler(uart_p, reg_value);
//     }

//     if (((reg_value) & ((u32)FPL011MIS_OEMIS | (u32)FPL011MIS_BEMIS | (u32)FPL011MIS_PEMIS | (u32)FPL011MIS_FEMIS)) != (u32)0)
//     {
//         /* Received Error Status interrupt */
//         FPl011ReceiveErrorHandler(uart_p, reg_value);
//     }

//     if ((reg_value & ((u32)FPL011MIS_RTMIS)) != (u32)0)
//     {
//         /* Received Timeout interrupt */
//         FPl011ReceiveTimeoutHandler(uart_p);
//     }

//     if (((reg_value) & ((u32)FPL011MIS_DSRMMIS | (u32)FPL011MIS_DCDMMIS | (u32)FPL011MIS_CTSMMIS | (u32)FPL011MIS_RIMMIS)) != (u32)0)
//     {
//         /* Modem status interrupt */
//     }

//     /* Clear the interrupt status. */
//     FUART_WRITEREG32(uart_p->config.base_address, FPL011ICR_OFFSET,
//                      reg_value);

// }
