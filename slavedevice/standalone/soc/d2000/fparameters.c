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
 * FilePath: parameters.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:58:45
 * Description:  This file is for
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */

#include "fmmu.h"
#include "ftypes.h"
#include "sdkconfig.h"
#include "fparameters.h"

#ifdef CONFIG_TARGET_ARMV8_AARCH64

static const struct ArmMmuRegion mmu_ranges[] =
{

    MMU_REGION_FLAT_ENTRY("DEVICE_REGION",
                          0x00, 0x40000000,
                          MT_DEVICE_NGNRE | MT_P_RW_U_RW | MT_NS),

    MMU_REGION_FLAT_ENTRY("PCIE_CONFIG_REGION",
                          0x40000000, 0x10000000,
                          MT_DEVICE_NGNRNE | MT_RW | MT_NS),

    MMU_REGION_FLAT_ENTRY("PCIE_REGION",
                          0x50000000, 0x30000000,
                          MT_DEVICE_NGNRE | MT_RW | MT_NS),

    MMU_REGION_FLAT_ENTRY("DDR_2G_REGION",
                            0x80000000, 0x80000000,
                            MT_NORMAL | MT_RW | MT_NS),

    MMU_REGION_FLAT_ENTRY("PCIE_REGION",
                          0x1000000000, 0x1000000000,
                          MT_DEVICE_NGNRE | MT_RW | MT_NS),

    MMU_REGION_FLAT_ENTRY("DDR_EXTEND_REGION",
                          0x2000000000, 0x2000000000,
                          MT_NORMAL | MT_RW | MT_NS),

};

const uint32_t mmu_regions_size = ARRAY_SIZE(mmu_ranges);

const struct ArmMmuConfig mmu_config =
{
    .num_regions = mmu_regions_size,
    .mmu_regions = mmu_ranges,
};

#else

#define DDR_MEM NORMAL_MEM

struct mem_desc platform_mem_desc[] =
{
    {
        0x80000000,
        0xFFFFFFFF,
        0x80000000,
        DDR_MEM
    },
    {
        0, //< QSPI
        0x1FFFFFFF,
        0,
        DEVICE_MEM
    },
    {
        0x20000000, //<! LPC
        0x27FFFFFF,
        0x20000000,
        DEVICE_MEM
    },
    {
        FDEV_BASE_ADDR, //<! Device register
        FDEV_END_ADDR,
        FDEV_BASE_ADDR,
        DEVICE_MEM
    },
    {
        0x30000000, //<! debug
        0x39FFFFFF,
        0x30000000,
        DEVICE_MEM
    },
    {
        0x3A000000, //<! Internal register space in the on-chip network
        0x3AFFFFFF,
        0x3A000000,
        DEVICE_MEM
    },
    {
        0x40000000,
        0x7FFFFFFF,
        0x40000000,
        DEVICE_MEM
    },
};

const u32 platform_mem_desc_size = sizeof(platform_mem_desc) / sizeof(platform_mem_desc[0]);

#endif

/**
 * @name: GetCpuMaskToAffval
 * @msg:  Convert information in cpu_mask to cluster_ID and target_list
 * @param {u32} *cpu_mask is each bit of cpu_mask represents a selected CPU, for example, 0x3 represents core0 and CORE1 .
 * @param {u32} *cluster_id is information about the cluster in which core resides ,format is
 * |--------[bit31-24]-------[bit23-16]-------------[bit15-8]-----------[bit7-0]
 * |--------Affinity level3-----Affinity level2-----Affinity level1-----Affinity level0
 * @param {u32} *target_list  is core mask in cluster
 * @return {u32} 0 indicates that the conversion was not successful , 1 indicates that the conversion was successful
 */
u32 GetCpuMaskToAffval(u32 *cpu_mask, u32 *cluster_id, u32 *target_list)
{
    if (*cpu_mask == 0)
    {
        return 0;
    }

    *target_list = 0;
    *cluster_id = 0;

    if (*cpu_mask & 0x3)
    {
        if ((*cpu_mask & 0x3) == 0x3)
        {
            *target_list = 3;
        }
        else if ((*cpu_mask & 0x1))
        {
            *target_list = 1;
        }
        else
        {
            *target_list = 2;
        }
        *cpu_mask &= ~0x3;
    }
    else if (*cpu_mask & 0xc)
    {
        *cluster_id = 0x100;
        if ((*cpu_mask & 0xc) == 0xc)
        {
            *target_list = 3;
        }
        else if ((*cpu_mask & 0x4))
        {
            *target_list = 1;
        }
        else
        {
            *target_list = 2;
        }
        *cpu_mask &= ~0xc;
    }
    else if (*cpu_mask & 0x30)
    {
        *cluster_id = 0x200;
        if ((*cpu_mask & 0x30) == 0x30)
        {
            *target_list = 3;
        }
        else if ((*cpu_mask & 0x10))
        {
            *target_list = 1;
        }
        else
        {
            *target_list = 2;
        }
        *cpu_mask &= ~0x30;
    }
    else if (*cpu_mask & 0xc0)
    {
        *cluster_id = 0x300;
        if ((*cpu_mask & 0xc0) == 0xc0)
        {
            *target_list = 3;
        }
        else if ((*cpu_mask & 0x40))
        {
            *target_list = 1;
        }
        else
        {
            *target_list = 2;
        }
        *cpu_mask &= ~0xc0;
    }
    else
    {
        *cpu_mask = 0;
        return 0;
    }

    return 1;
}

u64 GetMainCpuAffval(void)
{
    return 0;
}