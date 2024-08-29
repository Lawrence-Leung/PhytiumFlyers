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
 * FilePath: vector.S
 * Date: 2022-02-10 14:53:41
 * LastEditTime: 2022-02-17 17:28:06
 * Description:  This file is for the initial vector table for the processor
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe     2021/7/3     first release
 */




.globl _boot
.globl _vector_table
.weak _vector_table
.globl FiqInterruptHandler
.globl IrqInterruptHandler
.globl SwInterruptHandler
.globl DataAbortInterruptHandler
.globl PrefetchAbortInterruptHandler
.globl UndefineInterruptHandler


.set EXCEPTION_FRAME_SIZE, (17*4)
.set EXCEPTION_PC_OFFSET, (15*4)
.set EXCEPTION_CPSR_OFFSET, (16*4)
.set EXCEPTION_SP_OFFSET, (13*4)
.set EXCEPTION_PC_OFFSET, (14*4)

/* allocatable and executable */
.section .vectors, "a"

_vector_table:
    B	_boot
	B	Undefined
	B	SVCHandler
	B	PrefetchAbortHandler
	B	DataAbortHandler
	NOP	/* Placeholder for address exception vector*/
	B	IrqHandler
	B	FiqHandler

IrqHandler: /* 0x18 IRQ */
    stmdb	sp!,{r0-r3,r12,lr}		/* state save from compiled code*/
    vpush {d0-d7}
	vpush {d16-d31}
	vmrs r1, FPSCR
	push {r1}
	vmrs r1, FPEXC
	push {r1}

    /* Read value from the interrupt acknowledge register, which is stored in r0
	for future parameter and interrupt clearing use. */
    mrc p15, 0, r0, c12, c12, 0
    isb

    /* save r0 which is interrupt num, lr pushed to maintain alignment */
    PUSH	{r0, lr}

    bl	FExceptionInterruptHandler	/* IRQ vector */    
    
    /* get r0 value which is interrupt num */
    POP	    {r0, lr}

    /* Write the value read from ICCIAR to ICCEOIR, end interrupt.*/
	mcr p15, 0, r0, c12, c12, 1
	isb

	pop 	{r1}
	vmsr    FPEXC, r1
	pop 	{r1}
	vmsr    FPSCR, r1
	vpop    {d16-d31}
	vpop    {d0-d7}

	ldmia	sp!,{r0-r3,r12,lr}		/* state restore from compiled code */

	subs	pc, lr, #4			/* adjust return */




.macro PushExceptionReg
    sub     sp, sp, #EXCEPTION_FRAME_SIZE         /* Sizeof(ExceptionFrame)  */
    stmia   sp, {r0 - r12}          /* Calling r0-r12                  */
    mov     r0, sp
    mrs     r6, spsr                /* Save CPSR                       */
    str     lr, [r0, #15*4]         /* Push PC                         */
    str     r6, [r0, #16*4]         /* Push CPSR                       */
    cps     #0x1F                   /* C运行环境所运行的模式             */
    str     sp, [r0, #13*4]         /* Save calling SP                 */
    str     lr, [r0, #14*4]         /* Save calling PC                 */
.endm


Undefined:  /* Undefined handler */
    PushExceptionReg
    cps #0x1B
    bl UndefineInterruptHandler
    bl .

SVCHandler:
    PushExceptionReg
    bl SwInterruptHandler
    bl .

PrefetchAbortHandler:
    PushExceptionReg
    bl PrefetchAbortInterruptHandler
    bl .

DataAbortHandler:
    PushExceptionReg
    bl DataAbortInterruptHandler
    bl .

FiqHandler:					/* FIQ vector handler */
    PushExceptionReg
    bl FiqInterruptHandler
    bl .

.end