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
 * FilePath: trace_uart.S
 * Date: 2022-06-10 14:49:22
 * LastEditTime: 2022-06-13 16:13:32
 * Description:  This file is for aarch32 trace uart function implmentation
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe   2022/6/13    first release
 */

#include "ftrace.h"
#include "sdkconfig.h"

#ifdef CONFIG_BOOTUP_DEBUG_PRINTS

.text

/* define strings use in trace */
bootup_str:
	.asciz  "[1] Bootup...\r\n"

c_main_str:
	.asciz "[2] Enter C Main Function...\r\n"

lr_str:
	.asciz "\tLR:\t\t"

cpu_id_str:
	.asciz "\tCORE-ID:\t"

cpsr_str:
	.asciz "\tCPSR:\t\t"

spsr_str:
	.asciz "\tSPSR:\t\t"

cpsr_pe_mode_str:
	.asciz "\t PE MODE:\t"

cpsr_irq_str:
	.asciz "\t IRQ:\t\t"

cpsr_fiq_str:
	.asciz "\t FIQ:\t\t"

cpsr_lend_str:
	.asciz "\t Little-End:\t"

sp_str:
	.asciz "\tSP:\t\t"

sctlr_str:
	.asciz "\tSCTLR:\t\t"

sctrl_mmu_en_str:
	.asciz "\t MMU:\t\t"

sctrl_cache_en_str:
	.asciz "\t Cache:\t\t"

vbar_str:
	.asciz "\tVBAR:\t\t"

end_str:
	.asciz "***********************\r\n"

.align 2
/**
 * @name: FTraceUartInit
 * @msg: init uart for trace
 * @return: {NONE}
 * @param: NONE
 */
.globl FTraceUartInit
FTraceUartInit:
    /* Reserved for trace uart initialization */
    bx lr

/**
 * @name: FTracePutUart
 * @msg: print one byte by trace UART
 * @return: {NONE}
 * @param: r0, value to print
 * @note: r9 as register to save lr  
 */
FTracePutUart:
    /* save lr register */
    mov r9, lr

	ldr r1, =FTRACE_UART_UARTFR
1:
	ldr r2, [r1]
	and r2, r2, #0x20
	cmp r2, #0x0
	bne 1b

	ldr r1, =FTRACE_UART_UARTDR
	str r0, [r1]

	/* restore lr and return*/
	mov lr, r9
    bx lr

/**
 * @name: FTracePutString
 * @msg: print string end with '\0'
 * @param: r0, start address of string
 * @note: r8 as register to save lr
 */
FTracePutString:
	mov r4, r0 /* backup string label to print */
	/* save lr register */
	mov r8, lr

1:
	ldrb r0, [r4]
	bl FTracePutUart
	add r4, r4, #1
	cmp r0, #0 /* check if finished */
	bne 1b

	/* restore lr and return*/
	mov lr, r8
	bx lr

/**
 * @name: FTraceHexToAscii
 * @msg: convert hex bytes as ASCII
 * @param: r0, value to convert
 */
.macro FTraceHexToAscii
	cmp  r0,  #0x0
	blt  2f /* exit if < NUL */
	cmp  r0,  #0x9
	bhi  1f /* if > '9' */
	add  r0,  r0,  #0x30 /* 0x0 + 0x30 = 0x30 = '0' */
	b    2f /* if 0 ~ 9 */
1:
	cmp  r0,  #0xF /* character A ~ F */
	bhi  2f /*exit if > 'F' */
	add  r0,  r0,  #0x37 /* 0xA + 0x37 = 0x41 = 'A' */
2:
.endm

/**
 * @name: FTracePutHex
 * @msg: print a number in hex, like '0xabcd'
 * @param: r0, value to print
 * @note: r8, register to save lr 
 */
.globl FTracePutHex
FTracePutHex:
    /* save lr register */
    mov r8, lr

	mov r4,  r0 /* backup hex value to print in r4 */

	/* print 0x in uart */
	mov  r0,  #'0'
	bl FTracePutUart
	mov  r0,  #'x'
	bl FTracePutUart

	/* handle 32-bits value in loop */
	mov  r3,  #32 /* r3 = 32 */
loop_32_bits:
	sub  r3,  r3,  #4 /* print 4-bit value 0 ~ f */
	asr  r0,  r4,  r3 /* r4 hold to value to print */
	and  r0,  r0,  #0xF
	FTraceHexToAscii /* convert value to ascii code and print in uart */
	bl FTracePutUart
	cmp  r3,  #0 /* check if 32-bits loop over */
	bne loop_32_bits

	/* print \r\n to end the line */
	mov r0,   #'\r'
	bl FTracePutUart

	mov r0,   #'\n'
	bl FTracePutUart

	/* restore lr and return*/
	mov lr, r8
    bx lr

/**
 * @name: FTracePutBool
 * @msg: print a boolean value, like 'ON'/'OFF'
 * @param: r0, value to print
 * @note: r8, register to save lr 
 */
.globl FTracePutBool
FTracePutBool:
    /* save lr register */
    mov r8, lr
	mov r4, r0 /* backup hex value to print in r4 */

	cmp r0, #0
	bne .bool_on

	/* bool_off */
	mov r0,   #'O'
	bl FTracePutUart
	mov r0,   #'F'
	bl FTracePutUart
	mov r0,   #'F'
	bl FTracePutUart	

	b   .bool_exit

.bool_on:
	mov r0,   #'O'
	bl FTracePutUart
	mov r0,   #'N'
	bl FTracePutUart

.bool_exit:
	/* print \r\n to end the line */
	mov r0,   #'\r'
	bl FTracePutUart

	mov r0,   #'\n'
	bl FTracePutUart

	/* restore lr and return*/
	mov lr, r8
    bx lr

/**
 * @name: FTracePutBinary
 * @msg: print a binary value, like '0b0010'
 * @param: r0, value to print
 * @note: r8, register to save lr
 */
.globl FTracePutBinary
FTracePutBinary:
    /* save lr register */
    mov r8, lr
	mov r4, r0 /* backup hex value to print in r4 */

	/* print 0b in uart */
	mov  r0,  #'0'
	bl FTracePutUart
	mov  r0,  #'b'
	bl FTracePutUart	

	/* handle 4-bits value in loop */
	mov  r3,  #4 /* r3 = 4 */
loop_4_bits:
	sub  r3,  r3,  #1 /* print 4-bit value 0 ~ 9 */
	asr  r0,  r4,  r3 /* r4 hold to value to print */
	and  r0,  r0,  #0x1
	FTraceHexToAscii /* convert value to ascii code and print in uart */
	bl FTracePutUart
	cmp  r3,  #0 /* check if 4-bits loop over */
	bne loop_4_bits	

	/* print \r\n to end the line */
	mov r0,   #'\r'
	bl FTracePutUart

	mov r0,   #'\n'
	bl FTracePutUart

	/* restore lr and return*/
	mov lr, r8
    bx lr

.macro FTraceLabel, str_label
    ldr  r0, =\str_label
    bl   FTracePutString
.endm FTraceLabel

.macro FTraceCPUID
	FTraceLabel cpu_id_str
	mrc	 p15, 0, r0, c0, c0, 5	/* read MPIDR */
	bl   FTracePutHex
.endm

.macro FTraceLR
	FTraceLabel lr_str
	mov  r0, lr
	bl   FTracePutHex
.endm

.macro FTraceSPSR
	FTraceLabel spsr_str
	mrs  r0, spsr
	bl   FTracePutHex

	/* print current irq mode */
	FTraceLabel cpsr_irq_str
	mrs r0, spsr
	and r0, r0, #(1 << 7) /* bit[7] irq mask bit */
	eor r0, r0, #(1 << 7) /* 0 = irq enable */
	bl FTracePutBool

	/* print current fiq mode */
	FTraceLabel cpsr_fiq_str
	mrs r0, spsr
	and r0, r0, #(1 << 6) /* bit[6] fiq mask bit */
	eor r0, r0, #(1 << 6) /* 0 = fiq enable */
	bl FTracePutBool

	/* print if little-end */
	FTraceLabel cpsr_lend_str
	mrs r0, spsr
	and r0, r0, #(1 << 9)
	eor r0, r0, #(1 << 9) /* convert 0 -> 1 because bit[9] = 0 means little-end */
	bl FTracePutBool
.endm

.macro FTraceCPSR

	FTraceLabel cpsr_str
	mrs  r0, cpsr
	bl   FTracePutHex

	/* print current PE mode 
		0b000 = User, 0b0011 = Supervisor, 0b1111 = System
	*/
	FTraceLabel cpsr_pe_mode_str
	mrs  r0, cpsr
	and  r0, r0, #0xf /* bit[3:0] current PE mode */
	bl   FTracePutBinary

	/* print current irq mode */
	FTraceLabel cpsr_irq_str
	mrs r0, cpsr
	and r0, r0, #(1 << 7) /* bit[7] irq mask bit */
	eor r0, r0, #(1 << 7) /* 0 = irq enable */
	bl FTracePutBool

	/* print current fiq mode */
	FTraceLabel cpsr_fiq_str
	mrs r0, cpsr
	and r0, r0, #(1 << 6) /* bit[6] fiq mask bit */
	eor r0, r0, #(1 << 6) /* 0 = fiq enable */
	bl FTracePutBool

	/* print if little-end */
	FTraceLabel cpsr_lend_str
	mrs r0, cpsr
	and r0, r0, #(1 << 9)
	eor r0, r0, #(1 << 9) /* convert 0 -> 1 because bit[9] = 0 means little-end */
	bl FTracePutBool
.endm

.macro FTraceSP
	FTraceLabel sp_str
	mov r0, sp
	bl  FTracePutHex
.endm

/* C1 */
.macro FTraceSctrl
	FTraceLabel sctlr_str
	mrc	p15, 0, r0, c1, c0, 0
	bl  FTracePutHex

	FTraceLabel sctrl_mmu_en_str
	mrc	p15, 0, r0, c1, c0, 0
	and r0, r0, #(1 << 0) /* bit[0] 1 = enable MMU */
	bl FTracePutBool

	FTraceLabel sctrl_cache_en_str
	mrc	p15, 0, r0, c1, c0, 0
	and r0, r0, #(1 << 2) /* bit[2] 1 = enable cache / data cache */ 
	bl FTracePutBool

.endm

.macro FTraceVBAR
	FTraceLabel vbar_str
	mrc	p15, 0, r0, c12, c0, 0
	bl  FTracePutHex
.endm


/**
 * @name: FTraceBootup
 * @msg: print trace information when system bootup
 * @note: r7, save lr register
 */
.globl FTraceBootup
FTraceBootup:
	/* save lr register */
	mov  r7, lr

    FTraceLabel bootup_str

	FTraceCPUID

	FTraceCPSR

	mrs  r0, cpsr
	and  r0, r0, #0xf /* bit[3:0] current PE mode */

	/* cannot access spsr in User/System mode */
	cmp  r0, #0b0000 /* user mode */
	beq  .no_spsr
	cmp  r0, #0b1111 /* system mode */
	beq  .no_spsr

	FTraceSPSR
.no_spsr:

	FTraceSctrl

	FTraceVBAR

	FTraceSP

	FTraceLabel end_str

	/* restore lr and return*/
	mov  lr, r7
    bx   lr

/**
 * @name: FTraceCEntry
 * @msg: print trace information when C environment setup done
 * @note: r7, save lr register
 */
.globl FTraceCEntry
FTraceCEntry:
	/* save lr register */
	mov  r7, lr

	FTraceLabel c_main_str

	FTraceCPUID

	FTraceCPSR

	/* cannot access spsr in User/System mode */
	mrs  r0, cpsr
	and  r0, r0, #0xf /* bit[3:0] current PE mode */

	cmp  r0, #0b0000 /* user mode */
	beq  .no_spsr_
	cmp  r0, #0b1111 /* system mode */
	beq  .no_spsr_

	FTraceSPSR
.no_spsr_:

	FTraceSctrl

	FTraceVBAR

	FTraceSP

	FTraceLabel end_str

	/* restore lr and return*/
	mov  lr, r7
    bx   lr

#else

.globl FTraceUartInit
FTraceUartInit:
	bx	lr

.globl FTraceBootup
FTraceBootup:
	bx	lr

.globl FTraceCEntry
FTraceCEntry:
	bx	lr

#endif