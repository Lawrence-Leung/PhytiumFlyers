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
 * FilePath: finterrupt.h
 * Date: 2021-06-25 14:31:02
 * LastEditTime: 2022-02-18 08:24:27
 * Description:  This file is for interrupt functionality related apis
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2021/4/1       init commit
 */


#ifndef FINTERRUPT_H
#define FINTERRUPT_H

#include "ftypes.h"
#include "ferror_code.h"
#include "sdkconfig.h"
#include "fparameters.h"
#ifdef CONFIG_USE_GIC
    #include "fgic.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#define FINT_SUCCESS    FT_SUCCESS
#define FINT_SET_TARGET_ERR   FT_MAKE_ERRCODE(ErrorModGeneral, ErrInterrupt, 1)
#define FINT_INT_NUM_NOT_FIT   FT_MAKE_ERRCODE(ErrorModGeneral, ErrInterrupt, 2) /* Incorrect interrupt number usage */


#define INTERRUPT_DRV_INTS_ID  0
typedef FGic InterruptDrvType;

typedef enum
{
    INTERRUPT_ROLE_MASTER = 0,  /* The current core exists as the main core and automatically initializes all interrupt driver components when initializing interrupts   */
    INTERRUPT_ROLE_SLAVE,       /* The current core exists as a subordinate core and automatically initializes the necessary interrupt driver components when initializing interrupts   */
} INTERRUPT_ROLE_SELECT;

/*
 * Interrupt handler definition
 */
typedef void (*IrqHandler)(s32 vector, void *param);

struct IrqDesc
{
    IrqHandler handler;
    void *param;
};

#define INTERRUPT_CPU_ALL_SELECT 0xffffffffffffffffULL
#define INTERRUPT_CPU_TARGET_ALL_SET 0xffffffffUL


#define IRQ_MODE_TRIG_LEVEL (0x00) /* Trigger: level triggered interrupt */
#define IRQ_MODE_TRIG_EDGE (0x01)  /* Trigger: edge triggered interrupt */

#define IRQ_GROUP_PRIORITY_3    3 /* group priority valid mask is bit[7:3],subpriority valid mask is bit[4:0] */
#define IRQ_GROUP_PRIORITY_4    4 /* group priority valid mask is bit[7:4],subpriority valid mask is bit[3:0] */
#define IRQ_GROUP_PRIORITY_5    5 /* group priority valid mask is bit[7:5],subpriority valid mask is bit[4:0] */
#define IRQ_GROUP_PRIORITY_6    6 /* group priority valid mask is bit[7:6],subpriority valid mask is bit[5:0] */
#define IRQ_GROUP_PRIORITY_7    7 /* group priority valid mask is bit[7],subpriority valid mask is bit[6:0] */


#define IRQ_PRIORITY_OFFSET     4   /* implemented priority bit offset  */
#define IRQ_PRIORITY_VALUE_0    0x0
#define IRQ_PRIORITY_VALUE_1    0x1
#define IRQ_PRIORITY_VALUE_2    0x2
#define IRQ_PRIORITY_VALUE_3    0x3
#define IRQ_PRIORITY_VALUE_4    0x4
#define IRQ_PRIORITY_VALUE_5    0x5
#define IRQ_PRIORITY_VALUE_6    0x6
#define IRQ_PRIORITY_VALUE_7    0x7
#define IRQ_PRIORITY_VALUE_8    0x8
#define IRQ_PRIORITY_VALUE_9    0x9
#define IRQ_PRIORITY_VALUE_10   0xa
#define IRQ_PRIORITY_VALUE_11   0xb
#define IRQ_PRIORITY_VALUE_12   0xc
#define IRQ_PRIORITY_VALUE_13   0xd
#define IRQ_PRIORITY_VALUE_14   0xe
#define IRQ_PRIORITY_VALUE_15   0xf

#define IRQ_PRIORITY_MASK_0    0
#define IRQ_PRIORITY_MASK_1    0x10
#define IRQ_PRIORITY_MASK_2    0x20
#define IRQ_PRIORITY_MASK_3    0x30
#define IRQ_PRIORITY_MASK_4    0x40
#define IRQ_PRIORITY_MASK_5    0x50
#define IRQ_PRIORITY_MASK_6    0x60
#define IRQ_PRIORITY_MASK_7    0x70
#define IRQ_PRIORITY_MASK_8    0x80
#define IRQ_PRIORITY_MASK_9    0x90
#define IRQ_PRIORITY_MASK_10   0xa0
#define IRQ_PRIORITY_MASK_11   0xb0
#define IRQ_PRIORITY_MASK_12   0xc0
#define IRQ_PRIORITY_MASK_13   0xd0
#define IRQ_PRIORITY_MASK_14   0xe0
#define IRQ_PRIORITY_MASK_15   0xf0

#define PRIORITY_TRANSLATE_SET(x) ((((x)>> 1) | 0x80) & 0xff)

#define PRIORITY_TRANSLATE_GET(x) (((x)<< 1) & 0xff)

void InterruptInit(InterruptDrvType *int_driver_p, u32 instance_id, INTERRUPT_ROLE_SELECT role_select);

void InterruptMask(int int_id);
void InterruptUmask(int int_id);

void InterruptDeactivation(int int_id);
int InterruptGetAck(void);

FError InterruptSetTargetCpus(int int_id, u32 cpu_id);
FError InterruptGetTargetCpus(int int_id, u32 *cpu_p);

void InterruptSetTrigerMode(int int_id, unsigned int mode);
unsigned int InterruptGetTrigerMode(int int_id);

void InterruptSetPriority(int int_id, unsigned int priority);
unsigned int InterruptGetPriority(int int_id);

void InterruptSetPriorityMask(unsigned int priority);
unsigned int InterruptGetPriorityMask(void);

u32 InterruptGetCurrentPriority(void);

void InterruptSetPriorityGroupBits(unsigned int bits);
unsigned int InterruptGetPriorityGroupBits(void);

void InterruptInstall(int int_id, IrqHandler handler,
                      void *param, const char *name);

FError InterruptCoreInterSend(int int_id, u64 cpu_mask);

void InterruptEarlyInit(void);

u8 InterruptGetPriorityConfig(void);

#ifdef __cplusplus
}
#endif

#endif
