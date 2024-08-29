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
 * FilePath: fcrt0.S
 * Date: 2022-02-10 14:53:41
 * LastEditTime: 2022-02-17 17:31:05
 * Description:  This file is for 64bit C run-time code
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  huanghe   2021/11/13       initialization
 *  1.1  zhugengyu	2022/06/05	add debugging information
 *  1.2  zhugengyu 2023/02/23   support dcache flush for early boot
 */
#include "sdkconfig.h"
    .file "fcrt0.S"
    .section ".got2","aw"
    .align 2

    .text

    .align 4	
.LsbssStart:
    .quad __sbss_start__

.LsbssEnd:
    .quad __sbss_end__


.LbssStart:
    .quad __bss_start__

.LbssEnd:
    .quad __bss_end__


.LdataStart:
    .quad __data_start__

.LdataEnd:
    .quad  __data_end__

.LRomEnd:
    .quad __rom_end__

    .globl _startup
_startup:
    TLBI    VMALLE1
    ic      IALLU   /* Invalidate I cache to PoU */
	bl 		InvalidateFlushDcaches
	dsb	 	sy
	isb

	/* Disable MMU first */
	mov	x1,#0x0
	msr     SCTLR_EL1, x1
	isb

    mov x0,#0

/* clear sbss */

ldr x1,.LsbssStart
ldr x2,.LsbssEnd

.LloopsBss:
	cmp	x1,x2
	bge	.LenclsBss		    /* If no SBSS, no clearing required */
	str	x0, [x1], #8
	b	.LloopsBss

.LenclsBss:
    /* clear bss */
	ldr	x1,.LbssStart		/* calculate beginning of the BSS */
	ldr	x2,.LbssEnd		    /* calculate end of the BSS */


.LloopBss:
	cmp	x1,x2
	bge	.LenclBss		/* If no BSS, no clearing required */
	str	x0, [x1], #8
	b	.LloopBss

.LenclBss:

    ldr x0,.LRomEnd
    ldr x1,.LdataStart
    ldr x2,.LdataEnd

.LdataCopy:
    cmp x1,x2
    ldp x10,x11,[x0],#16 /* copy from source address [x1] */
    stp x10,x11,[x1],#16 /* copy from source address [x1] */
    blt .LdataCopy

.LendataCopy:
    /* flush dcache first */
	bl MmuInit

    /* Enable SError Exception for asynchronous abort */
	mrs 	x1,DAIF
    bic	x1,x1,#(0x1<<8) /* clear SError interrupt mask bit */
	bic	x1,x1,#(0x1<<7)	/* clear IRQ mask bit */
    msr	DAIF,x1

    /*Enable MMU */ 
	mov	x1,#0x0
	orr     x1, x1, #(1 << 18)     /* Set WFE non trapping */
	orr     x1, x1, #(1 << 17)     /* Set WFI non trapping */
	orr     x1, x1, #(1 << 5)      /* Set CP15 barrier enabled */
	orr     x1, x1, #(1 << 14)     /* This control does not cause any instructions to be trapped. */
	orr     x1, x1, #(1 << 12)     /* Set I bit */
	orr     x1, x1, #(1 << 2)      /* Set C bit */
	orr     x1, x1, #(1 << 0)      /* Set M bit */
	msr     SCTLR_EL1, x1
	isb


#ifdef CONFIG_USE_AMP    
    bl SpinInit               /* init spin lock init */
#endif
    bl FEarlyUartProbe        /* init early uart */
	bl InterruptEarlyInit
    bl FPsciInit
#ifdef CONFIG_ENABLE_CXX
    /* Call constructors of all cpp global objects */
    bl  FCxxInitGlobals
#endif

	bl FTraceCEntry
	
    /* make sure argc and argv are valid */
	mov	x0, #0
	mov	x1, #0
	
    bl main

#ifdef CONFIG_ENABLE_CXX
    /* Call deconstructors of all cpp global objects */
    bl  FCxxDeInitGlobals
#endif

.Lexit:	/* should never get here */
	b .Lexit

/* flush or invalidate dcache, refer to rt-thread __asm_flush_dcache_all */
InvalidateFlushDcacheLevel:
    lsl    x12, x0, #1
    msr    csselr_el1, x12       /* select cache level */
    isb                          /* sync change of cssidr_el1 */
    mrs    x6, ccsidr_el1        /* read the new cssidr_el1 */
    and    x2, x6, #7            /* x2 <- log2(cache line size)-4 */
    add    x2, x2, #4            /* x2 <- log2(cache line size) */
    mov    x3, #0x3ff			 
    and    x3, x3, x6, lsr #3    /* x3 <- max number of #ways, ccsidr_el1[12:3] */
    clz    w5, w3                /* bit position of #ways */
    mov    x4, #0x7fff
    and    x4, x4, x6, lsr #13    /* x4 <- max number of #sets, ccsidr_el1[27:13] */
    /* x12 <- cache level << 1 */
    /* x2 <- line length offset */
    /* x3 <- number of cache ways - 1 */
    /* x4 <- number of cache sets - 1 */
    /* x5 <- bit position of #ways */

InvalidateFlushCacheSet:
    mov    x6, x3            /* x6 <- working copy of #ways */	

InvalidateFlushCacheWay:
    lsl    x7, x6, x5		  /* x7 = x6 << x5 */
    orr    x9, x12, x7        /* x9 = x12 | x7,  map way and level to cisw value */
    lsl    x7, x4, x2
    orr    x9, x9, x7        /* map set number to cisw value */
    tbz    w1, #0, 1f		 /* x1 = 0f, flush cache */
    dc    isw, x9			 /* invalidate dcache */
    b    2f
1:    dc    cisw, x9         /* clean & invalidate by set/way */
2:    subs    x6, x6, #1        /* decrement the way */
    b.ge    InvalidateFlushCacheWay
    subs    x4, x4, #1        /* decrement the set */
    b.ge    InvalidateFlushCacheSet

    ret

.global InvalidateFlushDcaches
InvalidateFlushDcaches:
    mov    x1, x0                /* x1 = 0 flush, x1 = 1 invalidate */
    dsb    sy                    /* barrier for full system */
    mrs    x10, clidr_el1        /* read clidr_el1 */
    lsr    x11, x10, #24
    and    x11, x11, #0x7        /* x11 <- loc bit[26:24], level of cache hierarchy */
    cbz    x11, InvalidateFlushDcacheEnd        /* if loc is 0, no cache, exit */
    mov    x15, lr           /* preserve LR */
    mov    x0, #0            /* start flush at cache level 0 */
    /* x0  <- cache level */
    /* x10 <- clidr_el1 */
    /* x11 <- loc */
    /* x15 <- return address */

InvalidateFlushCachesLoopLevel:
    lsl    x12, x0, #1         /* x12 = x0 * 2 */
    add    x12, x12, x0        /* x0 <- tripled cache level */
    lsr    x12, x10, x12       /* get x10, clidr_el1[ctype-n] to x12 */
    and    x12, x12, #7        /* x12 <- cache type */
    cmp    x12, #2             /* if not 000(no-cache), 001(i-cache only) */
    b.lt   InvalidateFlushCachesSkipLevel /* skip if no cache or icache */
    bl     InvalidateFlushDcacheLevel     /* x1 = 0 flush, 1 invalidate */

InvalidateFlushCachesSkipLevel:
    add    x0, x0, #1        /* increment cache level */
    cmp    x11, x0
    b.gt   InvalidateFlushCachesLoopLevel

    mov    x0, #0
    msr    csselr_el1, x0        /* restore csselr_el1 */
    dsb    sy
    isb
    mov    lr, x15

InvalidateFlushDcacheEnd:
	ret

.Lstart:
	.size	_startup,.Lstart-_startup




