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
 * FilePath: fparameters.h
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-17 17:58:51
 * Description:  This file is for
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */

#ifndef D2000_FPARAMETERS_H
#define D2000_FPARAMETERS_H

#ifdef __cplusplus
extern "C"
{
#endif

#if !defined(__ASSEMBLER__)
#include "ftypes.h"
#endif

#define CORE0_AFF 0x0
#define CORE1_AFF 0x1
#define CORE2_AFF 0x100
#define CORE3_AFF 0x101
#define CORE4_AFF 0x200
#define CORE5_AFF 0x201
#define CORE6_AFF 0x300
#define CORE7_AFF 0x301
#define FCORE_NUM 8

/* cache */
#define CACHE_LINE_ADDR_MASK 0x3FUL
#define CACHE_LINE 64U

/*  Device register address */
#define FDEV_BASE_ADDR 0x28000000
#define FDEV_END_ADDR 0x2FFFFFFF


/* PCIE ECAM */

/* Pci express  */
#define FPCIE_ECAM_INSTANCE_NUM 1
#define FPCIE_ECAM_INSTANCE0 0

#define FPCIE_ECAM_MAX_OUTBOUND_NUM 8

/* Bus, Device and Function */
#define FPCIE_ECAM_CFG_MAX_NUM_OF_BUS 256
#define FPCIE_ECAM_CFG_MAX_NUM_OF_DEV 32
#define FPCIE_ECAM_CFG_MAX_NUM_OF_FUN 8

#define FPCIE_ECAM_INTA_IRQ_NUM     60
#define FPCIE_ECAM_INTB_IRQ_NUM     61
#define FPCIE_ECAM_INTC_IRQ_NUM     62
#define FPCIE_ECAM_INTD_IRQ_NUM     63

/*  max scan*/
#define FPCIE_MAX_SCAN_NUMBER 128

/* memory space */
#define FPCI_ECAM_CONFIG_BASE_ADDR 0x40000000        /* ecam */
#define FPCI_ECAM_CONFIG_REG_LENGTH 0x10000000

#define FPCI_ECAM_IO_CONFIG_BASE_ADDR 0x50000000     /* io address space */
#define FPCI_ECAM_IO_CONFIG_REG_LENGTH 0x08000000

#define FPCI_ECAM_MEM32_BASE_ADDR 0x58000000         /* mmio 32 */
#define FPCI_ECAM_MEM32_REG_LENGTH 0x27ffffff

#define FPCI_ECAM_MEM64_BASE_ADDR 0x1000000000       /* mmio 64 */
#define FPCI_ECAM_MEM64_REG_LENGTH 0x1fffffffff

#define FPCI_ECAM_INTX_NEED_EOI     1
#define FPCI_ECAM_INTX_CONTROL_STATUS_REG_NUM 2
#define FPCI_ECAM_INTX_CONTROL_STATUS_REG0 0x29100000
#define FPCI_ECAM_INTX_CONTROL_STATUS_REG1 0x29101000

#define FPCI_ECAM_INTX_CONFIG_ISTATUS_NUM  6
#define FPCI_ECAM_INTX_CONFIG_ISTATUS_REG0 0x29000184
#define FPCI_ECAM_INTX_CONFIG_ISTATUS_REG1 0x29010184
#define FPCI_ECAM_INTX_CONFIG_ISTATUS_REG2 0x29020184
#define FPCI_ECAM_INTX_CONFIG_ISTATUS_REG3 0x29030184
#define FPCI_ECAM_INTX_CONFIG_ISTATUS_REG4 0x29040184
#define FPCI_ECAM_INTX_CONFIG_ISTATUS_REG5 0x29050184


/* platform ahci host */
#define PLAT_AHCI_HOST_MAX_COUNT 5
#define AHCI_BASE_0     0
#define AHCI_BASE_1     0
#define AHCI_BASE_2     0
#define AHCI_BASE_3     0
#define AHCI_BASE_4     0

#define AHCI_IRQ_0      0
#define AHCI_IRQ_1      0
#define AHCI_IRQ_2      0
#define AHCI_IRQ_3      0
#define AHCI_IRQ_4      0

/* UART */
#if !defined(__ASSEMBLER__)
enum
{
    FUART0_ID = 0,
    FUART1_ID,
    FUART2_ID,
    FUART3_ID,

    FUART_NUM
};
#endif

#define FUART0_IRQ_NUM 38
#define FUART0_BASE_ADDR 0x28000000
#define FUART0_CLK_FREQ_HZ 48000000

#define FUART1_IRQ_NUM 39
#define FUART1_BASE_ADDR 0x28001000
#define FUART1_CLK_FREQ_HZ 48000000

#define FUART2_IRQ_NUM 40
#define FUART2_BASE_ADDR 0x28002000
#define FUART2_CLK_FREQ_HZ 48000000

#define FUART3_IRQ_NUM 41
#define FUART3_BASE_ADDR 0x28003000
#define FUART3_CLK_FREQ_HZ 48000000

#define FT_STDOUT_BASE_ADDR FUART1_BASE_ADDR
#define FT_STDIN_BASE_ADDR FUART1_BASE_ADDR

/* QSPI */
#if !defined(__ASSEMBLER__)
enum
{
    FQSPI0_ID = 0,

    FQSPI_NUM
};

/* FQSPI cs 0_3, chip number */
enum
{
    FQSPI_CS_0 = 0,
    FQSPI_CS_1 = 1,
    FQSPI_CS_2 = 2,
    FQSPI_CS_3 = 3,
    FQSPI_CS_NUM
};
#endif

#define FQSPI_BASE_ADDR 0x28014000
#define FQSPI_MEM_START_ADDR 0x0
#define FQSPI_MEM_END_ADDR 0x1FFFFFFF

/* GIC v3 */
#define ARM_GIC_NR_IRQS 1024
#define ARM_GIC_IRQ_START 0
#define FGIC_NUM 1


#define GICV3_BASE_ADDR 0x29a00000U
#define GICV3_DISTRIBUTOR_BASE_ADDR (GICV3_BASE_ADDR + 0)
#define GICV3_RD_BASE_ADDR (GICV3_BASE_ADDR + 0x100000U)
#define GICV3_RD_OFFSET (2U << 16)
#define FT_GICV3_VECTORTABLE_NUM GIC_INT_MAX_NUM

/*
 * The maximum priority value that can be used in the GIC.
 */
#define GICV3_MAX_INTR_PRIO_VAL 240U
#define GICV3_INTR_PRIO_MASK 0x000000f0U

#define ARM_GIC_IPI_COUNT 16 /* MPCore IPI count  */
#define SGI_INT_MAX 16
#define SPI_START_INT_NUM 32 /* SPI start at ID32        */
#define PPI_START_INT_NUM 16 /* PPI start at ID16        */
#define GIC_INT_MAX_NUM 1020 /* GIC max interrupts count */

/* GPIO */
#define FGPIO0_BASE_ADDR (0x28004000)
#define FGPIO1_BASE_ADDR (0x28005000)

#define FGPIO0_ID      0
#define FGPIO1_ID      1
#define FGPIO_NUM      2

#define FGPIO0_IRQ_NUM (42) /* gpio0 irq number */
#define FGPIO1_IRQ_NUM (43) /* gpio1 irq number */

/* IOCTRL */
#define FIOCTRL_BASE_ADDR  0x28180000

#if !defined(__ASSEMBLER__)
/* IOPAD */
enum
{
    FIOCTRL0_ID = 0,

    FIOCTRL_NUM
};
#endif

#define FIOCTRL_INDEX(offset, func_beg) \
    { \
        /* reg_off */   (offset),  \
        /* reg_bit */   (func_beg) \
    }

#define FIOCTRL_CRU_CLK_OBV_PAD           (FIOCtrlPinIndex)FIOCTRL_INDEX(0x200, 24)
#define FIOCTRL_SPI0_CSN0_PAD             (FIOCtrlPinIndex)FIOCTRL_INDEX(0x208, 16)
#define FIOCTRL_SPI0_SCK_PAD              (FIOCtrlPinIndex)FIOCTRL_INDEX(0x208, 12)
#define FIOCTRL_SPI0_SO_PAD               (FIOCtrlPinIndex)FIOCTRL_INDEX(0x208, 8)
#define FIOCTRL_SPI0_SI_PAD               (FIOCtrlPinIndex)FIOCTRL_INDEX(0x208, 4)

#define FIOCTRL_TJTAG_TDI_PAD             (FIOCtrlPinIndex)FIOCTRL_INDEX(0x204, 24) /* can0-tx: func 1 */
#define FIOCTRL_SWDITMS_SWJ_PAD           (FIOCtrlPinIndex)FIOCTRL_INDEX(0x204, 12) /* can0-rx: func 1 */

#define FIOCTRL_NTRST_SWJ_PAD             (FIOCtrlPinIndex)FIOCTRL_INDEX(0x204, 20) /* can1-tx: func 1 */
#define FIOCTRL_SWDO_SWJ_PAD              (FIOCtrlPinIndex)FIOCTRL_INDEX(0x204, 8)  /* can1-rx: func 1 */

#define FIOCTRL_I2C0_SCL_PAD              (FIOCtrlPinIndex)FIOCTRL_INDEX(0x208, 24) /* i2c0-scl: func 0 */
#define FIOCTRL_I2C0_SDA_PAD              (FIOCtrlPinIndex)FIOCTRL_INDEX(0x208, 20) /* i2c0-sda: func 0 */
#define FIOCTRL_ALL_PLL_LOCK_PAD          (FIOCtrlPinIndex)FIOCTRL_INDEX(0x200, 28) /* i2c1-scl: func 2 */
#define FIOCTRL_CRU_CLK_OBV_PAD           (FIOCtrlPinIndex)FIOCTRL_INDEX(0x200, 24) /* i2c1-sda: func 2 */
#define FIOCTRL_SWDO_SWJ_PAD              (FIOCtrlPinIndex)FIOCTRL_INDEX(0x204, 8)  /* i2c2-scl: func 2 */
#define FIOCTRL_TDO_SWJ_IN_PAD            (FIOCtrlPinIndex)FIOCTRL_INDEX(0x204, 4)  /* i2c2-sda: func 2 */
#define FIOCTRL_HDT_MB_DONE_STATE_PAD     (FIOCtrlPinIndex)FIOCTRL_INDEX(0x204, 0)  /* i2c3-scl: func 2 */
#define FIOCTRL_HDT_MB_FAIL_STATE_PAD     (FIOCtrlPinIndex)FIOCTRL_INDEX(0x208, 28) /* i2c3-sda: func 2 */

#define FIOCTRL_UART_2_RXD_PAD            (FIOCtrlPinIndex)FIOCTRL_INDEX(0x210, 0)  /* spi1_csn0: func 1 */
#define FIOCTRL_UART_2_TXD_PAD            (FIOCtrlPinIndex)FIOCTRL_INDEX(0x214, 28) /* spi1_sck: func 1 */
#define FIOCTRL_UART_3_RXD_PAD            (FIOCtrlPinIndex)FIOCTRL_INDEX(0x214, 24) /* spi1_so: func 1 */
#define FIOCTRL_UART_3_TXD_PAD            (FIOCtrlPinIndex)FIOCTRL_INDEX(0x214, 20) /* spi1_si: func 1 */
#define FIOCTRL_QSPI_CSN2_PAD             (FIOCtrlPinIndex)FIOCTRL_INDEX(0x214, 8)  /* spi1_csn1: func 1 */
#define FIOCTRL_QSPI_CSN3_PAD             (FIOCtrlPinIndex)FIOCTRL_INDEX(0x214, 4)  /* spi1_csn2: func 1 */

#define FIOCTRL_LPC_LAD0_PAD              (FIOCtrlPinIndex)FIOCTRL_INDEX(0x218, 12) /* gpio_1_a_3: func 1 */
#define FIOCTRL_LPC_LAD1_PAD              (FIOCtrlPinIndex)FIOCTRL_INDEX(0x218, 8)  /* gpio_1_a_4: func 1 */  

/* SPI */
#define FSPI0_BASE_ADDR 0x2800c000
#define FSPI1_BASE_ADDR 0x28013000
#define FSPI0_ID   0
#define FSPI1_ID   1
#define FSPI_CLK_FREQ_HZ 48000000
#define FSPI_NUM 2
#define FSPI0_IRQ_NUM 50
#define FSPI1_IRQ_NUM 51

/* I2C */
#if !defined(__ASSEMBLER__)
enum
{
    FI2C0_ID = 0,
    FI2C1_ID = 1,
    FI2C2_ID,
    FI2C3_ID,

    FI2C_NUM
};
#endif

#define FI2C0_BASE_ADDR 0x28006000
#define FI2C1_BASE_ADDR 0x28007000
#define FI2C2_BASE_ADDR 0x28008000
#define FI2C3_BASE_ADDR 0x28009000

#define FI2C0_IRQ_NUM 44
#define FI2C1_IRQ_NUM 45
#define FI2C2_IRQ_NUM 46
#define FI2C3_IRQ_NUM 47

#define FI2C_CLK_FREQ_HZ 48000000 /* 48MHz */

/* WDT */
#if !defined(__ASSEMBLER__)
enum
{
    FWDT0_ID = 0,
    FWDT1_ID = 1,

    FWDT_NUM
};
#endif

#define FWDT0_REFRESH_BASE_ADDR 0x2800a000
#define FWDT1_REFRESH_BASE_ADDR 0x28016000

#define FWDT_CONTROL_BASE_ADDR(x) ((x)+0x1000)

#define FWDT0_IRQ_NUM 48
#define FWDT1_IRQ_NUM 49

#define FWDT_CLK_FREQ_HZ 48000000 /* 48MHz */

/* SDCI */
#if !defined(__ASSEMBLER__)
enum
{
    FSDMMC0_ID = 0,

    FSDMMC_NUM
};
#endif

#define FSDMMC0_BASE_ADDR 0x28207C00

#define FSDMMC0_DMA_IRQ_NUM 52
#define FSDMMC0_CMD_IRQ_NUM 53
#define FSDMMC0_ERR_IRQ_NUM 54

#define FSDMMC_CLK_FREQ_HZ 600000000 /* 600 MHz */

/* GMAC */
#define FGMAC_PUB_REG_BASE_ADDR 0x2820B000 /* 公共寄存器基地址 */

#if !defined(__ASSEMBLER__)
enum
{
    FGMAC0_ID = 0,
    FGMAC1_ID,

    FGMAC_NUM
};
#endif

#define FGMAC0_BASE_ADDR 0x2820C000
#define FGMAC1_BASE_ADDR 0x28210000

#define FGMAC0_IRQ_NUM 81
#define FGMAC1_IRQ_NUM 82

#define FGMAC_DMA_MIN_ALIGN 128
#define FGMAC_MAX_PACKET_SIZE 1600

/* rtc base address */
#define RTC_CONTROL_BASE 0x2800D000

#define FT_CPUS_NR FCORE_NUM



/* can */
#define FCAN_CLK_FREQ_HZ 600000000

#define FCAN_REG_LENGTH 0x1000
#define FCAN0_BASE_ADDR 0x28207000
#define FCAN1_BASE_ADDR 0x28207400
#define FCAN2_BASE_ADDR 0x28207800
#define FCAN0_IRQ_NUM 119
#define FCAN1_IRQ_NUM 123
#define FCAN2_IRQNUM 124

#if !defined(__ASSEMBLER__)
enum
{
    FCAN0_ID = 0,
    FCAN1_ID = 1,
    FCAN2_ID = 2,

    FCAN_NUM
};
#endif

/* generic timer */
/* non-secure physical timer int id */
#define GENERIC_TIMER_NS_IRQ_NUM 30U

/* virtual timer int id */
#define GENERIC_VTIMER_IRQ_NUM 27U

#if !defined(__ASSEMBLER__)
enum
{
    GENERIC_TIMER_ID0 = 0, /* non-secure physical timer */
    GENERIC_TIMER_ID1 = 1, /* virtual timer */

    GENERIC_TIMER_NUM
};
#endif

#ifdef __cplusplus
}
#endif

#endif // !