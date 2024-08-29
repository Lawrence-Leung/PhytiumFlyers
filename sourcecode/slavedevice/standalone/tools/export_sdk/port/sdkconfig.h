/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc. 
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
 * FilePath: sdkopts.h
 * Date: 2022-09-16 13:54:28
 * LastEditTime: 2022-09-16 13:54:28
 * Description:  This file is for configure sdkconfig in non-Kconfig way
 * 
 * Modify History: 
 *  Ver     Who           Date                  Changes
 * -----   ------       --------     --------------------------------------
 *  1.0   zhugengyu    2023/03/01            first release
 */

#ifndef SDK_CONFIG_H
#define SDK_CONFIG_H

#include "sdkopts.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CONFIG_USE_BAREMETAL TARGET_NAME

#if (CPU_AARCH == 32)
#define CONFIG_TARGET_ARMV8_AARCH32
#define CONFIG_USE_CACHE
#define CONFIG_USE_MMU
#define CONFIG_USE_AARCH64_L1_TO_AARCH32
#elif (CPU_AARCH == 64)
#define CONFIG_TARGET_ARMV8_AARCH64
#define CONFIG_USE_CACHE
#define CONFIG_USE_MMU
#endif

#if (CPU_TYPE == CPU_TYPE_E2000D)
#define CONFIG_TARGET_E2000
#define CONFIG_TARGET_E2000D
#elif (CPU_TYPE == CPU_TYPE_E2000Q)
#define CONFIG_TARGET_E2000
#define CONFIG_TARGET_E2000Q
#elif (CPU_TYPE == CPU_TYPE_E2000S)
#define CONFIG_TARGET_E2000
#define CONFIG_TARGET_E2000S
#elif (CPU_TYPE == CPU_TYPE_D2000)
#define CONFIG_TARGET_D2000
#elif (CPU_TYPE == CPU_TYPE_FT2004)
#define CONFIG_TARGET_FT2004
#endif

#if (LOG_TYPE == LOG_TYPE_VERBOS)
#define CONFIG_LOG_VERBOS
#elif (LOG_TYPE == LOG_TYPE_DEBUG)
#define CONFIG_LOG_DEBUG
#elif (LOG_TYPE == LOG_TYPE_INFO)
#define CONFIG_LOG_INFO
#elif (LOG_TYPE == LOG_TYPE_WARN)
#define CONFIG_LOG_WARN
#elif (LOG_TYPE == LOG_TYPE_ERROR)
#define CONFIG_LOG_ERROR
#elif (LOG_TYPE == LOG_TYPE_NONE)
#define CONFIG_LOG_NONE
#endif

#define CONFIG_DEFAULT_DEBUG_PRINT_UART1

#define CONFIG_USE_DEFAULT_INTERRUPT_CONFIG
#define CONFIG_INTERRUPT_ROLE_MASTER

#define CONFIG_USE_SPI
#define CONFIG_USE_FSPIM

#define CONFIG_USE_QSPI
#define CONFIG_USE_FQSPI

#define CONFIG_USE_GIC
#define CONFIG_ENABLE_GICV3

#define CONFIG_USE_SERIAL
#define CONFIG_ENABLE_Pl011_UART

#define CONFIG_USE_GPIO
#define CONFIG_ENABLE_FGPIO

#define CONFIG_USE_ETH
#define CONFIG_ENABLE_FXMAC
#define CONFIG_FXMAC_PHY_COMMON

#define CONFIG_USE_CAN
#define CONFIG_USE_FCAN
#define CONFIG_FCAN_USE_CANFD

#define CONFIG_USE_I2C
#define CONFIG_USE_FI2C

#define CONFIG_USE_TIMER
#define CONFIG_ENABLE_TIMER_TACHO

#define CONFIG_USE_MIO
#define CONFIG_ENABLE_MIO

#define CONFIG_USE_SDMMC
#define CONFIG_ENABLE_FSDIO

#define CONFIG_USE_PCIE
#define CONFIG_ENABLE_F_PCIE

#define CONFIG_USE_WDT
#define CONFIG_USE_FWDT

#define CONFIG_USE_DMA
#define CONFIG_ENABLE_FGDMA
#define CONFIG_ENABLE_FDDMA

#define CONFIG_USE_NAND
#define CONFIG_ENABLE_FNAND
#define CONFIG_FNAND_COMMON_DEBUG_EN

#define CONFIG_USE_SATA
#define CONFIG_ENABLE_FSATA

#define CONFIG_USE_USB
#define CONFIG_ENABLE_USB_FXHCI

#define CONFIG_USE_ADC
#define CONFIG_USE_FADC

#define CONFIG_USE_PWM
#define CONFIG_USE_FPWM

#define CONFIG_USE_IPC
#define CONFIG_ENABLE_FSEMAPHORE

#define CONFIG_USE_MEDIA
#define CONFIG_ENABLE_FDC_DP

#define CONFIG_USE_SCMI_MHU
#define CONFIG_ENABLE_SCMI_MHU

#if (CPU_AARCH == 32)
#define CONFIG_AARCH32_RAM_LD
#define CONFIG_LINK_SCRIPT_ROM
#define CONFIG_ROM_START_UP_ADDR 0x80100000
#define CONFIG_ROM_SIZE_MB 1
#define CONFIG_LINK_SCRIPT_RAM
#define CONFIG_RAM_START_UP_ADDR 0x81000000
#define CONFIG_RAM_SIZE_MB 64
#define CONFIG_HEAP_SIZE 2
#define CONFIG_SVC_STACK_SIZE 0x1000
#define CONFIG_SYS_STACK_SIZE 0x1000
#define CONFIG_IRQ_STACK_SIZE 0x1000
#define CONFIG_ABORT_STACK_SIZE 0x1000
#define CONFIG_FIQ_STACK_SIZE 0x1000
#define CONFIG_UNDEF_STACK_SIZE 0x1000
#elif (CPU_AARCH == 64)
#define CONFIG_AARCH64_RAM_LD
#define CONFIG_LINK_SCRIPT_ROM
#define CONFIG_ROM_START_UP_ADDR 0x80100000
#define CONFIG_ROM_SIZE_MB 1
#define CONFIG_LINK_SCRIPT_RAM
#define CONFIG_RAM_START_UP_ADDR 0x81000000
#define CONFIG_RAM_SIZE_MB 64
#define CONFIG_HEAP_SIZE 2
#define CONFIG_STACK_SIZE 0x400
#define CONFIG_FPU_STACK_SIZE 0x1000
#endif

#define CONFIG_GCC_OPTIMIZE_LEVEL 0
#define CONFIG_OUTPUT_BINARY
#define CONFIG_COMPILE_DRIVER_ONLY
#define CONFIG_USE_NEW_LIBC

#define CONFIG_CONSOLE_PORT "/dev/ttyS3"
#define CONFIG_CONSOLE_YMODEM_RECV_DEST "./"
#define CONFIG_CONSOLE_BAUD_115200B
#define CONFIG_CONSOLE_BAUD_OTHER_VAL 115200
#define CONFIG_CONSOLE_BAUD 115200
#define CONFIG_CONSOLE_UPLOAD_YMODEM
#define CONFIG_CONSOLE_UPLOAD_DIR "/mnt/d/tftboot"
#define CONFIG_CONSOLE_UPLOAD_IMAGE_NAME "baremetal"

#ifdef __cplusplus
}
#endif

#endif /* SDK_CONFIG_H */