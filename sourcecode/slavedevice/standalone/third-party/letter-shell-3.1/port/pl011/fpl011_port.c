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
 * FilePath: fpl011_port.c
 * Date: 2022-02-10 14:53:43
 * LastEditTime: 2022-02-25 11:47:23
 * Description:  This files is for letter shell port to serial pl011
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2021/9/6     init commit
 * 1.1   huanghe    2022/1/5     support config letter shell run uart
 */


#include <string.h>
#include "fassert.h"
#include "fparameters.h"
#include "../shell_port.h"
#include "finterrupt.h"
#include "fpl011.h"
#include "sdkconfig.h"

static FPl011 serial;
extern Shell shell_object;

#if defined(CONFIG_DEFAULT_LETTER_SHELL_USE_UART2)
#define LETTER_SHELL_UART_ID    FUART2_ID
#elif defined(CONFIG_DEFAULT_LETTER_SHELL_USE_UART0)
#define LETTER_SHELL_UART_ID    FUART0_ID
#else
#define LETTER_SHELL_UART_ID    FUART1_ID
#endif




/**
 * @brief 用户shell写
 *
 * @param data 数据
 */
void LSUserShellWrite(char data)
{
    FPl011BlockSend(&serial, &data, 1);
}

/**
 * @brief 用户shell读
 *
 * @param data 数据
 * @return char 状态
 */
signed char LSUserShellRead(char *data)
{
    u32 length = 0;
    length = FPl011Receive(&serial,data,1);
    return (length > 0)? 1:0;
}

/**
 * @brief 用户shell读
 *
 * @param data 数据
 * @return char 状态
 */

static void LSUartIrqHandler(s32 vector, void *param)
{
    FPl011InterruptHandler(vector,param);
}


void LSSerialConfig(void)
{
    s32 ret = FT_SUCCESS;
    const FPl011Config * config_p;
    FPl011Config config_value;
    memset(&serial, 0, sizeof(serial));
    config_p = FPl011LookupConfig(LETTER_SHELL_UART_ID) ;
    memcpy(&config_value,config_p,sizeof(FPl011Config)); 
    /* 初始化PL011 */
    ret = FPl011CfgInitialize(&serial, &config_value);
    FASSERT(FT_SUCCESS == ret);
    FPl011SetOptions(&serial, FPL011_OPTION_UARTEN | FPL011_OPTION_RXEN | FPL011_OPTION_TXEN | FPL011_OPTION_FIFOEN );
    return;
}

/**
 * @brief 串口有数据输入时就执行相对应的命令,没有数据就阻塞在内部循环中
 *
 * @param 无
 * @return 无
 */

void LSSerialWaitLoop(void) 
{
    char data;
    while (TRUE)
    {
        data = FPl011BlockReceive(&serial);
        shellHandler(&shell_object, data);
    }
}

/**
 * @brief 串口有数据输入时就执行相对应的命令,没有数据程序会正常返回，不会等待
 *
 * @param 无
 * @return 无
 */

void LSSerialBNoWaitLoop(void)
{
    char data[16] = {0};
    u32 get_length = 0,index = 0;
    get_length = FPl011Receive(&serial, data, 16);
    
    while (get_length)
    {
        shellHandler(&shell_object, data[index]);
        index++;
        get_length--;
    }
}