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
 * FilePath: sys.c
 * Date: 2022-02-24 13:56:43
 * LastEditTime: 2022-03-21 17:04:18
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */


#include <metal/compiler.h>
#include <metal/io.h>
#include <metal/sys.h>
#include <stdint.h>
#ifndef __aarch64__
#include "faarch32.h"
#endif
#include "fmmu.h"
#include "fcache.h"

#define _DISABLE_INTERRUPTS()           \
	__asm volatile("MSR DAIFSET, #2" :: \
					   : "memory");     \
	__asm volatile("DSB SY");           \
	__asm volatile("ISB SY");

#define _ENABLE_INTERRUPTS()            \
	__asm volatile("MSR DAIFCLR, #2" :: \
					   : "memory");     \
	__asm volatile("DSB SY");           \
	__asm volatile("ISB SY");

void sys_irq_restore_enable(unsigned int flags)
{
#ifdef __aarch64__
	_ENABLE_INTERRUPTS();
#else
	MTCPSR(flags);
#endif
}

unsigned int sys_irq_save_disable(void)
{
	unsigned int state = 0;

#ifdef __aarch64__
	_DISABLE_INTERRUPTS();
#else
	state = MFCPSR();
	MTCPSR(state | 0xc0);
#endif

	return state;
}

void metal_machine_cache_flush(void *addr, unsigned int len)
{
	if (!addr && !len)
		FCacheDCacheFlush();
	else
		FCacheDCacheFlushRange((uintptr_t)addr, len);
}

void metal_machine_cache_invalidate(void *addr, unsigned int len)
{
	if (!addr && !len)
		FCacheDCacheInvalidate();
	else
		FCacheDCacheInvalidateRange((uintptr_t)addr, len);
}


void metal_weak metal_generic_default_poll(void)
{
	metal_asm volatile("wfi");
}



void *metal_machine_io_mem_map(void *va, metal_phys_addr_t pa,
							   size_t size, unsigned int flags)
{	
	if (!flags)
		return va;
		
	FSetTlbAttributes(pa, size, flags);

	return va;
}

