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
 * FilePath: fmmu.c
 * Date: 2022-02-10 14:53:41
 * LastEditTime: 2022-02-17 17:30:19
 * Description:  This file provides APIs for enabling/disabling MMU and setting the memory
 * attributes for sections, in the MMU translation table.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe     2021/7/3     first release
 */

#include "sdkconfig.h"
#include "fprintk.h"
#include "ftypes.h"
#include "fmmu.h"
#include "fcache.h"
#include "sdkconfig.h"
#include "fl3cache.h"
#include "faarch32.h"

#define DESC_SEC (0x2)          /* Section table select */

/* dump 2nd level page table */
void Dump2ndPageTable(u32 *ptb)
{
    int i;
    int fcnt = 0;

    for (i = 0; i < 256; i++)
    {
        u32 pte2 = ptb[i];
        if ((pte2 & 0x3) == 0)
        {
            if (fcnt == 0)
            {
                f_printk("    ");
            }
            f_printk("%04x: ", i);
            fcnt++;
            if (fcnt == 16)
            {
                f_printk("fault\r\n");
                fcnt = 0;
            }
            continue;
        }
        if (fcnt != 0)
        {
            f_printk("fault\r\n");
            fcnt = 0;
        }

        f_printk("    %04x: %lx: ", i, pte2);
        if ((pte2 & 0x3) == 0x1)
        {
            f_printk("L,ap:%lx,xn:%ld,texcb:%02lx\r\n",
                     ((pte2 >> 7) | (pte2 >> 4)) & 0xf,
                     (pte2 >> 15) & 0x1,
                     ((pte2 >> 10) | (pte2 >> 2)) & 0x1f);
        }
        else
        {
            f_printk("S,ap:%lx,xn:%ld,texcb:%02lx\r\n",
                     ((pte2 >> 7) | (pte2 >> 4)) & 0xf, pte2 & 0x1,
                     ((pte2 >> 4) | (pte2 >> 2)) & 0x1f);
        }
    }
}

void DumpPageTable(u32 *ptb)
{
    int i;
    int fcnt = 0;

    f_printk("page table@%p\r\n", ptb);
    for (i = 0; i < 1024 * 4; i++)
    {
        u32 pte1 = ptb[i];
        if ((pte1 & 0x3) == 0)
        {
            f_printk("%03x: ", i);
            fcnt++;
            if (fcnt == 16)
            {
                f_printk("fault\r\n");
                fcnt = 0;
            }
            continue;
        }
        if (fcnt != 0)
        {
            f_printk("fault\r\n");
            fcnt = 0;
        }

        f_printk("%03x: %08lx: ", i, pte1);
        if ((pte1 & 0x3) == 0x3)
        {
            f_printk("LPAE\r\n");
        }
        else if ((pte1 & 0x3) == 0x1)
        {
            f_printk("pte,ns:%ld,domain:%ld\r\n",
                     (pte1 >> 3) & 0x1, (pte1 >> 5) & 0xf);
            /*
             *rt_hw_cpu_dump_page_table_2nd((void*)((pte1 & 0xfffffc000)
             *                               - 0x80000000 + 0xC0000000));
             */
        }
        else if (pte1 & (1 << 18))
        {
            f_printk("super section,ns:%ld,ap:%lx,xn:%ld,texcb:%02lx\r\n",
                     (pte1 >> 19) & 0x1,
                     ((pte1 >> 13) | (pte1 >> 10)) & 0xf,
                     (pte1 >> 4) & 0x1,
                     ((pte1 >> 10) | (pte1 >> 2)) & 0x1f);
        }
        else
        {
            f_printk("section,ns:%ld,ap:%lx,"
                     "xn:%ld,texcb:%02lx,domain:%ld\r\n",
                     (pte1 >> 19) & 0x1,
                     ((pte1 >> 13) | (pte1 >> 10)) & 0xf,
                     (pte1 >> 4) & 0x1,
                     (((pte1 & (0x7 << 12)) >> 10) |
                      ((pte1 & 0x0c) >> 2)) &
                     0x1f,
                     (pte1 >> 5) & 0xf);
        }
    }
}

/* level1 page table, each entry for 1MB memory. */
static volatile unsigned long MMUTable[4 * 1024] __attribute__((aligned(16 * 1024)));
void SetMMUTable(u32 vaddrStart,
                 u32 vaddrEnd,
                 u32 paddrStart,
                 u32 attr)
{
    volatile u32 *pTT;
    volatile int i, nSec;
    pTT = (u32 *)MMUTable + (vaddrStart >> 20);
    nSec = (vaddrEnd >> 20) - (vaddrStart >> 20);
    for (i = 0; i <= nSec; i++)
    {
        *pTT = DESC_SEC | attr | (((paddrStart >> 20) + i) << 20);
        pTT++;
    }
}

/* DACR, Defines the access permission for each of the sixteen memory domains. */
u32 SetDomainReg(u32 domain_val)
{
    u32 old_domain;

    old_domain = AARCH32_READ_SYSREG_32(DACR);

    AARCH32_WRITE_SYSREG_32(DACR, domain_val);

    return old_domain;
}


void InitMMUTable(void)
{

    extern struct mem_desc platform_mem_desc[] ;
    extern const u32 platform_mem_desc_size ;

    struct mem_desc *mdesc ;
    u32 size ;

    mdesc = platform_mem_desc;
    size =  platform_mem_desc_size;
    for (; size > 0; size--)
    {

        SetMMUTable(mdesc->vaddr_start, mdesc->vaddr_end,
                    mdesc->paddr_start, mdesc->attr);
        mdesc++;
    }

}

void CpuMmuDisable(void)
{
    AARCH32_WRITE_SYSREG_32(TLBIALL, 0U);

    u32 reg_val = AARCH32_READ_SYSREG_32(SCTLR);
    reg_val &= ~(0x00000001U);
    AARCH32_WRITE_SYSREG_32(SCTLR, reg_val);
    DSB();
}

void CpuMmuEnable(void)
{
    u32 reg_val = AARCH32_READ_SYSREG_32(SCTLR);
    reg_val |= (0x00000001U);
    AARCH32_WRITE_SYSREG_32(SCTLR, reg_val);
    DSB();
}

void CpuTlbSet(volatile unsigned long *addr)
{
    u32 ttbr0 = ((u32)addr | 0x6a);
    AARCH32_WRITE_SYSREG_32(TTBR0, ttbr0);
    DMB();
}

void InitMMU(void)
{
    CpuMmuDisable();
    SetDomainReg(0x55555555); /* refer to DACR */
    CpuTlbSet(MMUTable);
    CpuMmuEnable();
}


/**
 * @name: InitCache
 * @msg:  Enable MMU for processor in 32bit mode. This function
 *          invalidates the instruction and data caches before enabling MMU.
 * @return {*}
 * @note:
 */
void InitCache(void)
{
    FCacheDCacheFlush();
    FCacheICacheFlush();
    FCacheL3CacheInvalidate();
    FCacheDCacheDisable();
    FCacheICacheDisable();
    FCacheL3CacheDisable();
    InitMMU();
    FCacheICacheEnable();
    FCacheDCacheEnable();
}

/**
 * @name: FSetTlbAttributes
 * @msg:  This function sets the memory attributes for a section
 * @param {uintptr} addr is 32-bit address for which the attributes need to be set.
 * @param {fsize_t} size of the mapped memory region in bytes
 * @param {u32} attrib or the specified memory region.
 * @return {*}
 */
void FSetTlbAttributes(uintptr addr, fsize_t size, u32 attrib)
{
    volatile u32 *ptt;
    volatile int i, nsec;
    ptt = (u32 *)MMUTable + (addr >> 20);
    nsec = size >> 20;

    for (i = 0; i <= nsec; i++)
    {
        *ptt = DESC_SEC | attrib | (((addr >> 20) + i) << 20) ;
        ptt++;
    }

    FCacheDCacheFlush();
    /* Invalidate all cached copies of translation table entries from TLBs that are from any level of the translation table walk. */
    AARCH32_WRITE_SYSREG_32(TLBIALL, 0U);
    /* Invalidate all entries from branch predictors. */
    AARCH32_WRITE_SYSREG_32(BPIALL, 0U);

    DSB();
    ISB();
}



