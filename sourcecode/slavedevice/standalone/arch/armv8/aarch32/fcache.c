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
 * FilePath: fcache.c
 * Date: 2022-02-10 14:53:41
 * LastEditTime: 2022-02-17 17:29:23
 * Description:  This file is for the arm cache functionality.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe     2021/7/3     first release
 */


#include "ftypes.h"
#include "fcache.h"
#include "faarch32.h"

#define FREG_CONTROL_DCACHE_BIT (0x00000001U << 2U)
#define FREG_CONTROL_ICACHE_BIT (0x00000001U << 12U)
#define IRQ_FIQ_MASK 0xC0U /* Mask IRQ and FIQ interrupts in cpsr */
#define SELECT_D_CACHE 0

extern s32 _svc_stack_end;
extern s32 _fiq_stack_start;

static inline u32 FCacheIcacheLineSize(void)
{
    u32 ctr = AARCH32_READ_SYSREG_32(CTR);
    return 4 << (ctr & 0xF);
}

static inline u32 FCacheDcacheLineSize(void)
{
    u32 ctr = AARCH32_READ_SYSREG_32(CTR);
    return 4 << ((ctr >> 16) & 0xF);
}

/*  Dcache */
void FCacheDCacheEnable(void)
{
    u32 ctrl_reg;
    ctrl_reg = AARCH32_READ_SYSREG_32(SCTLR);

    if ((ctrl_reg & FREG_CONTROL_DCACHE_BIT) == 0x00000000U)
    {
        FCacheDCacheInvalidate();

        ctrl_reg |= FREG_CONTROL_DCACHE_BIT;
        AARCH32_WRITE_SYSREG_32(SCTLR, ctrl_reg);
    }
}



void FCacheDCacheDisable(void)
{
    u32 ctrl_reg;
    FCacheDCacheFlush();
    ctrl_reg = AARCH32_READ_SYSREG_32(SCTLR);

    ctrl_reg &= ~(FREG_CONTROL_DCACHE_BIT);
    AARCH32_WRITE_SYSREG_32(SCTLR, ctrl_reg);
}

void FCacheDCacheFlushLine(intptr adr)
{
    u32 currmask;

    currmask = MFCPSR();
    MTCPSR(currmask | IRQ_FIQ_MASK);

    /* Clean data or unified cache line by virtual address to PoC. */
    AARCH32_WRITE_SYSREG_32(DCCMVAC, (adr & (~0x3F)));

    /* Wait for invalidate to complete */
    DSB();
    MTCPSR(currmask);
}

void FCacheDCacheInvalidateLine(intptr adr)
{
    u32 currmask;

    currmask = MFCPSR();
    MTCPSR(currmask | IRQ_FIQ_MASK);

    /* Invalidate data or unified cache line by virtual address to PoC. */
    AARCH32_WRITE_SYSREG_32(DCIMVAC, (adr & (~0x3F)));

    /* Wait for invalidate to complete */
    DSB();
    MTCPSR(currmask);
}

extern s32 _supervisor_stack_end ;
extern s32 __undef_stack ;

void FCacheDCacheInvalidate(void)
{
    register u32 csid_reg;
    register u32 c7_reg;
    u32 line_size;
    u32 num_ways;
    u32 way;
    u32 way_index;
    u32 way_adjust;
    u32 set;
    u32 set_index;
    u32 num_set;
    u32 num_cache_level;
    u32 cache_level;
    u32 currmask;
    u32 stack_start;
    u32 stack_end;
    u32 stack_size;

    stack_end = (u32)&_supervisor_stack_end;
    stack_start = (u32)&__undef_stack;
    stack_size = stack_start - stack_end;

    /*Flush stack memory to save return address */
    FCacheDCacheFlushRange(stack_end, stack_size);

    currmask = MFCPSR();
    MTCPSR(currmask | IRQ_FIQ_MASK);

    /* Number of level of cache */
    num_cache_level = (AARCH32_READ_SYSREG_32(CLIDR) >> 24U) & 0x00000007U;

    for (cache_level = 0U, way_adjust = 0x1E; cache_level < num_cache_level;
         cache_level++, way_adjust = way_adjust - 2)
    {

        AARCH32_WRITE_SYSREG_32(CSSELR, ((cache_level << 1) | SELECT_D_CACHE));

        ISB();

        csid_reg = AARCH32_READ_SYSREG_32(CCSIDR);

        /* Get the cacheline size, way size, index size from csidr */
        line_size = (csid_reg & 0x00000007U) + 0x00000004U;

        /* Number of ways */
        num_ways = (csid_reg & 0x00001FFFU) >> 3U;
        num_ways += 0x00000001U;

        /*Number of set */
        num_set = (csid_reg >> 13U) & 0x00007FFFU;
        num_set += 0x00000001U;

        way = 0U;
        set = 0U;

        /* Invalidate all the cachelines */
        for (way_index = 0U; way_index < num_ways; way_index++)
        {
            for (set_index = 0U; set_index < num_set; set_index++)
            {
                c7_reg = way | set | (cache_level << 1);
                AARCH32_WRITE_SYSREG_32(DCISW, c7_reg);
                set += (0x00000001U << line_size);
            }
            set = 0U;
            way += (0x00000001U << way_adjust);
        }

        /* Wait for invalidate to complete */
        DSB();
    }
    MTCPSR(currmask);
}

void FCacheDCacheInvalidateRange(intptr adr, intptr len)
{
    const u32 cacheline = FCacheDcacheLineSize();
    u32 end;
    u32 tempadr = adr;
    u32 tempend;
    u32 currmask;

    currmask = MFCPSR();
    MTCPSR(currmask | IRQ_FIQ_MASK);
    if (len != 0U)
    {
        end = tempadr + len;
        tempend = end;

        if ((tempadr & (cacheline - 1U)) != 0U)
        {
            tempadr &= (~(cacheline - 1U));
            FCacheDCacheInvalidateLine(tempadr);
            tempadr += cacheline;
        }
        if ((tempend & (cacheline - 1U)) != 0U)
        {
            tempend &= (~(cacheline - 1U));
            FCacheDCacheInvalidateLine(tempend);
        }

        while (tempadr < tempend)
        {
            /* Select cache level 0 and D cache in CSSR */
            AARCH32_WRITE_SYSREG_32(CSSELR, 0x0);
            /* Invalidate Data cache line */
            AARCH32_WRITE_SYSREG_32(DCIMVAC, (tempadr & (~0x3F)));  

            /* Wait for invalidate to complete */
            DSB();
            /* Select cache level 0 and D cache in CSSR */
            AARCH32_WRITE_SYSREG_32(CSSELR, 0x2);

            /* Invalidate Data cache line */
            AARCH32_WRITE_SYSREG_32(DCIMVAC, (tempadr & (~0x3F)));

            /* Wait for invalidate to complete */
            DSB();
            tempadr += cacheline;
        }
    }
    MTCPSR(currmask);
}

void FCacheDCacheFlush(void)
{
    register u32 csid_reg;
    register u32 c7_reg;
    u32 line_size;
    u32 num_ways;
    u32 way;
    u32 way_index;
    u32 way_adjust;
    u32 set;
    u32 set_index;
    u32 num_set;
    u32 num_cache_level;
    u32 cache_level;
    u32 currmask;

    currmask = MFCPSR();
    MTCPSR(currmask | IRQ_FIQ_MASK);

    /* Number of level of cache*/
    num_cache_level = (AARCH32_READ_SYSREG_32(CLIDR) >> 24U) & 0x00000007U;
    

    for (cache_level = 0U, way_adjust = 0x1E; cache_level < num_cache_level;
         cache_level++, way_adjust = way_adjust - 2)
    {
        /* Select cache level  and D cache in CSSELR */
        AARCH32_WRITE_SYSREG_32(CSSELR, ((cache_level << 1) | SELECT_D_CACHE));
        ISB();

        csid_reg = AARCH32_READ_SYSREG_32(CCSIDR);

        /* Get the cacheline size, way size, index size from csidr */
        line_size = (csid_reg & 0x00000007U) + 0x00000004U;

        /* Number of ways */
        num_ways = (csid_reg & 0x00001FFFU) >> 3U;
        num_ways += 0x00000001U;

        /*Number of set*/
        num_set = (csid_reg >> 13U) & 0x00007FFFU;
        num_set += 0x00000001U;

        way = 0U;
        set = 0U;

        /* Invalidate all the cachelines */
        for (way_index = 0U; way_index < num_ways; way_index++)
        {
            for (set_index = 0U; set_index < num_set; set_index++)
            {
                c7_reg = way | set | (cache_level << 1);
                AARCH32_WRITE_SYSREG_32(DCCISW, c7_reg);
                set += (0x00000001U << line_size);
            }
            set = 0U;
            way += (0x00000001U << way_adjust);
        }

        /* Wait for invalidate to complete */
        DSB();
    }
    MTCPSR(currmask);
}

void FCacheDCacheFlushRange(intptr adr, intptr len)
{
    const u32 cacheline = FCacheDcacheLineSize();
    u32 end;
    u32 tempadr = adr;
    u32 tempend;
    u32 currmask;

    currmask = MFCPSR();
    MTCPSR(currmask | IRQ_FIQ_MASK);
    if (len != 0U)
    {
        end = tempadr + len;
        tempend = end;

        if ((tempadr & (cacheline - 1U)) != 0U)
        {
            tempadr &= (~(cacheline - 1U));
            FCacheDCacheFlushLine(tempadr);
            tempadr += cacheline;
        }
        if ((tempend & (cacheline - 1U)) != 0U)
        {
            tempend &= (~(cacheline - 1U));
            FCacheDCacheFlushLine(tempend);
        }

        while (tempadr < tempend)
        {
            /* Clean Data cache line */
            AARCH32_WRITE_SYSREG_32(DCCMVAC, (tempadr & (~0x3F)));

            /* Wait for invalidate to complete */
            DSB();
            tempadr += cacheline;
        }
    }
    MTCPSR(currmask);
}

/*  Icache */

void FCacheICacheEnable(void)
{
    u32 ctrl_reg;

    ctrl_reg = AARCH32_READ_SYSREG_32(SCTLR);
    /* enable caches only if they are disabled */
    if ((ctrl_reg & FREG_CONTROL_ICACHE_BIT) == 0x00000000U)
    {
        /* invalidate the instruction cache */
        FCacheICacheInvalidate();
        ctrl_reg |= FREG_CONTROL_ICACHE_BIT;
        /* enable the instruction cache */
        AARCH32_WRITE_SYSREG_32(SCTLR, ctrl_reg);
    }
}

void FCacheICacheDisable(void)
{
    u32 ctrl_reg;

    ctrl_reg = AARCH32_READ_SYSREG_32(SCTLR);
    /* invalidate the instruction cache */
    FCacheICacheInvalidate();
    ctrl_reg &= ~(FREG_CONTROL_ICACHE_BIT);
    /* disable the instruction cache */
    AARCH32_WRITE_SYSREG_32(SCTLR, ctrl_reg);
}

void FCacheICacheInvalidate(void)
{
    unsigned int currmask;

    currmask = MFCPSR();
    MTCPSR(currmask | IRQ_FIQ_MASK);
    AARCH32_WRITE_SYSREG_32(CSSELR, 0x1); /* select Instruction cache. */
    ISB();
    DSB();
    /* invalidate the instruction cache */
    AARCH32_WRITE_SYSREG_32(ICIALLU, 0x0);
    /* Wait for invalidate to complete */
    DSB();
    MTCPSR(currmask);
}

void FCacheICacheInvalidateLine(u32 adr)
{
    u32 currmask;

    currmask = MFCPSR();
    MTCPSR(currmask | IRQ_FIQ_MASK);

    AARCH32_WRITE_SYSREG_32(CSSELR, 0x1);
    /*Invalidate I Cache line*/
    AARCH32_WRITE_SYSREG_32(ICIMVAU, adr & (~0x3F));
    /* Wait for invalidate to complete */
    DSB();
    MTCPSR(currmask);
}

void FCacheICacheInvalidateRange(intptr adr, u32 len)
{
    const u32 cacheline = FCacheIcacheLineSize();
    u32 end;
    u32 tempadr = adr;
    u32 tempend;
    u32 currmask;

    currmask = MFCPSR();
    MTCPSR(currmask | IRQ_FIQ_MASK);

    if (len != 0x00000000U)
    {
        end = tempadr + len;
        tempend = end;
        tempadr &= ~(cacheline - 0x00000001U);

        /* Select cache Level 0 I-cache in CSSR */
        AARCH32_WRITE_SYSREG_32(CSSELR, 0x1);
        ISB();
        while (tempadr < tempend)
        {
            /*Invalidate I Cache line*/
            AARCH32_WRITE_SYSREG_32(ICIMVAU, adr & (~0x3F));

            tempadr += cacheline;
        }
    }
    /* Wait for invalidate to complete */
    DSB();
    MTCPSR(currmask);
}

void FCacheICacheFlush(void)
{
    AARCH32_WRITE_SYSREG_32(ICIALLU, 0);
    DSB();
    ISB();
}

