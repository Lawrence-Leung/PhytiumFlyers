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
 * LastEditTime: 2022-02-17 17:33:35
 * Description:  This file provides APIs for enabling/disabling MMU and setting the memory
 * attributes for sections, in the MMU translation table.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe     2021/7/3     first release
 */


#include "faarch64.h"
#include "fcache.h"
#include <sys/errno.h>
#include "ftypes.h"
#include "fassert.h"
#include "fmmu.h"
#include "fkernel.h"
#include "fl3cache.h"

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/******************************
 *  mmu config define
 ******************************/
#define CONFIG_ARM64_VA_BITS 42
#define CONFIG_ARM64_PA_BITS 42
#define CONFIG_MAX_XLAT_TABLES 16

/*
 * 48-bit address with 4KB granule size:
 *
 * +------------+------------+------------+------------+-----------+
 * | VA [47:39] | VA [38:30] | VA [29:21] | VA [20:12] | VA [11:0] |
 * +---------------------------------------------------------------+
 * |     L0     |     L1     |     L2     |     L3     | block off |
 * +------------+------------+------------+------------+-----------+
 */

/* Only 4K granule is supported */
#define PAGE_SIZE_SHIFT 12U

/* 48-bit VA address */
#define VA_SIZE_SHIFT_MAX 48U

/* Maximum 4 XLAT table levels (L0 - L3) */
#define XLAT_LAST_LEVEL 3U

/* The VA shift of L3 depends on the granule size */
#define L3_XLAT_VA_SIZE_SHIFT PAGE_SIZE_SHIFT

/* Number of VA bits to assign to each table (9 bits) */
#define LN_XLAT_VA_SIZE_SHIFT (PAGE_SIZE_SHIFT - 3)

/* Starting bit in the VA address for each level */
#define L2_XLAT_VA_SIZE_SHIFT (L3_XLAT_VA_SIZE_SHIFT + LN_XLAT_VA_SIZE_SHIFT) /* 21 */
#define L1_XLAT_VA_SIZE_SHIFT (L2_XLAT_VA_SIZE_SHIFT + LN_XLAT_VA_SIZE_SHIFT) /* 30 */
#define L0_XLAT_VA_SIZE_SHIFT (L1_XLAT_VA_SIZE_SHIFT + LN_XLAT_VA_SIZE_SHIFT) /* 39 */

#define LEVEL_TO_VA_SIZE_SHIFT(level)           \
    (PAGE_SIZE_SHIFT + (LN_XLAT_VA_SIZE_SHIFT * \
                        (XLAT_LAST_LEVEL - (level)))) /* 12 + (9*(3-level)) */

/* Number of entries for each table (512) */
#define LN_XLAT_NUM_ENTRIES ((1U << PAGE_SIZE_SHIFT) / 8U)

/* Virtual Address Index within a given translation table level */
#define XLAT_TABLE_VA_IDX(va_addr, level) \
    ((va_addr >> LEVEL_TO_VA_SIZE_SHIFT(level)) & (LN_XLAT_NUM_ENTRIES - 1))

/*
 * Calculate the initial translation table level from CONFIG_ARM64_VA_BITS
 * For a 4 KB page size:
 *
 * (va_bits <= 21)   - base level 3
 * (22 <= va_bits <= 30) - base level 2
 * (31 <= va_bits <= 39) - base level 1
 * (40 <= va_bits <= 48) - base level 0
 */
#define GET_BASE_XLAT_LEVEL(va_bits)          \
    ((va_bits > L0_XLAT_VA_SIZE_SHIFT)   ? 0U \
     : (va_bits > L1_XLAT_VA_SIZE_SHIFT) ? 1U \
     : (va_bits > L2_XLAT_VA_SIZE_SHIFT) ? 2U \
     : 3U)

/* Level for the base XLAT */
#define BASE_XLAT_LEVEL GET_BASE_XLAT_LEVEL(CONFIG_ARM64_VA_BITS)

#if (CONFIG_ARM64_PA_BITS == 48)
    #define TCR_PS_BITS TCR_PS_BITS_256TB
#elif (CONFIG_ARM64_PA_BITS == 44)
    #define TCR_PS_BITS TCR_PS_BITS_16TB
#elif (CONFIG_ARM64_PA_BITS == 42)
    #define TCR_PS_BITS TCR_PS_BITS_4TB
#elif (CONFIG_ARM64_PA_BITS == 40)
    #define TCR_PS_BITS TCR_PS_BITS_1TB
#elif (CONFIG_ARM64_PA_BITS == 36)
    #define TCR_PS_BITS TCR_PS_BITS_64GB
#else
    #define TCR_PS_BITS TCR_PS_BITS_4GB
#endif

/* Upper and lower attributes mask for page/block descriptor */
#define DESC_ATTRS_UPPER_MASK GENMASK(63, 51)
#define DESC_ATTRS_LOWER_MASK GENMASK(11, 2)

#define DESC_ATTRS_MASK (DESC_ATTRS_UPPER_MASK | DESC_ATTRS_LOWER_MASK)

#define SCTLR_M_BIT BIT(0)
#define SCTLR_A_BIT BIT(1)
#define SCTLR_C_BIT BIT(2)
#define SCTLR_SA_BIT BIT(3)
#define SCTLR_I_BIT BIT(12)

/*
 * TCR definitions.
 */
#define TCR_EL1_IPS_SHIFT 32U
#define TCR_EL2_PS_SHIFT 16U
#define TCR_EL3_PS_SHIFT 16U

#define TCR_T0SZ_SHIFT 0U
#define TCR_T0SZ(x) ((64 - (x)) << TCR_T0SZ_SHIFT)

#define TCR_IRGN_NC (0ULL << 8)
#define TCR_IRGN_WBWA (1ULL << 8)
#define TCR_IRGN_WT (2ULL << 8)
#define TCR_IRGN_WBNWA (3ULL << 8)
#define TCR_IRGN_MASK (3ULL << 8)
#define TCR_ORGN_NC (0ULL << 10)
#define TCR_ORGN_WBWA (1ULL << 10)
#define TCR_ORGN_WT (2ULL << 10)
#define TCR_ORGN_WBNWA (3ULL << 10)
#define TCR_ORGN_MASK (3ULL << 10)
#define TCR_SHARED_NON (0ULL << 12)
#define TCR_SHARED_OUTER (2ULL << 12)
#define TCR_SHARED_INNER (3ULL << 12)
#define TCR_TG0_4K (0ULL << 14)
#define TCR_TG0_64K (1ULL << 14)
#define TCR_TG0_16K (2ULL << 14)
#define TCR_EPD1_DISABLE (1ULL << 23)

#define TCR_PS_BITS_4GB 0x0ULL
#define TCR_PS_BITS_64GB 0x1ULL
#define TCR_PS_BITS_1TB 0x2ULL
#define TCR_PS_BITS_4TB 0x3ULL
#define TCR_PS_BITS_16TB 0x4ULL
#define TCR_PS_BITS_256TB 0x5ULL

/*
 * Caching mode definitions. These are mutually exclusive.
 */

/** No caching. Most drivers want this. */
#define K_MEM_CACHE_NONE 2

/** Write-through caching. Used by certain drivers. */
#define K_MEM_CACHE_WT 1

/** Full write-back caching. Any RAM mapped wants this. */
#define K_MEM_CACHE_WB 0

/** Reserved bits for cache modes in k_map() flags argument */
#define K_MEM_CACHE_MASK (BIT(3) - 1)

/*
 * Region permission attributes. Default is read-only, no user, no exec
 */

/** Region will have read/write access (and not read-only) */
#define K_MEM_PERM_RW BIT(3)

/** Region will be executable (normally forbidden) */
#define K_MEM_PERM_EXEC BIT(4)

/** Region will be accessible to user mode (normally supervisor-only) */
#define K_MEM_PERM_USER BIT(5)

/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

static u64 xlat_tables[CONFIG_MAX_XLAT_TABLES * LN_XLAT_NUM_ENTRIES] __aligned(LN_XLAT_NUM_ENTRIES *sizeof(u64));
static u16 xlat_use_count[CONFIG_MAX_XLAT_TABLES];

/************************** Function Prototypes ******************************/
extern void AsmInvalidateTlbAll();
/* Returns a reference to a free table */
static u64 *NewTable(void)
{
    unsigned int i;

    /* Look for a free table. */
    for (i = 0U; i < CONFIG_MAX_XLAT_TABLES; i++)
    {
        if (xlat_use_count[i] == 0U)
        {
            xlat_use_count[i] = 1U;
            return &xlat_tables[i * LN_XLAT_NUM_ENTRIES];
        }
    }

    MMU_DEBUG("CONFIG_MAX_XLAT_TABLES, too small");
    return NULL;
}

static inline unsigned int TableIndex(u64 *pte)
{
    unsigned int i = (pte - xlat_tables) / LN_XLAT_NUM_ENTRIES;

    FASSERT_MSG(i < CONFIG_MAX_XLAT_TABLES, "table %x out of range", pte);
    return i;
}

/* Makes a table free for reuse. */
static void FreeTable(u64 *table)
{
    unsigned int i = TableIndex(table);

    MMU_DEBUG("freeing table [%d]%x\r\n", i, table);
    FASSERT_MSG(xlat_use_count[i] == 1U, "table still in use");
    xlat_use_count[i] = 0U;
}

/* Adjusts usage count and returns current count. */
static int TableUsage(u64 *table, int adjustment)
{
    unsigned int i = TableIndex(table);

    xlat_use_count[i] += adjustment;
    FASSERT_MSG(xlat_use_count[i] > 0, "usage count underflow");
    return xlat_use_count[i];
}

static inline int IsTableUnused(u64 *table)
{
    return TableUsage(table, 0) == 1;
}

static inline int IsFreeDesc(u64 desc)
{
    return (desc & PTE_DESC_TYPE_MASK) == PTE_INVALID_DESC;
}

static inline int IsTableDesc(u64 desc, unsigned int level)
{
    return (level != XLAT_LAST_LEVEL) &&
           (desc & PTE_DESC_TYPE_MASK) == PTE_TABLE_DESC;
}

static inline int IsBlockDesc(u64 desc)
{
    return (desc & PTE_DESC_TYPE_MASK) == PTE_BLOCK_DESC;
}

static inline u64 *PteDescTable(u64 desc)
{
    u64 address = desc & GENMASK(47, PAGE_SIZE_SHIFT);

    return (u64 *)address;
}

static inline int IsDescSuperset(u64 desc1, u64 desc2,
                                 unsigned int level)
{
    u64 mask = DESC_ATTRS_MASK | GENMASK(47, LEVEL_TO_VA_SIZE_SHIFT(level));

    return (desc1 & mask) == (desc2 & mask);
}


static void DebugShowPte(u64 *pte, unsigned int level)
{
#if defined(DUMP_PTE)
    MMU_DEBUG("%d ", level);
    MMU_DEBUG("%.*s", level * 2U, ". . . ");
    MMU_DEBUG("[%d]%x: ", TableIndex(pte), pte);

    if (IsFreeDesc(*pte))
    {
        MMU_DEBUG("---\r\n");
        return;
    }

    if (IsTableDesc(*pte, level))
    {
        u64 *table = PteDescTable(*pte);

        MMU_DEBUG("[Table] [%d]%x\r\n", TableIndex(table), table);
        return;
    }

    if (IsBlockDesc(*pte))
    {
        MMU_DEBUG("[Block] ");
    }
    else
    {
        MMU_DEBUG("[Page] ");
    }

    uint8_t mem_type = (*pte >> 2) & MT_TYPE_MASK;

    MMU_DEBUG((mem_type == MT_NORMAL) ? "MEM" : ((mem_type == MT_NORMAL_NC) ? "NC" : "DEV"));
    MMU_DEBUG((*pte & PTE_BLOCK_DESC_AP_RO) ? "-RO" : "-RW");
    MMU_DEBUG((*pte & PTE_BLOCK_DESC_NS) ? "-NS" : "-S");
    MMU_DEBUG((*pte & PTE_BLOCK_DESC_AP_ELx) ? "-ELx" : "-ELh");
    MMU_DEBUG((*pte & PTE_BLOCK_DESC_PXN) ? "-PXN" : "-PX");
    MMU_DEBUG((*pte & PTE_BLOCK_DESC_UXN) ? "-UXN" : "-UX");
    MMU_DEBUG("\r\n");
#endif
    return;
}

static void SetPteTableDesc(u64 *pte, u64 *table, unsigned int level)
{
    /* Point pte to new table */
    *pte = PTE_TABLE_DESC | (u64)table;
    DebugShowPte(pte, level);
}

static void SetPteBlockDesc(u64 *pte, u64 desc, unsigned int level)
{
    if (desc)
    {
        desc |= (level == XLAT_LAST_LEVEL) ? PTE_PAGE_DESC : PTE_BLOCK_DESC;
    }
    *pte = desc;
    DebugShowPte(pte, level);
}

static u64 *ExpandToTable(u64 *pte, unsigned int level)
{
    u64 *table;

    FASSERT_MSG(level < XLAT_LAST_LEVEL, "can't expand last level");

    table = NewTable();
    if (!table)
    {
        return NULL;
    }

    if (!IsFreeDesc(*pte))
    {
        /*
         * If entry at current level was already populated
         * then we need to reflect that in the new table.
         */
        u64 desc = *pte;
        unsigned int i, stride_shift;

        MMU_DEBUG("expanding PTE 0x%016llx into table [%d]%x\r\n",
                  desc, TableIndex(table), table);
        FASSERT_MSG(IsBlockDesc(desc), "");

        if (level + 1 == XLAT_LAST_LEVEL)
        {
            desc |= PTE_PAGE_DESC;
        }

        stride_shift = LEVEL_TO_VA_SIZE_SHIFT(level + 1);
        for (i = 0U; i < LN_XLAT_NUM_ENTRIES; i++)
        {
            table[i] = desc | (i << stride_shift);
        }
        TableUsage(table, LN_XLAT_NUM_ENTRIES);
    }
    else
    {
        /*
         * Adjust usage count for parent table's entry
         * that will no longer be free.
         */
        TableUsage(pte, 1);
    }

    /* Link the new table in place of the pte it replaces */
    SetPteTableDesc(pte, table, level);
    TableUsage(table, 1);

    return table;
}


static int SetMapping(struct ArmMmuPtables *ptables,
                      uintptr virt, fsize_t size,
                      u64 desc, int may_overwrite)
{
    u64 *pte, *ptes[XLAT_LAST_LEVEL + 1];
    u64 level_size;
    u64 *table = ptables->base_xlat_table;
    unsigned int level = BASE_XLAT_LEVEL;
    int ret = 0;

    while (size)
    {
        FASSERT_MSG(level <= XLAT_LAST_LEVEL,
                    "max translation table level exceeded\r\n");

        /* Locate PTE for given virtual address and page table level */
        pte = &table[XLAT_TABLE_VA_IDX(virt, level)];
        ptes[level] = pte;

        if (IsTableDesc(*pte, level))
        {
            /* Move to the next translation table level */
            level++;
            table = PteDescTable(*pte);
            continue;
        }

        if (!may_overwrite && !IsFreeDesc(*pte))
        {
            /* the entry is already allocated */
            MMU_DEBUG("entry already in use: "
                      "level %d pte %x *pte 0x%016llx",
                      level, pte, *pte);
            ret = -EBUSY;
            break;
        }

        level_size = 1ULL << LEVEL_TO_VA_SIZE_SHIFT(level);

        if (IsDescSuperset(*pte, desc, level))
        {
            /* This block already covers our range */
            level_size -= (virt & (level_size - 1));
            if (level_size > size)
            {
                level_size = size;
            }
            goto move_on;
        }

        if ((size < level_size) || (virt & (level_size - 1)))
        {
            /* Range doesn't fit, create subtable */
            table = ExpandToTable(pte, level);
            if (!table)
            {
                ret = -ENOMEM;
                break;
            }
            level++;
            continue;
        }

        /* Adjust usage count for corresponding table */
        if (IsFreeDesc(*pte))
        {
            TableUsage(pte, 1);
        }
        if (!desc)
        {
            TableUsage(pte, -1);
        }
        /* Create (or erase) block/page descriptor */
        SetPteBlockDesc(pte, desc, level);

        /* recursively free unused tables if any */
        while (level != BASE_XLAT_LEVEL &&
               IsTableUnused(pte))
        {
            FreeTable(pte);
            pte = ptes[--level];
            SetPteBlockDesc(pte, 0, level);
            TableUsage(pte, -1);
        }

move_on:
        virt += level_size;
        desc += desc ? level_size : 0;
        size -= level_size;

        /* Range is mapped, start again for next range */
        table = ptables->base_xlat_table;
        level = BASE_XLAT_LEVEL;
        MMU_DEBUG("virt %p \r\n", virt);
    }

    return ret;
}

static u64 GetRegionDesc(u32 attrs)
{
    unsigned int mem_type;
    u64 desc = 0U;

    /* NS bit for security memory access from secure state */
    desc |= (attrs & MT_NS) ? PTE_BLOCK_DESC_NS : 0;

    /*
     * AP bits for EL0 / ELh Data access permission
     *
     *   AP[2:1]   ELh  EL0
     * +--------------------+
     *     00      RW   NA
     *     01      RW   RW
     *     10      RO   NA
     *     11      RO   RO
     */

    /* AP bits for Data access permission */
    desc |= (attrs & MT_RW) ? PTE_BLOCK_DESC_AP_RW : PTE_BLOCK_DESC_AP_RO;

    /* Mirror permissions to EL0 */
    desc |= (attrs & MT_RW_AP_ELX) ? PTE_BLOCK_DESC_AP_ELx : PTE_BLOCK_DESC_AP_EL_HIGHER;

    /* the access flag */
    desc |= PTE_BLOCK_DESC_AF;

    /* memory attribute index field */
    mem_type = MT_TYPE(attrs);
    desc |= PTE_BLOCK_DESC_MEMTYPE(mem_type);

    switch (mem_type)
    {
        case MT_DEVICE_NGNRNE:
        case MT_DEVICE_NGNRE:
        case MT_DEVICE_GRE:
            /* Access to Device memory and non-cacheable memory are coherent
             * for all observers in the system and are treated as
             * Outer shareable, so, for these 2 types of memory,
             * it is not strictly needed to set shareability field
             */
            desc |= PTE_BLOCK_DESC_OUTER_SHARE;
            /* Map device memory as execute-never */
            desc |= PTE_BLOCK_DESC_PXN;
            desc |= PTE_BLOCK_DESC_UXN;
            break;
        case MT_NORMAL_NC:
        case MT_NORMAL:
            /* Make Normal RW memory as execute never */
            if ((attrs & MT_RW) && (attrs & MT_P_EXECUTE_NEVER))
            {
                desc |= PTE_BLOCK_DESC_PXN;
            }

            if (((attrs & MT_RW) && (attrs & MT_RW_AP_ELX)) ||
                (attrs & MT_U_EXECUTE_NEVER))
            {
                desc |= PTE_BLOCK_DESC_UXN;
            }

            if (mem_type == MT_NORMAL)
            {
                desc |= PTE_BLOCK_DESC_OUTER_SHARE;
            }
            else
            {
                desc |= PTE_BLOCK_DESC_OUTER_SHARE;
            }
    }

    return desc;
}


static int AddMap(struct ArmMmuPtables *ptables, const char *name,
                  uintptr phys, uintptr virt, fsize_t size, u32 attrs)
{
    u64 desc = GetRegionDesc(attrs);
    int may_overwrite = !(attrs & MT_NO_OVERWRITE);

    MMU_DEBUG("mmap [%s]: virt %p phys %p size %p attr %p\r\n",
              name, virt, phys, size, desc);
    FASSERT_MSG(((virt | phys | size) & (CONFIG_MMU_PAGE_SIZE - 1)) == 0,
                "address/size are not page aligned\r\n");
    desc |= phys;
    return SetMapping(ptables, virt, size, desc, may_overwrite);
}


static int RemoveMap(struct ArmMmuPtables *ptables, const char *name,
                     uintptr virt, fsize_t size)
{
    int ret;

    MMU_DEBUG("unmmap [%s]: virt %p size %p\r\n", name, virt, size);
    FASSERT_MSG(((virt | size) & (CONFIG_MMU_PAGE_SIZE - 1)) == 0,
                "address/size are not page aligned\r\n");

    ret = SetMapping(ptables, virt, size, 0, 1);
    return ret;
}

static inline void AddArmMmuRegion(struct ArmMmuPtables *ptables,
                                   const struct ArmMmuRegion *region,
                                   u32 extra_flags)
{
    if (region->size || region->attrs)
    {
        /* MMU not yet active: must use unlocked version */
        AddMap(ptables, region->name, region->base_pa, region->base_va,
               region->size, region->attrs | extra_flags);
    }
}

static void SetupPageTables(struct ArmMmuPtables *ptables)
{
    unsigned int index;
    const struct ArmMmuRegion *region;
    uintptr max_va = 0, max_pa = 0;

    MMU_DEBUG("xlat tables:\r\n");
    for (index = 0U; index < CONFIG_MAX_XLAT_TABLES; index++)
    {
        MMU_DEBUG("%d: %x\r\n", index, xlat_tables + index * LN_XLAT_NUM_ENTRIES);
    }

    /* 从不同的board 中获取，内存映射表中地址范围 */
    for (index = 0U; index < mmu_config.num_regions; index++)
    {
        region = &mmu_config.mmu_regions[index];
        max_va = max(max_va, region->base_va + region->size);
        max_pa = max(max_pa, region->base_pa + region->size);
    }

    FASSERT_MSG(max_va <= (1ULL << CONFIG_ARM64_VA_BITS),
                "Maximum VA not supported\r\n");
    FASSERT_MSG(max_pa <= (1ULL << CONFIG_ARM64_PA_BITS),
                "Maximum PA not supported\r\n");

    /* setup translation table for execution regions */
    for (index = 0U; index < mmu_config.num_regions; index++)
    {
        region = &mmu_config.mmu_regions[index];
        AddArmMmuRegion(ptables, region, 0);
    }

    AsmInvalidateTlbAll();
}

/* Translation table control register settings */
static u64 GetTcr(int el)
{
    u64 tcr;
    u64 va_bits = CONFIG_ARM64_VA_BITS;
    u64 tcr_ps_bits;

    tcr_ps_bits = TCR_PS_BITS;

    if (el == 1)
    {
        tcr = (tcr_ps_bits << TCR_EL1_IPS_SHIFT);
        /*
         * TCR_EL1.EPD1: Disable translation table walk for addresses
         * that are translated using TTBR1_EL1.
         */
        tcr |= TCR_EPD1_DISABLE;
    }
    else
    {
        tcr = (tcr_ps_bits << TCR_EL3_PS_SHIFT);
    }

    tcr |= TCR_T0SZ(va_bits);
    /*
     * Translation table walk is cacheable, inner/outer WBWA and
     * inner shareable
     */
    tcr |= TCR_TG0_4K | TCR_SHARED_INNER | TCR_ORGN_WBWA | TCR_IRGN_WBWA;

    return tcr;
}

static void EnableMmuEl1(struct ArmMmuPtables *ptables, unsigned int flags)
{
    u64 val;

    /* Set MAIR, TCR and TBBR registers */
    __asm__ volatile("msr mair_el1, %0"
                     :
                     : "r"(MEMORY_ATTRIBUTES)
                     : "memory", "cc");
    __asm__ volatile("msr tcr_el1, %0"
                     :
                     : "r"(GetTcr(1))
                     : "memory", "cc");
    __asm__ volatile("msr ttbr0_el1, %0"
                     :
                     : "r"((u64)ptables->base_xlat_table)
                     : "memory", "cc");

    /* Ensure these changes are seen before MMU is enabled */
    ISB();

    /* Invalidate all data caches before enable them */
    FCacheDCacheInvalidate();

    /* Ensure the MMU enable takes effect immediately */
    ISB();

    MMU_DEBUG("MMU enabled with dcache\r\n");
}

/* ARM MMU Driver Initial Setup */

static struct ArmMmuPtables kernel_ptables;


/**
 * @name: MmuInit
 * @msg:  This function provides the default configuration mechanism for the Memory
 * Management Unit (MMU)
 * @return {*}
 */
void MmuInit(void)
{
    unsigned int flags = 0U;
    u64 val = 0U;
    FCacheL3CacheFlush();
    /* 增加粒度判断 */
    val = AARCH64_READ_SYSREG(ID_AA64MMFR0_EL1);


    FASSERT_MSG((CONFIG_MMU_PAGE_SIZE == KB(4)) && (!(val & ID_AA64MMFR0_EL1_4K_NO_SURPOORT)),
                "Only 4K page size is supported\r\n");

    /* Current MMU code supports only EL1 */
    val = AARCH64_READ_SYSREG(CurrentEL);

    FASSERT_MSG(GET_EL(val) == MODE_EL1, "Exception level not EL1, MMU not enabled!\n");

    /* Ensure that MMU is already not enabled */
    val = AARCH64_READ_SYSREG(sctlr_el1);
    FASSERT_MSG((val & SCTLR_ELx_M) == 0, "MMU is already enabled\n");

    kernel_ptables.base_xlat_table = NewTable();
    SetupPageTables(&kernel_ptables);
    FCacheL3CacheDisable();
    /* currently only EL1 is supported */
    EnableMmuEl1(&kernel_ptables, flags);

}

static void ArchMemMap(uintptr virt, uintptr phys, fsize_t size, u32 flags)
{
    int ret = AddMap(&kernel_ptables, "dynamic", phys, virt, size, flags);
    if (ret)
    {
        MMU_WRNING("warning AddMap() returned %d", ret);
    }
    else
    {
        AsmInvalidateTlbAll();
    }
}

static fsize_t MemRegionAlign(uintptr *aligned_addr, fsize_t *aligned_size,
                              uintptr addr, fsize_t size, fsize_t align)
{
    fsize_t addr_offset;

    *aligned_addr = rounddown(addr, align) ;
    addr_offset   = addr - *aligned_addr  ;
    *aligned_size = roundup(size +, align);

    return addr_offset;
}

/**
 * @name: FSetTlbAttributes
 * @msg:  This function sets the memory attributes for a section
 * @param {uintptr} addr is 32-bit address for which the attributes need to be set.
 * @param {fsize_t} size of the mapped memory region in bytes
 * @param {u32} attrib or the specified memory region. fmmu.h contains commonly used memory attributes definitions which can be
 *          utilized for this function.
 * @return {*}
 */
void FSetTlbAttributes(uintptr addr, fsize_t size, u32 attrib)
{
    uintptr_t aligned_phys, addr_offset;
    size_t aligned_size;
    MemRegionAlign(&aligned_phys, &aligned_size,
                   addr, size, CONFIG_MMU_PAGE_SIZE);

    FASSERT_MSG(aligned_size != 0U, "0-length mapping at 0x%lx", aligned_phys);
    FASSERT_MSG(aligned_phys < (aligned_phys + (aligned_size - 1)),
                "wraparound for physical address 0x%lx (size %zu)",
                aligned_phys, aligned_size);

    MMU_DEBUG("addr %p,size %d,aligned_phys %p,aligned_size %d \r\n", addr, size, aligned_phys, aligned_size);

    ArchMemMap(aligned_phys, aligned_phys, aligned_size, attrib);
}

void ArchMemUnmap(uintptr addr, fsize_t size)
{
    int ret = RemoveMap(&kernel_ptables, "dynamic", (uintptr)addr, size);

    if (ret)
    {
        MMU_WRNING("RemoveMap() returned %d", ret);
    }
    else
    {
        AsmInvalidateTlbAll();
    }
}

int ArchPagePhysGet(uintptr virt, uintptr *phys)
{
    u64 par;
    int key;

    __asm__ volatile("at S1E1R, %0"
                     :
                     : "r"(virt));
    ISB();
    par = AARCH64_READ_SYSREG(PAR_EL1);

    if (par & BIT(0))
    {
        return -EFAULT;
    }

    if (phys)
    {
        *phys = par & GENMASK(47, 12);
    }
    return 0;
}