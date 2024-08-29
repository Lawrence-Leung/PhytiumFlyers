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
 * LastEditTime: 2022-02-17 17:32:48
 * Description:  This file contains low-level driver functions for the processor exception
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2021/7/3     first release
 * 1.1  zhugengyu	2022/06/03		add debugging information	
 * 1.2  wangxiaodong 2023/2/23	  delete InterruptDeactivation in FExceptionInterruptHandler function
 */


#include "fexception.h"
#include "fdebug.h"
#include "fprintk.h"
#include "fkernel.h"
#include "faarch64.h"
#include "finterrupt.h"
#include "sdkconfig.h"
#ifdef CONFIG_USE_BACKTRACE
    #include "backtrace.h"
#endif

#include "fexception.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void FExcDefaultErrorEndHandler(void);

/************************** Variable Definitions *****************************/
static FExcInterruptEndHandler sync_end_handler = FExcDefaultErrorEndHandler;
static FExcInterruptEndHandler serr_end_handler = FExcDefaultErrorEndHandler;

/*****************************************************************************/
void FExcRegisterSyncEndHandler(FExcInterruptEndHandler handler)
{
    if (handler != sync_end_handler)
    {
        sync_end_handler = handler;
    }
}

void FExcRegisterSerrEndHandler(FExcInterruptEndHandler handler)
{
    if (handler != serr_end_handler)
    {
        serr_end_handler = handler;
    }
}

static void FExcDefaultErrorEndHandler(void)
{
    while (TRUE)
    {

    }
}

static void FExcShowRegister(FExcFrame *regs)
{
    f_printk("Execption:\r\n");
    f_printk("x00:0x%llx x01:0x%llx x02:0x%llx x03:0x%llx\r\n", regs->x0, regs->x1, regs->x2, regs->x3);
    f_printk("x04:0x%llx x05:0x%llx x06:0x%llx x07:0x%llx\r\n", regs->x4, regs->x5, regs->x6, regs->x7);
    f_printk("x08:0x%llx r09:0x%llx x10:0x%llx x11:0x%llx\r\n", regs->x8, regs->x9, regs->x10, regs->x11);
    f_printk("x12:0x%llx x13:0x%llx x14:0x%llx x15:0x%llx\r\n", regs->x12, regs->x13, regs->x14, regs->x15);
    f_printk("x16:0x%llx x17:0x%llx x18:0x%llx x19:0x%llx\r\n", regs->x16, regs->x17, regs->x18, regs->x19);
    f_printk("x29:0x%llx x30:0x%llx\r\n", regs->x29, regs->x30);
    f_printk("CPACR_EL1 0x%x\r\n", regs->cpacr);
    f_printk("SPSR_EL1 0x%x\r\n", regs->pstate);
    f_printk("ELR_EL1 0x%x\r\n", regs->elr);
    f_printk("SP 0x%x\r\n", regs->sp);
}

static void FExcPrintEcCause(u32 ec)
{

    switch (ec)
    {
        case 0b000000:
            f_printk("Unknown reason");
            break;
        case 0b000001:
            f_printk("Trapped WFI or WFE instruction execution");
            break;
        case 0b000011:
            f_printk("Trapped MCR or MRC access with (coproc==0b1111) that "
                     "is not reported using EC 0b000000");
            break;
        case 0b000100:
            f_printk("Trapped MCRR or MRRC access with (coproc==0b1111) "
                     "that is not reported using EC 0b000000");
            break;
        case 0b000101:
            f_printk("Trapped MCR or MRC access with (coproc==0b1110)");
            break;
        case 0b000110:
            f_printk("Trapped LDC or STC access");
            break;
        case 0b000111:
            f_printk("Trapped access to SVE, Advanced SIMD, or "
                     "floating-point functionality");
            break;
        case 0b001100:
            f_printk("Trapped MRRC access with (coproc==0b1110)");
            break;
        case 0b001101:
            f_printk("Branch Target Exception");
            break;
        case 0b001110:
            f_printk("Illegal Execution state");
            break;
        case 0b010001:
            f_printk("SVC instruction execution in AArch32 state");
            break;
        case 0b011000:
            f_printk("Trapped MSR, MRS or System instruction execution in "
                     "AArch64 state, that is not reported using EC "
                     "0b000000, 0b000001 or 0b000111");
            break;
        case 0b011001:
            f_printk("Trapped access to SVE functionality");
            break;
        case 0b100000:
            f_printk("Instruction Abort from a lower Exception level, that "
                     "might be using AArch32 or AArch64");
            break;
        case 0b100001:
            f_printk("Instruction Abort taken without a change in Exception "
                     "level.");
            break;
        case 0b100010:
            f_printk("PC alignment fault exception.");
            break;
        case 0b100100:
            f_printk("Data Abort from a lower Exception level, that might "
                     "be using AArch32 or AArch64");
            break;
        case 0b100101:
            f_printk("Data Abort taken without a change in Exception level");
            break;
        case 0b100110:
            f_printk("SP alignment fault exception");
            break;
        case 0b101000:
            f_printk("Trapped floating-point exception taken from AArch32 "
                     "state");
            break;
        case 0b101100:
            f_printk("Trapped floating-point exception taken from AArch64 "
                     "state.");
            break;
        case 0b101111:
            f_printk("SError interrupt");
            break;
        case 0b110000:
            f_printk("Breakpoint exception from a lower Exception level, "
                     "that might be using AArch32 or AArch64");
            break;
        case 0b110001:
            f_printk("Breakpoint exception taken without a change in "
                     "Exception level");
            break;
        case 0b110010:
            f_printk("Software Step exception from a lower Exception level, "
                     "that might be using AArch32 or AArch64");
            break;
        case 0b110011:
            f_printk("Software Step exception taken without a change in "
                     "Exception level");
            break;
        case 0b110100:
            f_printk("Watchpoint exception from a lower Exception level, "
                     "that might be using AArch32 or AArch64");
            break;
        case 0b110101:
            f_printk("Watchpoint exception taken without a change in "
                     "Exception level.");
            break;
        case 0b111000:
            f_printk("BKPT instruction execution in AArch32 state");
            break;
        case 0b111100:
            f_printk("BRK instruction execution in AArch64 state.");
            break;
        default:
            f_printk("Error %x is unknow ", ec);
            break;
    }

    f_printk("\r\n");
}

static void FExcPrintIssDfsc(u32 dfsc)
{
    switch (dfsc)
    {
        case 0b00000:
            f_printk("Address size fault, level 0 of translation or translation table base register");
            break;
        case 0b000001:
            f_printk("Address size fault, level 1");
            break;
        case 0b000010:
            f_printk("Address size fault, level 2");
            break;
        case 0b000011:
            f_printk("Address size fault, level 3");
            break;
        case 0b000100:
            f_printk("Translation fault, level 0");
            break;
        case 0b000101:
            f_printk("Translation fault, level 1");
            break;
        case 0b000110:
            f_printk("Translation fault, level 2");
            break;
        case 0b000111:
            f_printk("Translation fault, level 3");
            break;
        case 0b001001:
            f_printk("Access flag fault, level 1");
            break;
        case 0b001010:
            f_printk("Access flag fault, level 2");
            break;
        case 0b001011:
            f_printk("Access flag fault, level 3");
            break;
        case 0b001101:
            f_printk("Permission fault, level 1");
            break;
        case 0b001110:
            f_printk("Permission fault, level 2");
            break;
        case 0b001111:
            f_printk("Permission fault, level 3");
            break;
        case 0b010000:
            f_printk("Synchronous external abort, not on translation table walk");
            break;
        case 0b011000:
            f_printk("Synchronous parity or ECC error on memory access, not on translation table walk");
            break;
        case 0b010100:
            f_printk("Synchronous external abort, on translation table walk, level 0");
            break;
        case 0b010101:
            f_printk("Synchronous external abort, on translation table walk, level 1");
            break;
        case 0b010110:
            f_printk("Synchronous external abort, on translation table walk, level 2");
            break;
        case 0b010111:
            f_printk("Synchronous external abort, on translation table walk, level 3");
            break;
        case 0b011100:
            f_printk("Synchronous parity or ECC error on memory access on translation table walk, level 0");
            break;
        case 0b011101:
            f_printk("Synchronous parity or ECC error on memory access on translation table walk, level 1");
            break;
        case 0b011110:
            f_printk("Synchronous parity or ECC error on memory access on translation table walk, level 2");
            break;
        case 0b011111:
            f_printk("Synchronous parity or ECC error on memory access on translation table walk, level 3");
            break;
        case 0b100001:
            f_printk("Alignment fault");
            break;
        case 0b110000:
            f_printk("TLB conflict abort");
            break;
        case 0b110001:
            f_printk("Unsupported atomic hardware update fault, if the implementation includes ARMv8.1-TTHM. Otherwise reserved.");
            break;
        case 0b110100:
            f_printk("IMPLEMENTATION DEFINED fault (Lockdown)");
            break;
        case 0b110101:
            f_printk("IMPLEMENTATION DEFINED fault (Unsupported Exclusive or Atomic access)");
            break;
        case 0b111101:
            f_printk("Section Domain Fault, used only for faults reported in the PAR_EL1");
            break;
        case 0b111110:
            f_printk("Page Domain Fault, used only for faults reported in the PAR_EL1");
            break;
        default:
            f_printk("Error %x is unknow ", dfsc);
            break;
    }

    f_printk("\r\n");
}
void SynchronousInterrupt(FExcFrame *exc)
{
    u64 esr, far;
    u64 ec, iss;
    f_printk("\r\n Synchronous exception:\r\n");
    esr = AARCH64_READ_SYSREG(esr_el1);
    far = AARCH64_READ_SYSREG(far_el1);
    ec = (esr >> 26) & 0x3f;
    iss = (esr >> 0) & 0x1ffffff;

    f_printk("\r\n Synchronous exception detected, ec:0x%x iss:0x%x far:0x%x\r\n", ec, iss, far);
    FExcPrintEcCause(ec);
    FExcShowRegister(exc);

#ifdef CONFIG_USE_BACKTRACE
    backtrace(__func__);
#endif

    if (sync_end_handler)
    {
        sync_end_handler();
    }
}

void SErrorInterrupt(FExcFrame *exc)
{
    u64 esr, far;
    u64 ec, iss;
    f_printk("\r\n SError exception:\r\n");
    esr = AARCH64_READ_SYSREG(esr_el1);
    far = AARCH64_READ_SYSREG(far_el1);
    ec = (esr >> 26) & 0x3f;
    iss = (esr >> 0) & 0x1ffffff;
    f_printk("\r\n SError exception detected, ec:0x%x iss:0x%x far:0x%x\r\n", ec, iss, far);
    FExcPrintEcCause(ec);
    FExcShowRegister(exc);

#ifdef CONFIG_USE_BACKTRACE
    backtrace(__func__);
#endif

    if (serr_end_handler)
    {
        serr_end_handler();
    }
}

void FExceptionInterruptHandler(void *temp)
{
    void *param;
    long ir;
    extern struct IrqDesc isr_table[];
    ir = (long)temp;

    if (ir == 1023)
    {
        /* Spurious interrupt */
        return;
    }

    /* get interrupt service routine */
    if (isr_table[ir].handler)
    {
        /* Interrupt for myself. */
        param = isr_table[ir].param;
        /* turn to interrupt service routine */
        isr_table[ir].handler(ir, param);
    }
    else
    {
        f_printk("no handler is exist \r\n");
    }

}

_WEAK void FIQInterrupt(void *value)
{
    FASSERT(0);
    (void)value;
    while (1)
    {
    }
}