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
 * FilePath: ftrace.h
 * Date: 2022-06-09 16:35:50
 * LastEditTime: 2022-06-09 16:35:51
 * Description:  This file is for trace macro definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/6/13    first release
 */
#ifndef FTRACE_H
#define FTRACE_H

/***************************** Include Files *********************************/
#if !defined(__ASSEMBLER__)
#include "ftypes.h"
#endif

#include "fparameters.h"

#ifdef __cplusplus
extern "C"
{
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FTRACE_UART_BASE    FUART1_BASE_ADDR /* UART-1 as trace Uart */
#define FTRACE_UART_UARTDR (FTRACE_UART_BASE + 0x0U)  /* UART data register offset */
#define FTRACE_UART_UARTFR (FTRACE_UART_BASE + 0x18U) /* UART status register offset */

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif