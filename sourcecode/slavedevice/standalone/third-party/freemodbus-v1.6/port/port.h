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
 * FilePath: port.h
 * Date: 2022-09-29 18:07:34
 * LastEditTime: 2022-09-29 18:07:34
 * Description:  This file is for modbus variable types and critical section
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  liushengming 2022/09/29    first commit
 */

#ifndef _PORT_H
#define _PORT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "ftypes.h"

#if defined(__aarch64__)

#define ENTER_CRITICAL_SECTION()        \
	__asm volatile("MSR DAIFSET, #2" :: \
					   : "memory");     \
	__asm volatile("DSB SY");           \
	__asm volatile("ISB SY");

#define EXIT_CRITICAL_SECTION()         \
	__asm volatile("MSR DAIFCLR, #2" :: \
					   : "memory");     \
	__asm volatile("DSB SY");           \
	__asm volatile("ISB SY");

#else
#include "faarch32.h"
#define ENTER_CRITICAL_SECTION() \
	do                           \
	{                            \
		u32 state;               \
		state = MFCPSR();        \
		MTCPSR(state | 0xc0);    \
	} while (0);

#define EXIT_CRITICAL_SECTION()   \
	do                            \
	{                             \
		MTCPSR(MFCPSR() & ~0xc0); \
	} while (0);
#endif

	typedef unsigned int BOOL;

	typedef unsigned char UCHAR;
	typedef char CHAR;

	typedef uint16_t USHORT;
	typedef int16_t SHORT;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

	/* ----------------------- Function prototypes ------------------------------*/

#ifdef __cplusplus
}
#endif

#endif