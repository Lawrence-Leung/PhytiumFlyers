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
 * FilePath: fmmu.h
 * Date: 2022-02-10 14:53:41
 * LastEditTime: 2022-02-17 17:33:43
 * Description:  This file provides APIs for enabling/disabling MMU and setting the memory
 * attributes for sections, in the MMU translation table.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * 1.0   huanghe     2021/7/3     first release
 */


#ifndef FMMU_H
#define FMMU_H

/***************************** Include Files *********************************/

#include "fprintk.h"
#include "ftypes.h"
#include "sdkconfig.h"

#ifdef __cplusplus
extern "C"
{
#endif

/***************** Macros (Inline Functions) Definitions *********************/

/* 对外提供接口 */

/* Following Memory types supported through MAIR encodings can be passed
 * by user through "attrs"(attributes) field of specified memory region.
 * As MAIR supports such 8 encodings, we will reserve attrs[2:0];
 * so that we can provide encodings upto 7 if needed in future.
 */
#define MT_TYPE_MASK 0x7U
#define MT_TYPE(attr) (attr & MT_TYPE_MASK)
#define MT_DEVICE_NGNRNE 0U
#define MT_DEVICE_NGNRE 1U
#define MT_DEVICE_GRE 2U
#define MT_NORMAL_NC 3U
#define MT_NORMAL 4U
#define MT_NORMAL_WT 5U

#define MEMORY_ATTRIBUTES ((0x00 << (MT_DEVICE_NGNRNE * 8)) | \
                           (0x04 << (MT_DEVICE_NGNRE * 8)) |  \
                           (0x0c << (MT_DEVICE_GRE * 8)) |    \
                           (0x44 << (MT_NORMAL_NC * 8)) |     \
                           (0xffUL << (MT_NORMAL * 8)) |      \
                           (0xbbUL << (MT_NORMAL_WT * 8)))

/* More flags from user's perpective are supported using remaining bits
 * of "attrs" field, i.e. attrs[31:3], underlying code will take care
 * of setting PTE fields correctly.
 *
 * current usage of attrs[31:3] is:
 * attrs[3] : Access Permissions
 * attrs[4] : Memory access from secure/ns state
 * attrs[5] : Execute Permissions privileged mode (PXN)
 * attrs[6] : Execute Permissions unprivileged mode (UXN)
 * attrs[7] : Mirror RO/RW permissions to EL0
 * attrs[8] : Overwrite existing mapping if any
 *
 */
#define MT_PERM_SHIFT 3U    /* Selects between read-only and read/write access */
#define MT_SEC_SHIFT 4U     /* Non-secure bit. For memory accesses from Secure state, specifies whether the output address is in the Secure or Non-secure address map */
#define MT_P_EXECUTE_SHIFT 5U /* The Privileged execute-never bit. Determines whether the region is executable at EL1 */
#define MT_U_EXECUTE_SHIFT 6U /*  */
#define MT_RW_AP_SHIFT 7U   /* Selects between Application level (EL0) control and the higher Exception level control. */
#define MT_NO_OVERWRITE_SHIFT 8U

#define MT_RO (0U << MT_PERM_SHIFT) /* Selects read-only access */
#define MT_RW (1U << MT_PERM_SHIFT) /* Selects read/write access */

#define MT_RW_AP_EL_HIGHER (0U << MT_RW_AP_SHIFT) /* EL0 can't access */
#define MT_RW_AP_ELX (1U << MT_RW_AP_SHIFT)       /* EL0 can access */

#define MT_SECURE (0U << MT_SEC_SHIFT)  /* Access the Secure PA space. */
#define MT_NS (1U << MT_SEC_SHIFT)      /* Access the Non-secure PA space. */

#define MT_P_EXECUTE (0U << MT_P_EXECUTE_SHIFT)
#define MT_P_EXECUTE_NEVER (1U << MT_P_EXECUTE_SHIFT)

#define MT_U_EXECUTE (0U << MT_U_EXECUTE_SHIFT)
#define MT_U_EXECUTE_NEVER (1U << MT_U_EXECUTE_SHIFT)

#define MT_NO_OVERWRITE (1U << MT_NO_OVERWRITE_SHIFT)

#define MT_P_RW_U_RW (MT_RW | MT_RW_AP_ELX | MT_P_EXECUTE_NEVER | MT_U_EXECUTE_NEVER)
#define MT_P_RW_U_NA (MT_RW | MT_RW_AP_EL_HIGHER | MT_P_EXECUTE_NEVER | MT_U_EXECUTE_NEVER)
#define MT_P_RO_U_RO (MT_RO | MT_RW_AP_ELX | MT_P_EXECUTE_NEVER | MT_U_EXECUTE_NEVER)
#define MT_P_RO_U_NA (MT_RO | MT_RW_AP_EL_HIGHER | MT_P_EXECUTE_NEVER | MT_U_EXECUTE_NEVER)
#define MT_P_RO_U_RX (MT_RO | MT_RW_AP_ELX | MT_P_EXECUTE_NEVER | MT_U_EXECUTE)
#define MT_P_RX_U_RX (MT_RO | MT_RW_AP_ELX | MT_P_EXECUTE | MT_U_EXECUTE)
#define MT_P_RX_U_NA (MT_RO | MT_RW_AP_EL_HIGHER | MT_P_EXECUTE | MT_U_EXECUTE_NEVER)


/*
 * PTE descriptor can be Block descriptor or Table descriptor
 * or Page descriptor.
 */
#define PTE_DESC_TYPE_MASK 3U
#define PTE_BLOCK_DESC 1U
#define PTE_TABLE_DESC 3U
#define PTE_PAGE_DESC 3U
#define PTE_INVALID_DESC 0U

/*
 * Block and Page descriptor attributes fields
 */
#define PTE_BLOCK_DESC_MEMTYPE(x) (x << 2)
#define PTE_BLOCK_DESC_NS (1ULL << 5)
#define PTE_BLOCK_DESC_AP_ELx (1ULL << 6)
#define PTE_BLOCK_DESC_AP_EL_HIGHER (0ULL << 6)
#define PTE_BLOCK_DESC_AP_RO (1ULL << 7)
#define PTE_BLOCK_DESC_AP_RW (0ULL << 7)
#define PTE_BLOCK_DESC_NON_SHARE (0ULL << 8)
#define PTE_BLOCK_DESC_OUTER_SHARE (2ULL << 8)
#define PTE_BLOCK_DESC_INNER_SHARE (3ULL << 8)
#define PTE_BLOCK_DESC_AF (1ULL << 10)
#define PTE_BLOCK_DESC_NG (1ULL << 11)
#define PTE_BLOCK_DESC_PXN (1ULL << 53)
#define PTE_BLOCK_DESC_UXN (1ULL << 54)

/* Convenience macros to represent the ARMv8-A-specific
* configuration for memory access permission and
* cache-ability attribution.
*/

#define MMU_REGION_ENTRY(_name, _base_pa, _base_va, _size, _attrs) \
    {                                                              \
        .name = _name,                                             \
                .base_pa = _base_pa,                                       \
                           .base_va = _base_va,                                       \
                                      .size = _size,                                             \
                                              .attrs = _attrs,                                           \
    }

#define MMU_REGION_FLAT_ENTRY(name, adr, sz, attrs) \
    MMU_REGION_ENTRY(name, adr, adr, sz, attrs)

/*  模块内配置 */
#define CONFIG_MMU_PAGE_SIZE 0x1000 /* log2 resualt is 12 */


#ifdef CONFIG_MMU_DEBUG_PRINTS
/* To dump page table entries while filling them, set DUMP_PTE macro */
#define DUMP_PTE 1
#define MMU_DEBUG(fmt, ...) f_printk(fmt, ##__VA_ARGS__)
#else
#define MMU_DEBUG(...)
#endif

#define MMU_WRNING(fmt, ...) f_printk(fmt, ##__VA_ARGS__)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) \
    ((long)((sizeof(array) / sizeof((array)[0]))))
#endif

/* KB, MB, GB */
#define KB(x) ((x) << 10)
#define MB(x) (KB(x) << 10)
#define GB(x) (MB(x) << 10)

/**************************** Type Definitions *******************************/

/* Region definition data structure */
struct ArmMmuRegion
{
    /* Region Base Physical Address */
    uintptr_t base_pa;
    /* Region Base Virtual Address */
    uintptr_t base_va;
    /* Region size */
    size_t size;
    /* Region Name */
    const char *name;
    /* Region Attributes */
    uint32_t attrs;
};

/* MMU configuration data structure */
struct ArmMmuConfig
{
    /* Number of regions */
    unsigned int num_regions;
    /* Regions */
    const struct ArmMmuRegion *mmu_regions;
};

struct ArmMmuPtables
{
    uint64_t *base_xlat_table;
};

/* Reference to the MMU configuration.
*
* This struct is defined and populated for each SoC (in the SoC definition),
* and holds the build-time configuration information for the fixed MMU
* regions enabled during kernel initialization.
*/
extern const struct ArmMmuConfig mmu_config;

void MmuInit(void);

void FSetTlbAttributes(uintptr addr, fsize_t size, u32 attrib);

#ifdef __cplusplus
}
#endif

#endif
