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
 * FilePath: fgcc_debug.c
 * Date: 2023-01-03 15:29:17
 * LastEditTime: 2023-01-03 15:29:17
 * Description:  This file is for c debug print in assembly language
 * 
 * Modify History: 
 *  Ver   Who  Date   Changes
 * ----- ------  -------- --------------------------------------
 *  1.0  huanghe	2021/11/13		initialization
 *  1.1  zhugengyu	2022/06/05		add debugging information		
 */


#include <stdio.h>
#include "fprintk.h"
#include "sdkconfig.h"
#ifdef CONFIG_USE_BACKTRACE
    #include "backtrace.h"
#endif
#include "fparameters.h"
#include "fearly_uart.h"


void SyncDoubleIn(void)
{
    f_printk("SyncDoubleIn \r\n") ;

#ifdef CONFIG_USE_BACKTRACE
    backtrace(__func__);
#endif

    while (1)
    {
        /* code */
    }

}

void FloatSave(void)
{

}


void HangPrint(void)
{
    OutByte('h') ;
    OutByte('a') ;
    OutByte('n') ;
    OutByte('g') ;
    while (1);
}

void TestCode(void)
{
    OutByte('h') ;
}
