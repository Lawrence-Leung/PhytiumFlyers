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
 * FilePath: early_uart.c
 * Date: 2022-02-11 13:33:28
 * LastEditTime: 2022-02-17 17:59:26
 * Description:  This file is for
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   rtos       2022/6/25    init commit
 * 1.1   zhangyan   2023/7/11    modify
 */

/***************************** Include Files *********************************/
#include "fkernel.h"
#include "fio.h"
#include "fparameters.h"
#include "fearly_uart.h"
#include "fpl011.h"

/**************************** Type Definitions *******************************/
static FPl011 early_uart;

void FEarlyUartProbe(void)
{
    FPl011Config config;
    config = *FPl011LookupConfig(EARLY_UART_CTRL_ID);
    FPl011CfgInitialize(&early_uart, &config);
    FPl011SetOptions(&early_uart, FPL011_OPTION_UARTEN | FPL011_OPTION_RXEN | FPL011_OPTION_TXEN | FPL011_OPTION_FIFOEN );
    return;
}
/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
void OutByte(s8 byte)
{
    FPl011BlockSend(&early_uart, &byte, 1);
}

char GetByte(void)
{
    return (char)(FPl011BlockReceive(&early_uart));
}