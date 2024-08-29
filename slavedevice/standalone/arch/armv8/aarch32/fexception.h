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
 * FilePath: fexception.h
 * Date: 2022-02-10 14:53:41
 * LastEditTime: 2022-02-17 17:29:56
 * Description:  This file contains low-level driver functions for the processor exception
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe      2021/7/3    first release
 * 1.1   zhugengyu	  2022/6/5	  add debugging information	
 * 1.2   wangxiaodong 2023/2/23	  add nested interrupt enable and disable	
 */


#ifndef ARCH_ARMV8_AARCH32_EXCEPTION_H
#define ARCH_ARMV8_AARCH32_EXCEPTION_H

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "ftypes.h"
#include "fassert.h"

/************************** Constant Definitions *****************************/
#define  FEXC_FRAME_SIZE  68U /* sizeof(FExcFrame) */


/****************************************************************************/
/**
* @brief	Enable nested interrupts by clearing the I and F bits in CPSR. This
* 			API is defined for cortex-a.
*
* @return   None.
*
* @note     This macro is supposed to be used from interrupt handlers. In the
*			interrupt handler the interrupts are disabled by default (I and F
*			are 1). To allow nesting of interrupts, this macro should be
*			used. It clears the I and F bits by changing the ARM mode to
*			system mode. Once these bits are cleared and provided the
*			preemption of interrupt conditions are met in the GIC, nesting of
*			interrupts will start happening.
*			Caution: This macro must be used with caution. Before calling this
*			macro, the user must ensure that the source of the current IRQ
*			is appropriately cleared. Otherwise, as soon as we clear the I and
*			F bits, there can be an infinite loop of interrupts with an
*			eventual crash (all the stack space getting consumed).
******************************************************************************/
#define INTERRUPT_NESTED_ENABLE() \
                __asm__ __volatile__ ("stmfd   sp!, {lr}"); \
                __asm__ __volatile__ ("mrs     lr, spsr");  \
                __asm__ __volatile__ ("stmfd   sp!, {lr}"); \
                __asm__ __volatile__ ("msr     cpsr_c, #0x13"); \
                __asm__ __volatile__ ("stmfd   sp!, {lr}");

/****************************************************************************/
/**
* @brief	Disable the nested interrupts by setting the I and F bits. This API
*			is defined for cortex-a.
*
* @return   None.
*
* @note     This macro is meant to be called in the interrupt service routines.
*			This macro cannot be used independently. It can only be used when
*			nesting of interrupts have been enabled by using the macro
*			INTERRUPT_NESTED_ENABLE(). In a typical flow, the user first
*			calls the INTERRUPT_NESTED_ENABLE in the ISR at the appropriate
*			point. The user then must call this macro before exiting the interrupt
*			service routine. This macro puts the ARM back in IRQ/FIQ mode and
*			hence sets back the I and F bits.
******************************************************************************/
#define INTERRUPT_NESTED_DISABLE() \
                __asm__ __volatile__ ("ldmfd   sp!, {lr}");   \
                __asm__ __volatile__ ("msr     cpsr_c, #0x92"); \
                __asm__ __volatile__ ("ldmfd   sp!, {lr}"); \
                __asm__ __volatile__ ("msr     spsr_cxsf, lr"); \
                __asm__ __volatile__ ("ldmfd   sp!, {lr}"); \

/**************************** Type Definitions *******************************/
typedef struct
{
    u32 r0;
    u32 r1;
    u32 r2;
    u32 r3;
    u32 r4;
    u32 r5;
    u32 r6;
    u32 r7;
    u32 r8;
    u32 r9;
    u32 r10;
    u32 r11;
    u32 r12;
    u32 sp;
    u32 lr;
    u32 pc;
    u32 cpsr;
} FExcFrame;

FASSERT_STATIC(sizeof(FExcFrame) == FEXC_FRAME_SIZE);

typedef void (*FExcInterruptEndHandler)(void);
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
void FExcRegisterDataAbortEndHandler(FExcInterruptEndHandler handler);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
void FExceptionInterruptHandler(void *temp);

#ifdef __cplusplus
}
#endif

#endif