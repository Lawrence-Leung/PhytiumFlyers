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
 * FilePath: ftrace_uart.S
 * Date: 2022-06-10 14:49:22
 * LastEditTime: 2022-06-13 16:13:32
 * Description:  This file is for aarch64 trace uart function implmentation
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/6/13    first release
 * 1.1   zhugengyu  2023/2/21    add trace item sctrl_el2
 */

#include "ftrace.h"
#include "sdkconfig.h"

#ifdef CONFIG_BOOTUP_DEBUG_PRINTS

.section ".text.debug"

/* define strings use in trace */
bootup_str:
	.string "[1] Bootup...\r\n"

el1_str:
	.string "[4] Enter EL1...\r\n"

el2_str:
	.string "[3] Enter EL2...\r\n"

el3_str:
	.string "[2] Enter EL3...\r\n"

c_main_str:
	.string "[5] Enter C Main Function...\r\n"

mpidr_el1_str:
	.string "\tMPIDR_EL1:\t"

cur_el_str:
	.string "\tCurrentEL:\t"

sctlr_el1_str:
	.string "\tSCTLR_EL1:\t"

sctlr_mmu_str:
	.string "\t MMU:\t\t"

sctlr_cache_str:
	.string "\t Cache:\t\t"

sctrl_lend_el1_str:
	.string "\t Little-End(EL1):"

sctrl_lend_el0_str:
	.string "\t Little-End(EL0):"

cpacr_el1_str:
	.string "\tCPACR_EL1:\t"

daif_str:
	.string "\tDAIF:\t\t"

tcr_el1_str:
	.string "\tTCR_EL1:\t"

ttbr0_el1_str:
	.string "\tTTBR0_EL1:\t"

ttbr1_el1_str:
	.string "\tTTBR1_EL1:\t"

vbar_el1_str:
	.string "\tVBAR_EL1:\t"

hcr_el2_str:
	.string "\tHCR_EL2:\t"

sctlr_el2_str:
	.string "\tSCTLR_EL2:\t"

xzr_str:
	.string "\tXZR:\t\t"

spsr_el1_str:
	.string "\tSPSR_EL1:\t"

sp_el1_str:
	.string "\tSP_EL1:\t\t"

end_str:
	.string "***********************\r\n"

.align 3 /* align with 8 bytes */

/**
 * @name: FTraceUartInit
 * @msg: init uart for trace
 * @return: {NONE}
 * @param: NONE
 */
.globl FTraceUartInit
FTraceUartInit:
    /* Reserved for trace uart initialization */
    ret

/**
 * @name: FTracePutUart
 * @msg: print one byte by trace UART
 * @return: {NONE}
 * @param: x0, value to print
 * @note: x21 as register to save lr  
 */
FTracePutUart:
	/* save lr register */
	mov x21, x30

	ldr x1, =FTRACE_UART_UARTFR
1:
	ldr w2, [x1]
	and w2, w2, #0x20
	cmp w2, #0x0
	b.ne 1b

	ldr x1, =FTRACE_UART_UARTDR
	str w0, [x1]
	/* restore lr and return*/
	mov x30, x21
	ret

/**
 * @name: FTracePutString
 * @msg: print string end with '\0'
 * @param: x0, start address of string
 * @note: x20 as register to save lr
 */
FTracePutString:
	mov x4, x0
	/* save lr register */
	mov x20, x30
1:
	ldrb w0, [x4]
	bl FTracePutUart
	add x4, x4, 1
	cmp w0, #0
	bne 1b

	/* restore lr and return*/
	mov x30, x20
	ret

/**
 * @name: FTraceHexToAscii
 * @msg: convert hex bytes as ASCII
 * @param: x0, value to convert
 */
.macro FTraceHexToAscii
	cmp  x0,  #0x0
	b.lt 2f /* exit if < NUL */
	cmp  x0,  #0x9 
	b.hi 1f /* if > '9' */
	add  x0,  x0,  #0x30 /* 0x0 + 0x30 = 0x30 = '0' */
	b    2f /* if 0 ~ 9 */
1:
	cmp  x0,  #0xF /* character A ~ F */
	b.hi 2f /*exit if > 'F' */
	add  x0,  x0,  #0x37 /* 0xA + 0x37 = 0x41 = 'A' */
2:
.endm

/**
 * @name: FTracePutHex
 * @msg: print a number in hex (64-bit), like '0xabcd'
 * @param: x0, value to print
 * @note: x20, register to save lr 
 */
FTracePutHex:
	/* save lr register */
	mov x20, x30

	mov  x4,  x0 /* backup hex value to print in x4 */

	/* print 0x in uart */
	mov  w0,  #'0'
	bl FTracePutUart
	mov  w0,  #'x'
	bl FTracePutUart	

	/* handle 64/32-bits value in loop */
	mov  x3,  #64 /* x3 = 64/32 */
loop_64_bits:
	sub  x3,  x3,  #4 /* print 4-bit value 0 ~ f */
	asr  x0,  x4,  x3 /* x4 hold to value to print */
	and  x0,  x0,  #0xF
	FTraceHexToAscii /* convert value to ascii code and print in uart */
	bl FTracePutUart
	cmp  x3,  #0 /* check if 64-bits loop over */
	b.ne loop_64_bits

	/* print \r\n to end the line */
	mov x0,   #'\r'
	bl FTracePutUart

	mov x0,   #'\n'
	bl FTracePutUart

	/* restore lr and return*/
	mov x30, x20
	ret	

/**
 * @name: FTracePutBool
 * @msg: print a boolean value (1-bit), like 'ON'/'OFF'
 * @param: x0, value to print
 * @note: x20, register to save lr 
 */
.globl FTracePutBool
FTracePutBool:
	/* save lr register */
	mov x20, x30
	mov  x4,  x0 /* backup hex value to print in x4 */

	cmp x0, #0
	bne .bool_on

	/* bool_off */
	mov x0,   #'O'
	bl FTracePutUart
	mov x0,   #'F'
	bl FTracePutUart
	mov x0,   #'F'
	bl FTracePutUart	

	b   .bool_exit

.bool_on:
	mov x0,   #'O'
	bl FTracePutUart
	mov x0,   #'N'
	bl FTracePutUart

.bool_exit:
	/* print \r\n to end the line */
	mov x0,   #'\r'
	bl FTracePutUart

	mov x0,   #'\n'
	bl FTracePutUart	

	/* restore lr and return*/
	mov x30, x20
	ret	

/**
 * @name: FTracePutBinary
 * @msg: print a binary value (4-bit), like '0b0010'
 * @param: x0, value to print
 * @note: x20, register to save lr
 */
.globl FTracePutBinary
FTracePutBinary:
	/* save lr register */
	mov x20, x30
	mov  x4,  x0 /* backup hex value to print in x4 */

	/* print 0b in uart */
	mov  x0,  #'0'
	bl FTracePutUart
	mov  x0,  #'b'
	bl FTracePutUart

	/* handle 64/32-bits value in loop */
	mov  x3,  #4 /* x3 = 64/32 */
loop_4_bits:
	sub  x3,  x3,  #1 /* print 4-bit value 0 ~ f */
	asr  x0,  x4,  x3 /* x4 hold to value to print */
	and  x0,  x0,  #0x1
	FTraceHexToAscii /* convert value to ascii code and print in uart */
	bl FTracePutUart
	cmp  x3,  #0 /* check if 64-bits loop over */
	b.ne loop_4_bits

	/* print \r\n to end the line */
	mov x0,   #'\r'
	bl FTracePutUart

	mov x0,   #'\n'
	bl FTracePutUart

	/* restore lr and return*/
	mov x30, x20
	ret	

/* use x0 */
.macro FTraceLabel, str_label
	adrp x0, \str_label
	add  x0, x0, :lo12:\str_label
	bl   FTracePutString
.endm FTraceLabel

.macro FTraceEL
	/* printf current exception level */
	FTraceLabel cur_el_str
	mrs  x5, CurrentEL
	lsr  x0, x5, #2 /* x2 = x5 bit[3:2] */
	and  x0, x0, #0b11
	bl   FTracePutBinary
.endm

.macro FTraceXZR
	/* print xzr to see if it is zero */
	FTraceLabel xzr_str
	mov  x0, XZR
	bl FTracePutHex
.endm

.macro FTraceSCTLR_EL2
	FTraceLabel sctlr_el2_str
    mrs  x0, SCTLR_EL2
	bl FTracePutHex
.endm

.macro FTraceHCR_EL2
	FTraceLabel hcr_el2_str
    mrs  x0, HCR_EL2
	bl FTracePutHex
.endm

.macro FTraceMPIDR_EL1
	FTraceLabel mpidr_el1_str
    mrs  x0, MPIDR_EL1
	bl FTracePutHex
.endm

.macro FTraceSCTLR_EL1
	FTraceLabel sctlr_el1_str
    mrs  x0, SCTLR_EL1
	bl FTracePutHex

	FTraceLabel sctlr_mmu_str
	mrs  x0, SCTLR_EL1
	and  x0, x0, #(1 << 0) /* bit[0] 1 = MMU enable */
	bl   FTracePutBool

	FTraceLabel sctlr_cache_str
	mrs  x0, SCTLR_EL1
	and  x0, x0, #(1 << 2) /* bit[2] 1 = Cache enable */
	bl   FTracePutBool

	FTraceLabel sctrl_lend_el1_str
	mrs  x0, SCTLR_EL1
	and  x0, x0, #(1 << 25) /* bit[25] 0 = EL1 little-end */
	eor  x0, x0, #(1 << 25)
	bl   FTracePutBool

	FTraceLabel sctrl_lend_el0_str
	mrs  x0, SCTLR_EL1
	and  x0, x0, #(1 << 24) /* bit[24] 0 = EL0 little-end */
	eor  x0, x0, #(1 << 24)
	bl   FTracePutBool
.endm

.macro FTraceCAACR_EL1
	FTraceLabel cpacr_el1_str
    mrs  x0, CPACR_EL1
	bl FTracePutHex
.endm

.macro FTraceDAIF
	FTraceLabel daif_str
    mrs  x0, DAIF
	lsr  x0, x0, #6 /* bit[9:6] DAIF */
	and  x0, x0, 0xf
	bl FTracePutBinary
.endm

.macro FTraceTCR_EL1
	FTraceLabel tcr_el1_str
    mrs  x0, TCR_EL1
	bl FTracePutHex
.endm

.macro FTraceTTBR0_EL1
	FTraceLabel ttbr0_el1_str
    mrs  x0, TTBR0_EL1
	bl FTracePutHex
.endm

.macro FTraceTTBR1_EL1
	FTraceLabel ttbr1_el1_str
    mrs  x0, TTBR1_EL1
	bl FTracePutHex
.endm

.macro FTraceVBAR_EL1
	FTraceLabel vbar_el1_str
    mrs  x0, VBAR_EL1
	bl FTracePutHex
.endm

.macro FTraceSP_EL1
	FTraceLabel sp_el1_str
	mov x0, sp
	bl FTracePutHex
.endm

/**
 * @name: FTraceBootup
 * @msg: print trace information when system bootup
 * @note: x28, save lr register
 */
.globl FTraceBootup
FTraceBootup:
	/* save lr register */
	mov  x28, x30

	FTraceLabel bootup_str

	FTraceEL

	FTraceLabel end_str

	/* restore lr and return*/
	mov  x30, x28
	ret	

/**
 * @name: FTraceEL3
 * @msg: print trace information when system in EL3
 * @note: x28, save lr register
 */
.globl FTraceEL3
FTraceEL3:
	/* save lr register */
	mov  x28, x30

	FTraceLabel bootup_str

	FTraceEL	

	FTraceXZR
	
	FTraceLabel end_str

	/* restore lr and return*/
	mov  x30, x28
	ret

/**
 * @name: FTraceEL2
 * @msg: print trace information when system in EL2
 * @note: x28, save lr register
 */
.globl FTraceEL2
FTraceEL2:
	/* save lr register */
	mov  x28, x30

	FTraceLabel el2_str

	/* printf current exception level */
	FTraceEL

	/* print HCR_EL2 */
    FTraceHCR_EL2

	FTraceSCTLR_EL2

	FTraceLabel end_str

	/* restore lr and return*/
	mov  x30, x28
	ret	

/**
 * @name: FTraceEL1
 * @msg: print trace information when system in EL1
 * @note: x28, save lr register
 */
.globl FTraceEL1
FTraceEL1:
	/* save lr register */
	mov  x28, x30

	FTraceLabel el1_str

	/* printf current exception level */
	FTraceEL 

	/* print sp */
	FTraceSP_EL1

	/* print MPIDR_EL1 */
	FTraceMPIDR_EL1

	/* print SCTLR_EL1 */
	FTraceSCTLR_EL1

	/* print CPACR_EL1 */
	FTraceCAACR_EL1

	/* print DAIF */
	FTraceDAIF	

	/* print TCR_EL1 */
	FTraceTCR_EL1	

	/* print TTBR0_EL1 */
	FTraceTTBR0_EL1

	/* print TTBR1_EL1 */
	FTraceTTBR1_EL1

	/* print VBAR_EL1 */
	FTraceVBAR_EL1

	FTraceLabel end_str

	/* restore lr and return*/
	mov  x30, x28
	ret	

/**
 * @name: FTraceCEntry
 * @msg: print trace information when C environment setup done
 * @note: x28 save lr register
 */
.global FTraceCEntry
FTraceCEntry:
	/* save lr register */
	mov  x28, x30

	FTraceLabel c_main_str

	/* printf current exception level */
	FTraceEL 

	/* print sp */
	FTraceSP_EL1

	/* print MPIDR_EL1 */
	FTraceMPIDR_EL1

	/* print SCTLR_EL1 */
	FTraceSCTLR_EL1

	/* print CPACR_EL1 */
	FTraceCAACR_EL1

	/* print DAIF */
	FTraceDAIF	

	/* print TCR_EL1 */
	FTraceTCR_EL1	

	/* print TTBR0_EL1 */
	FTraceTTBR0_EL1

	/* print TTBR1_EL1 */
	FTraceTTBR1_EL1

	/* print VBAR_EL1 */
	FTraceVBAR_EL1

	FTraceLabel end_str

	/* restore lr and return*/
	mov  x30, x28
	ret	

#else

.globl FTraceUartInit
FTraceUartInit:
	ret

.globl FTraceBootup
FTraceBootup:
	ret

.globl FTraceEL3
FTraceEL3:
	ret

.globl FTraceEL2
FTraceEL2:
	ret

.globl FTraceEL1
FTraceEL1:
	ret

.globl FTraceCEntry
FTraceCEntry:
	ret	

#endif