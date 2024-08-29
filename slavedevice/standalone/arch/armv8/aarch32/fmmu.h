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
 * LastEditTime: 2022-02-17 17:30:29
 * Description:  This file is for
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */


#ifndef ARCH_AARCH32_MMU_H
#define ARCH_AARCH32_MMU_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "ftypes.h"

/* mmu Short-descriptor translation table section(TTB)  */
#define  TTB_SETC_SIZE (1024*1024)
#define  TTB_SETC_SIZE_MASK (~(TTB_SETC_SIZE - 1UL))


#define MT_XN (1 << 4)                    /* eXecute Never */
#define MT_NS (1 << 19)
#define MT_NG (1 << 17)

#define AP_0_1_00   (0<<10)
#define AP_0_1_01   (1<<10)
#define AP_0_1_10   (2<<10)
#define AP_0_1_11   (3<<10)
#define AP_2_0      (0<<15)
#define AP_2_1      (1<<15)

/* only SCTRL.AFE == 0 */
#define MT_P_NA_U_NA   (AP_0_1_00 | AP_2_0)   /* PL1 access=NA, user=NA */
#define MT_P_RW_U_NA   (AP_0_1_01 | AP_2_0)   /* PL1 access=RW, user=NA */
#define MT_P_RW_U_RO   (AP_0_1_10 | AP_2_0)   /* PL1 access=RW, user=RO */
#define MT_P_RW_U_RW   (AP_0_1_11 | AP_2_0)   /* PL1 access=RW, user=RW */

#define MT_P_RO_U_NA   (AP_0_1_01 | AP_2_1)   /* PL1 access=RO, user=NA */
#define MT_P_RO_U_RO   (AP_0_1_11 | AP_2_1)   /* PL1 access=RO, user=RO */

#define MT_SHAREABLE (1 << 16)                  /* shareable */

/* only SCTRL.TRE == 0 ,Close TEX  */
#define TREN_TEX2_0_000 (0<<12)
#define TREN_TEX2_0_001 (1<<12)
#define TREN_TEX2_0_010 (2<<12)
#define TREN_TEX2_0_011 (3<<12)
#define TREN_TEX2_0_111 (7<<12)
#define TREN_TEX2_0_110 (6<<12)
#define TREN_TEX2_0_101 (5<<12)
#define TREN_TEX2_0_100 (4<<12)

#define CACHEABLE (1<<3)
#define BUFFERABLE (1<<2)

#define MT_DEVICE_NGNRNE   (TREN_TEX2_0_000)             /* Device-nGnRnE , Outer Shareable */
#define MT_DEVICE_NGNRE    (TREN_TEX2_0_000|BUFFERABLE)  /* Device-nGnRE , Outer Shareable */
#define MT_DEVICE_NGNRE_2  (TREN_TEX2_0_010)             /* Device-nGnRE , Outer Shareable */

#define MT_NORMAL_WT       (TREN_TEX2_0_000|CACHEABLE|MT_SHAREABLE) /* Outer and Inner Write-Through, Read-Allocate No Write-Allocate , Outer Shareable  */

#define MT_NORMAL_WB_WCN   (TREN_TEX2_0_000|CACHEABLE|BUFFERABLE|MT_SHAREABLE) /* Outer and Inner Write-Back, Read-Allocate No Write-Allocate , Outer Shareable  */

#define MT_NORMAL_NC       (TREN_TEX2_0_001|MT_SHAREABLE) /* Outer and Inner Non-cacheable , Outer Shareable ,  For compatibility with ARMv7 software should set SHAREABLE to 1 , Outer Shareable */
#define MT_NORMAL          (TREN_TEX2_0_001|BUFFERABLE|CACHEABLE|MT_SHAREABLE) /* Outer and Inner Write-Back, Read-Allocate Write-Allocate , Outer Shareable , Outer Shareable  */

#define DOMAIN_FAULT  (0x0)
#define DOMAIN_CHK    (0x1)
#define DOMAIN_NOTCHK (0x3)
#define DOMAIN0       (0x0 << 5)
#define DOMAIN1       (0x1 << 5)

#define DOMAIN0_ATTR (DOMAIN_CHK << 0)
#define DOMAIN1_ATTR (DOMAIN_FAULT << 2)

/* device mapping type */
#define DEVICE_MEM (MT_DEVICE_NGNRNE | MT_P_RW_U_RW | MT_XN)
/* normal memory mapping type */
#define NORMAL_MEM (MT_NORMAL_WB_WCN|MT_P_RW_U_RW)

struct mem_desc
{
    u32 vaddr_start;
    u32 vaddr_end;
    u32 paddr_start;
    u32 attr;
};


void FSetTlbAttributes(uintptr addr, fsize_t size, u32 attrib);
void CpuMmuDisable(void);
void CpuMmuEnable(void);
void CpuTlbSet(volatile unsigned long *addr);


#ifdef __cplusplus
}
#endif

#endif