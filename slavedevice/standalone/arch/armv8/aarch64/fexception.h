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
 * LastEditTime: 2022-02-17 17:32:53
 * Description:  This file contains low-level driver functions for the processor exception
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2021/7/3     first release
 * 1.1  zhugengyu	2022/6/3		add debugging information	
 * 1.2   wangxiaodong 2023/2/23	  add nested interrupt enable and disable	

 */
#ifndef FEXCEPTION_H
#define FEXCEPTION_H

/***************************** Include Files *********************************/
#include "ftypes.h"
#include "fassert.h"

#ifdef __cplusplus
extern "C"
{
#endif

/************************** Constant Definitions *****************************/
#define FEXC_FRAME_SIZE         208U

/****************************************************************************/
/**
* @brief	Enable nested interrupts by clearing the I bit in DAIF.This
*			macro is defined for Cortex-A 64 bit mode and Cortex-A 64 bit
*			BSP configured to run at EL1 NON SECURE
*
* @return   None.
*
* @note     This macro is supposed to be used from interrupt handlers. In the
*			interrupt handler the interrupts are disabled by default (I bit
*			is set as 1). To allow nesting of interrupts, this macro should be
*			used. It clears the I bit. Once that bit is cleared and provided the
*			preemption of interrupt conditions are met in the GIC, nesting of
*			interrupts will start happening.
*			Caution: This macro must be used with caution. Before calling this
*			macro, the user must ensure that the source of the current IRQ
*			is appropriately cleared. Otherwise, as soon as we clear the I
*			bit, there can be an infinite loop of interrupts with an
*			eventual crash (all the stack space getting consumed).
******************************************************************************/
#define INTERRUPT_NESTED_ENABLE() \
                __asm__ __volatile__ ("mrs    X1, ELR_EL1"); \
                __asm__ __volatile__ ("mrs    X2, SPSR_EL1");  \
                __asm__ __volatile__ ("stp    X1,X2, [sp,#-0x10]!"); \
                __asm__ __volatile__ ("mrs    X1, DAIF");  \
                __asm__ __volatile__ ("bic    X1,X1,#(0x1<<7)");  \
                __asm__ __volatile__ ("msr    DAIF, X1");  \

/****************************************************************************/
/**
* @brief	Disable the nested interrupts by setting the I bit in DAIF. This
*			macro is defined for Cortex-A 64 bit mode and Cortex-A 64 bit
*			BSP configured to run at EL1 NON SECURE
*
* @return   None.
*
* @note     This macro is meant to be called in the interrupt service routines.
*			This macro cannot be used independently. It can only be used when
*			nesting of interrupts have been enabled by using the macro
*			INTERRUPT_NESTED_ENABLE(). In a typical flow, the user first
*			calls the INTERRUPT_NESTED_ENABLE in the ISR at the appropriate
*			point. The user then must call this macro before exiting the interrupt
*			service routine. This macro puts the ARM back in IRQ mode and
*			hence sets back the I bit.
******************************************************************************/
#define INTERRUPT_NESTED_DISABLE() \
                __asm__ __volatile__ ("ldp    X1,X2, [sp,#0x10]!"); \
                __asm__ __volatile__ ("msr    ELR_EL1, X1"); \
                __asm__ __volatile__ ("msr    SPSR_EL1, X2"); \
                __asm__ __volatile__ ("mrs    X1, DAIF");  \
                __asm__ __volatile__ ("orr    X1, X1, #(0x1<<7)"); \
                __asm__ __volatile__ ("msr    DAIF, X1");  \

/**************************** Type Definitions *******************************/
typedef struct
{
    u64 sp;
    u64 pstate;
    u64 cpacr;
    u64 elr;
    u64 x29;
    u64 x30;
    u64 x18;
    u64 x19;
    u64 x16;
    u64 x17;
    u64 x14;
    u64 x15;
    u64 x12;
    u64 x13;
    u64 x10;
    u64 x11;
    u64 x8;
    u64 x9;
    u64 x6;
    u64 x7;
    u64 x4;
    u64 x5;
    u64 x2;
    u64 x3;
    u64 x0;
    u64 x1;
} FExcFrame;

FASSERT_STATIC(sizeof(FExcFrame) == FEXC_FRAME_SIZE);

typedef void (*FExcInterruptEndHandler)(void);
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
void FExcRegisterSyncEndHandler(FExcInterruptEndHandler handler);
void FExcRegisterSerrEndHandler(FExcInterruptEndHandler handler);

/************************** Variable Definitions *****************************/
void FExceptionInterruptHandler(void *temp);
/*****************************************************************************/

#ifdef __cplusplus
}
#endif


#endif