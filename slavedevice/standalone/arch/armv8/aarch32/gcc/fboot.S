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
 * FilePath: boot.S
 * Date: 2022-02-10 14:53:41
 * LastEditTime: 2022-02-17 17:28:26
 * Description:  This file is for 32bit Processor Boot Code
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  huanghe	2021-11		initialization
 *  1.1  zhugengyu	2022/06/05	add debugging information
 *  1.2  zhugengyu  2023/02/21	add hardcoded aarch64 instructions to flush dcache when bootup	
 */


#include "sdkconfig.h"

.global _boot
.global _vector_table
.global __sys_stack
.global __irq_stack
.global __supervisor_stack
.global __abort_stack
.global __fiq_stack
.global __undef_stack

.set vector_base, _vector_table

.set _sys_stack, __sys_stack
.set _irq_stack, __irq_stack
.set _svc_stack, __supervisor_stack
.set _abort_stack, __abort_stack
.set _fiq_stack, __fiq_stack
.set _undef_stack, __undef_stack

.org 0
.text

.section .boot,"ax"

_boot:
Startup_Aarch32:
#ifdef CONFIG_USE_AARCH64_L1_TO_AARCH32
#ifdef CONFIG_BOOT_WITH_FLUSH_CACHE
 	/* hard code aarch64 instruction to flush dcache, refer to rt-thread __asm_flush_dcache_all */
	.long 0xd2800000	/* mov	x0, #0x0 // clean and invaildate d-cache */
	.long 0x1400001a	/* b	<InvalidateFlushDcaches> */

InvalidateFlushDcacheLevel:
	.long 0xd37ff80c	/* lsl	x12, x0, #1 */
	.long 0xd51a000c	/* msr	csselr_el1, x12 */
	.long 0xd5033fdf	/* isb */
	.long 0xd5390006	/* mrs	x6, ccsidr_el1 */
	.long 0x924008c2	/* and	x2, x6, #0x7 */
	.long 0x91001042	/* add	x2, x2, #0x4 */
	.long 0xd2807fe3	/* mov	x3, #0x3ff */
	.long 0x8a460c63	/* and	x3, x3, x6, lsr #3 */
	.long 0x5ac01065	/* clz	w5, w3 */
	.long 0xd28fffe4	/* mov	x4, #0x7fff */
	.long 0x8a463484	/* and	x4, x4, x6, lsr #13 */

InvalidateFlushCacheSet:
	.long 0xaa0303e6	/* mov	x6, x3 */

InvalidateFlushCacheWay:
	.long 0x9ac520c7	/* lsl	x7, x6, x5 */
	.long 0xaa070189	/* orr	x9, x12, x7 */
	.long 0x9ac22087	/* lsl	x7, x4, x2 */
	.long 0xaa070129	/* orr	x9, x9, x7 */
	.long 0x36000061	/* tbz	w1, #0, <InvalidateFlushCacheWay+0x1c> */
	.long 0xd5087649	/* dc	isw, x9 */
	.long 0x14000002	/* b	<InvalidateFlushCacheWay+0x20> */
	.long 0xd5087e49	/* dc	cisw, x9 */
	.long 0xf10004c6	/* subs	x6, x6, #0x1 */
	.long 0x54fffeea	/* b.ge	 <InvalidateFlushCacheWay> */
	.long 0xf1000484	/* subs	x4, x4, #0x1 */
	.long 0x54fffe8a	/* b.ge	 <InvalidateFlushCacheSet> */
	.long 0xd65f03c0	/* ret */

InvalidateFlushDcaches:
	.long 0xaa0003e1	/* mov	x1, x0 */
	.long 0xd5033f9f	/* dsb	sy */
	.long 0xd539002a	/* mrs	x10, clidr_el1 */
	.long 0xd358fd4b	/* lsr	x11, x10, #24 */
	.long 0x9240096b	/* and	x11, x11, #0x7 */
	.long 0xb400024b	/* cbz	x11, <InvalidateFlushDcacheEnd> */
	.long 0xaa1e03ef	/* mov	x15, x30 */
	.long 0xd2800000	/* mov	x0, #0x0 */

InvalidateFlushCachesLoopLevel:
	.long 0xd37ff80c	/* lsl	x12, x0, #1 */
	.long 0x8b00018c	/* add	x12, x12, x0 */
	.long 0x9acc254c	/* lsr	x12, x10, x12 */
	.long 0x9240098c	/* and	x12, x12, #0x7 */
	.long 0xf100099f	/* cmp	x12, #0x2 */
	.long 0x5400004b	/* b.lt	<InvalidateFlushCachesSkipLevel> */
	.long 0x97ffffd9	/* bl	<InvalidateFlushDcacheLevel> */

InvalidateFlushCachesSkipLevel:
	.long 0x91000400	/* add	x0, x0, #0x1 */
	.long 0xeb00017f	/* cmp	x11, x0 */
	.long 0x54fffeec	/* b.gt	<InvalidateFlushCachesLoopLevel> */
	.long 0xd2800000	/* mov	x0, #0x0 */
	.long 0xd51a0000	/* msr	csselr_el1, x0 */
	.long 0xd5033f9f	/* dsb	sy */
	.long 0xd5033fdf	/* isb */
	.long 0xaa0f03fe	/* mov	x30, x15 */

InvalidateFlushDcacheEnd:
#endif

    .long 0xd5384240 	/* mrs	x0, currentel                      */
    .long 0xd342fc00 	/* lsr	x0, x0, #2                         */
    .long 0x92400400 	/* and	x0, x0, #0x3                       */
    .long 0xf1000c1f 	/* cmp	x0, #0x3                           */
    .long 0x540003a1 	/* b.ne	1d0080c4 <el2_mode>                */

el3_mode:
    .long 0xd53ecca0 	/* mrs	x0, s3_6_c12_c12_5 - ICC_SRE_EL3   */
    .long 0xb2400c00 	/* orr	x0, x0, #0xf                       */
    .long 0xd51ecca0 	/* msr	s3_6_c12_c12_5, x0                 */
    .long 0xd5033fdf 	/* isb                                     */
    .long 0xd53cc9a0 	/* mrs	x0, s3_4_c12_c9_5 - ICC_SRE_EL2    */
    .long 0xb2400c00 	/* orr	x0, x0, #0xf                       */
    .long 0xd51cc9a0 	/* msr	s3_4_c12_c9_5, x0                  */
    .long 0xd5033fdf 	/* isb                                     */
    .long 0xd538cca0 	/* mrs	x0, s3_0_c12_c12_5 - ICC_SRE_EL1   */
    .long 0xb2400000 	/* orr	x0, x0, #0x1                       */
    .long 0xd518cca0 	/* msr	s3_0_c12_c12_5, x0                 */
    .long 0xd5033fdf 	/* isb                                     */

    .long 0xd2803620 	/* mov	x0, #0x1b1                         */
    .long 0xd51e1100 	/* msr	scr_el3, x0                        */
    .long 0xd2867fe0 	/* mov	x0, #0x33ff                        */
    .long 0xd51c1140 	/* msr	cptr_el2, x0                       */
    .long 0xd2810000 	/* mov	x0, #0x800                         */
    .long 0xf2a61a00 	/* movk	x0, #0x30d0, lsl #16               */
    .long 0xd5181000 	/* msr	sctlr_el1, x0                      */
    .long 0x910003e0 	/* mov	x0, sp                             */
    .long 0xd51c4100 	/* msr	sp_el1, x0                         */
    .long 0xd53ec000 	/* mrs	x0, vbar_el3                       */
    .long 0xd518c000 	/* msr	vbar_el1, x0                       */
    .long 0xd2803a60 	/* mov	x0, #0x1d3                         */
    .long 0xd51e4000 	/* msr	spsr_el3, x0                       */
    .long 0x10000500 	/* adr	x0, 1d008158 <el1_mode>            */
    .long 0xd51e4020 	/* msr	elr_el3, x0                        */
    .long 0xd69f03e0 	/* eret                                    */

el2_mode:
    .long 0xd53cc9a0 	/* mrs	x0, s3_4_c12_c9_5 - ICC_SRE_EL2    */
    .long 0xb2400c00 	/* orr	x0, x0, #0xf                       */
    .long 0xd51cc9a0 	/* msr	s3_4_c12_c9_5, x0                  */
    .long 0xd5033fdf 	/* isb                                     */
    .long 0xd538cca0 	/* mrs	x0, s3_0_c12_c12_5 - ICC_SRE_EL1   */
    .long 0xb2400000 	/* orr	x0, x0, #0x1                       */
    .long 0xd518cca0 	/* msr	s3_0_c12_c12_5, x0                 */
    .long 0xd5033fdf 	/* isb                                     */
    .long 0xd53ce100 	/* mrs	x0, cnthctl_el2                    */
    .long 0xb2400400 	/* orr	x0, x0, #0x3                       */
    .long 0xd51ce100 	/* msr	cnthctl_el2, x0                    */
    .long 0xd51ce07f 	/* msr	cntvoff_el2, xzr                   */
    .long 0xd5380000 	/* mrs	x0, midr_el1                       */
    .long 0xd53800a1 	/* mrs	x1, mpidr_el1                      */
    .long 0xd51c0000 	/* msr	vpidr_el2, x0                      */
    .long 0xd51c00a1 	/* msr	vmpidr_el2, x1                     */
    .long 0xd2867fe0 	/* mov	x0, #0x33ff                        */
    .long 0xd51c1140 	/* msr	cptr_el2, x0                       */
    .long 0xd51c117f 	/* msr	hstr_el2, xzr                      */
    .long 0xd2a00600 	/* mov	x0, #0x300000                      */
    .long 0xd5181040 	/* msr	cpacr_el1, x0                      */
    .long 0xd2800000 	/* mov	x0, #0x0                           */
    .long 0xb2630000 	/* orr	x0, x0, #0x20000000                */
    .long 0xd51c1100 	/* msr	hcr_el2, x0                        */
    .long 0xd53c1100 	/* mrs	x0, hcr_el2                        */
    .long 0xd2810000 	/* mov	x0, #0x800                         */
    .long 0xf2a61a00 	/* movk	x0, #0x30d0, lsl #16               */
    .long 0xd5181000 	/* msr	sctlr_el1, x0                      */
    .long 0x910003e0 	/* mov	x0, sp                             */
    .long 0xd51c4100 	/* msr	sp_el1, x0                         */
    .long 0xd53cc000 	/* mrs	x0, vbar_el2                       */
    .long 0xd518c000 	/* msr	vbar_el1, x0                       */
    .long 0xd2803a60 	/* mov	x0, #0x1d3                         */
    .long 0xd51c4000 	/* msr	spsr_el2, x0                       */
    .long 0x10000060 	/* adr	x0, 1d008158 <el1_mode>            */
    .long 0xd51c4020 	/* msr	elr_el2, x0                        */
    .long 0xd69f03e0 	/* eret                                    */
el1_mode:
#endif
	bl	  FTraceUartInit
	bl    FTraceBootup
	cpsid i						/* Mask interrupts */

    mov      r0, #0
	mov      r1, #0
	mov      r2, #0
	mov      r3, #0
	mov      r4, #0
	mov      r5, #0
	mov      r6, #0
	mov      r7, #0
	mov      r8, #0
	mov      r9, #0
	mov      r10, #0
	mov      r11, #0
	mov      r12, #0
    mcr  p15, 0, r0, c1, c0, 0  /* reset control register */
    isb

    /* set VBAR to the vector_base address in linker script */
	ldr	r0, =vector_base
	mcr	p15, 0, r0, c12, c0, 0

    /* Invalidate caches and TLBs */
	mov	r0,#0				/* r0 = 0  */
	mcr	p15, 0, r0, c8, c7, 0		/* invalidate TLBs */
	mcr	p15, 0, r0, c7, c5, 0		/* invalidate icache */
	mcr	p15, 0, r0, c7, c5, 6		/* Invalidate branch predictor array */
	bl	invalidate_dcache		/* invalidate dcache */

	/* Disable MMU, if enabled */
	mrc	p15, 0, r0, c1, c0, 0		/* read CP15 register 1 */
	bic	r0, r0, #0x1			    /* clear bit 0 */
	mcr	p15, 0, r0, c1, c0, 0		/* write value back */
    isb

	mrc p15, 0, r1, c1, c0, 0 	/* Read Control Register configuration data */
    bic r1, r1, #(0x1 << 12) 	/* Disable I Cache */
    bic r1, r1, #(0x1 << 2) 	/* Disable D Cache */
    mcr p15, 0, r1, c1, c0, 0 	/* Write Control Register configuration data */ 
	
    /* disable the data alignment check */
    mrc p15, 0, r1, c1, c0, 0
    bic r1, #(1<<1)
    mcr p15, 0, r1, c1, c0, 0
    isb

    mrs	r0, cpsr			/* get the current PSR */
	mvn	r1, #0x1f			/* set up the irq stack pointer */
	and	r2, r1, r0
	orr	r2, r2, #0x12			/* IRQ mode */
	msr	cpsr, r2
	ldr	r13,=_irq_stack			/* IRQ stack pointer */

	mrs	r0, cpsr			/* get the current PSR */
	mvn	r1, #0x1f			/* set up the Abort  stack pointer */
	and	r2, r1, r0
	orr	r2, r2, #0x17			/* Abort mode */
	msr	cpsr, r2
	ldr	r13,=_abort_stack		/* Abort stack pointer */

	mrs	r0, cpsr			/* get the current PSR */
	mvn	r1, #0x1f			/* set up the FIQ stack pointer */
	and	r2, r1, r0
	orr	r2, r2, #0x11			/* FIQ mode */
	msr	cpsr, r2
	ldr	r13,=_fiq_stack			/* FIQ stack pointer */

	mrs	r0, cpsr			/* get the current PSR */
	mvn	r1, #0x1f			/* set up the Undefine stack pointer */
	and	r2, r1, r0
	orr	r2, r2, #0x1b			/* Undefine mode */
	msr	cpsr, r2
	ldr	r13,=_undef_stack		/* Undefine stack pointer */
    

    mrs	r0, cpsr			    /* get the current PSR */
	mvn	r1, #0x1f			    /* set up the supervisor stack pointer */
	and	r2, r1, r0
	orr	r2, r2, #0x13			/* supervisor mode */
	msr	cpsr, r2
	ldr	r13,=_svc_stack			/* Supervisor stack pointer */

	mrs	r0, cpsr			/* get the current PSR */
	mvn	r1, #0x1f			/* set up the system stack pointer */
	and	r2, r1, r0
	orr	r2, r2, #0x1F			/* SYS mode */
	msr	cpsr, r2
	ldr	r13,=_sys_stack			/* SYS stack pointer */
	
    bl 	 _startup		/* jump to start */

invalidate_dcache:
	mrc	p15, 1, r0, c0, c0, 1		/* read CLIDR */
	ands	r3, r0, #0x7000000
	mov	r3, r3, lsr #23			/* cache level value (naturally aligned) */
	beq	finished
	mov	r10, #0				/* start with level 0 */
loop1:
	add	r2, r10, r10, lsr #1		/* work out 3xcachelevel */
	mov	r1, r0, lsr r2			/* bottom 3 bits are the Cache type for this level */
	and	r1, r1, #7			/* get those 3 bits alone */
	cmp	r1, #2
	blt	skip				/* no cache or only instruction cache at this level */
	mcr	p15, 2, r10, c0, c0, 0		/* write the Cache Size selection register */
	isb					/* isb to sync the change to the CacheSizeID reg */
	mrc	p15, 1, r1, c0, c0, 0		/* reads current Cache Size ID register */
	and	r2, r1, #7			/* extract the line length field */
	add	r2, r2, #4			/* add 4 for the line length offset (log2 16 bytes) */
	ldr	r4, =0x3ff
	ands	r4, r4, r1, lsr #3		/* r4 is the max number on the way size (right aligned) */
	clz	r5, r4				/* r5 is the bit position of the way size increment */
	ldr	r7, =0x7fff
	ands	r7, r7, r1, lsr #13		/* r7 is the max number of the index size (right aligned) */
loop2:
	mov	r9, r4				/* r9 working copy of the max way size (right aligned) */
loop3:
	orr	r11, r10, r9, lsl r5		/* factor in the way number and cache number into r11 */
	orr	r11, r11, r7, lsl r2		/* factor in the index number */
	mcr	p15, 0, r11, c7, c6, 2		/* invalidate by set/way */
	subs	r9, r9, #1			/* decrement the way number */
	bge	loop3
	subs	r7, r7, #1			/* decrement the index */
	bge	loop2
skip:
	add	r10, r10, #2			/* increment the cache number */
	cmp	r3, r10
	bgt	loop1

finished:
	mov	r10, #0				/* switch back to cache level 0 */
	mcr	p15, 2, r10, c0, c0, 0		/* select current cache level in cssr */
	dsb
	isb

	bx	lr

.end