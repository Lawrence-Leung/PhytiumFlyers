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
 * FilePath: fexception.c
 * Date: 2022-02-10 14:53:41
 * LastEditTime: 2022-02-17 17:29:43
 * Description:  This file contains low-level driver functions for the processor exception
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe     2021/7/3     first release
 * 1.1  zhugengyu	 2022/06/05	  add debugging information	
 * 1.2  wangxiaodong 2023/2/23	  delete InterruptDeactivation in FExceptionInterruptHandler function
 */
/***************************** Include Files *********************************/

#include "sdkconfig.h"
#include "fexception.h"
#include "fprintk.h"
#include "faarch32.h"
#include "ftypes.h"
#ifdef CONFIG_USE_BACKTRACE
    #include "backtrace.h"
#endif
#include "finterrupt.h"

/************************** Constant Definitions *****************************/
/* DBGDSCR Register Definitions */
#define DBGDSCR_MOE_POS         (2U)
#define DBGDSCR_MOE_MASK        (0xFUL << DBGDSCR_MOE_POS)

#define DBGDSCR_MOE_BREAKPOINT      (1)
#define DBGDSCR_MOE_BKPT_INSTRUCTION    (3)
#define DBGDSCR_MOE_VECTOR_CATCH    (5)
#define DBGDSCR_MOE_SYNC_WATCHPOINT (10)

/* FSR Register Definitions */
#define FSR_FS_BACKGROUND_FAULT     (0)
#define FSR_FS_ALIGNMENT_FAULT      (1)
#define FSR_FS_DEBUG_EVENT          (2)
#define FSR_FS_ACCESS_FLAG_FAULT_1ST (3) /* 0b00011 */
#define FSR_FS_ACCESS_FLAG_FAULT_2ND (4) /* 0b00110 */
#define FSR_FS_TRANSLATION_FAULT_1ST (5) /* 0b00101 */
#define FSR_FS_TRANSLATION_FAULT_2ND (7) /* 0b00111 */
#define FSR_FS_SYNC_EXTERNAL_ABORT  (8)
#define FSR_FS_PERMISSION_FAULT     (13)
#define FSR_FS_ASYNC_EXTERNAL_ABORT (22)
#define FSR_FS_ASYNC_PARITY_ERROR   (24)
#define FSR_FS_SYNC_PARITY_ERROR    (25)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void FExcDefaultErrorEndHandler(void);

/************************** Variable Definitions *****************************/
static FExcInterruptEndHandler data_abort_handler = FExcDefaultErrorEndHandler;

/*****************************************************************************/
void FExcRegisterDataAbortEndHandler(FExcInterruptEndHandler handler)
{
    if (handler != data_abort_handler)
    {
        data_abort_handler = handler;
    }
}

static void FExcDefaultErrorEndHandler(void)
{
    while (TRUE)
    {

    }
}

static const char *GetDbgdscrMoeString(u32 moe)
{
    switch (moe)
    {
        case DBGDSCR_MOE_BREAKPOINT:
            return "Breakpoint";
        case DBGDSCR_MOE_BKPT_INSTRUCTION:
            return "BKPT Instruction";
        case DBGDSCR_MOE_VECTOR_CATCH:
            return "Vector Catch";
        case DBGDSCR_MOE_SYNC_WATCHPOINT:
            return "Synchronous Watchpoint";
        default:
            return "Unknown";
    }
}

static void DumpDebugEvent(void)
{
    /* Read and parse debug mode of entry */
    u32 dbgdscr;
    u32 moe;

    dbgdscr = AARCH32_READ_SYSREG_32(DBGDSCR);

    moe = (dbgdscr & DBGDSCR_MOE_MASK) >> DBGDSCR_MOE_POS;

    /* Print debug event information */
    f_printk("Debug Event (%s) \r\n", GetDbgdscrMoeString(moe));
}

static void DumpFault(u32 status, u32 addr)
{
    /*
     * Dump fault status and, if applicable, tatus-specific information.
     * Note that the fault address is only displayed for the synchronous
     * faults because it is unpredictable for asynchronous faults.
     */
    switch (status)
    {
        case FSR_FS_ALIGNMENT_FAULT:
            f_printk("Alignment Fault @ 0x%08x\r\n", addr);
            break;
        case FSR_FS_BACKGROUND_FAULT:
            f_printk("Background Fault @ 0x%08x\r\n", addr);
            break;
        case FSR_FS_PERMISSION_FAULT:
            f_printk("Permission Fault @ 0x%08x\r\n", addr);
            break;
        case FSR_FS_SYNC_EXTERNAL_ABORT:
            f_printk("Synchronous External Abort @ 0x%08x\r\n", addr);
            break;
        case FSR_FS_ASYNC_EXTERNAL_ABORT:
            f_printk("Asynchronous External Abort");
            break;
        case FSR_FS_SYNC_PARITY_ERROR:
            f_printk("Synchronous Parity/ECC Error @ 0x%08x\r\n", addr);
            break;
        case FSR_FS_ASYNC_PARITY_ERROR:
            f_printk("Asynchronous Parity/ECC Error");
            break;
        case FSR_FS_DEBUG_EVENT:
            DumpDebugEvent();
            break;
        case FSR_FS_ACCESS_FLAG_FAULT_1ST:
        case FSR_FS_ACCESS_FLAG_FAULT_2ND:
            f_printk("Access Flag Fault @ 0x%x\r\n", addr);
            break;
        case FSR_FS_TRANSLATION_FAULT_1ST:
        case FSR_FS_TRANSLATION_FAULT_2ND:
            f_printk("Translation Fault @ 0x%x\r\n", addr);
            break;
        default:
            f_printk("Unknown (%u)", status);
    }
}

/**
 * @name: ShowRegister
 * @msg:  this function will show registers of CPU
 * @param {FExcFrame} *regs_p the registers point
 * @return {*}
 * @note:
 */
void ShowRegister(FExcFrame *regs_p)
{
    f_printk("Execption:\r\n");
    f_printk("r00:0x%08x r01:0x%08x r02:0x%08x r03:0x%08x\r\n", regs_p->r0, regs_p->r1, regs_p->r2, regs_p->r3);
    f_printk("r04:0x%08x r05:0x%08x r06:0x%08x r07:0x%08x\r\n", regs_p->r4, regs_p->r5, regs_p->r6, regs_p->r7);
    f_printk("r08:0x%08x r09:0x%08x r10:0x%08x\r\n", regs_p->r8, regs_p->r9, regs_p->r10);
    f_printk("r11 :0x%08x r12 :0x%08x\r\n", regs_p->r11, regs_p->r12);
    f_printk("sp :0x%08x lr :0x%08x pc :0x%08x\r\n", regs_p->sp, regs_p->lr, regs_p->pc);
    f_printk("cpsr:0x%08x\r\n", regs_p->cpsr);
}


void FExceptionInterruptHandler(void *temp)
{
    void *param;
    IrqHandler isr_func;
    extern struct IrqDesc isr_table[];
    int ir;
    ir = (int)temp;
    if (ir == 1023)
    {
        /* Spurious interrupt */
        return;
    }
    isr_func = isr_table[ir].handler;
    if (isr_func)
    {
        /* Interrupt for myself. */
        param = isr_table[ir].param;
        /* turn to interrupt service routine */
        isr_func(ir, param);
    }

}

_WEAK void FiqInterruptHandler(FExcFrame *regs_p)
{
    f_printk("Fiq interrupt:\r\n");
    ShowRegister(regs_p);
#ifdef CONFIG_USE_BACKTRACE
    backtrace(__func__);
#endif
    while (1)
    {
    }
}

_WEAK void SwInterruptHandler(FExcFrame *regs_p)
{
    f_printk("Svc interrupt:\r\n");
    ShowRegister(regs_p);
#ifdef CONFIG_USE_BACKTRACE
    backtrace(__func__);
#endif
    while (1)
    {
    }
}

_WEAK void DataAbortInterruptHandler(FExcFrame *regs_p)
{
    u32 dfsr, dfar;
    u32 fs;

    /* data fault status register */
    dfsr = AARCH32_READ_SYSREG_32(DFSR);

    /* data fault address register*/
    dfar = AARCH32_READ_SYSREG_32(DFAR);

    fs = (dfsr & 0xf) | ((dfsr & (1 << 10)) >> 6);

    f_printk("Data abort interrupt:\r\n");
    f_printk("dfsr %x \r\n", dfsr);
    f_printk("dfar %x \r\n", dfar);
    ShowRegister(regs_p);
    DumpFault(fs, dfar);

#ifdef CONFIG_USE_BACKTRACE
    backtrace(__func__);
#endif

    if (data_abort_handler)
    {
        data_abort_handler();
    }
}

_WEAK void PrefetchAbortInterruptHandler(FExcFrame *regs_p)
{
    u32 ifsr, ifar;
    u32 fs;
    
    ifsr = AARCH32_READ_SYSREG_32(IFSR);
    ifar = AARCH32_READ_SYSREG_32(IFAR);
    fs = (ifsr & 0xf) | ((ifsr & (1 << 10)) >> 6);
    f_printk("prefetch abort:\r\n");
    f_printk("ifsr %x \r\n", ifsr);
    f_printk("ifar %x \r\n", ifar);
    ShowRegister(regs_p);
    DumpFault(fs, ifar);
#ifdef CONFIG_USE_BACKTRACE
    backtrace(__func__);
#endif
    while (1)
    {
    }
}

_WEAK void UndefineInterruptHandler(FExcFrame *regs_p)
{
    f_printk("Undefined instruction:\r\n");
    ShowRegister(regs_p);

#ifdef CONFIG_USE_BACKTRACE
    backtrace(__func__);
#endif
    while (1)
    {
    }
}
