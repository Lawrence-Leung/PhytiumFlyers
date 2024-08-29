#ifndef SDK_CONFIG_H__
#define SDK_CONFIG_H__

/* Project Configuration */

#define CONFIG_TARGET_NAME "export_test"
/* end of Project Configuration */
#define CONFIG_EXPORT_TYPE_RTT
#define CONFIG_USE_BAREMETAL

/* Platform Setting */

/* Arch Configuration */

/* CONFIG_TARGET_ARMV8_AARCH32 is not set */
#define CONFIG_TARGET_ARMV8_AARCH64
#define CONFIG_USE_CACHE
#define CONFIG_USE_MMU
/* CONFIG_USE_SYS_TICK is not set */
/* CONFIG_MMU_DEBUG_PRINTS is not set */
/* CONFIG_BOOT_WITH_FLUSH_CACHE is not set */
/* end of Arch Configuration */

/* Board Configuration */

/* CONFIG_TARGET_FT2004 is not set */
/* CONFIG_TARGET_D2000 is not set */
/* CONFIG_TARGET_E2000Q is not set */
#define CONFIG_TARGET_E2000D
/* CONFIG_TARGET_E2000S is not set */
#define CONFIG_TARGET_E2000
#define CONFIG_DEFAULT_DEBUG_PRINT_UART1
/* CONFIG_DEFAULT_DEBUG_PRINT_UART0 is not set */
/* CONFIG_DEFAULT_DEBUG_PRINT_UART2 is not set */
/* end of Board Configuration */

/* Components Configuration */

#define CONFIG_USE_SPI
#define CONFIG_USE_FSPIM
#define CONFIG_USE_QSPI

/* Qspi Configuration */

#define CONFIG_USE_FQSPI
/* end of Qspi Configuration */
#define CONFIG_USE_GIC
#define CONFIG_ENABLE_GICV3
#define CONFIG_USE_SERIAL

/* Usart Configuration */

#define CONFIG_ENABLE_Pl011_UART
/* end of Usart Configuration */
#define CONFIG_USE_GPIO
#define CONFIG_ENABLE_FGPIO
#define CONFIG_USE_ETH

/* Eth Configuration */

#define CONFIG_ENABLE_FXMAC
/* CONFIG_ENABLE_FGMAC is not set */
#define CONFIG_FXMAC_PHY_COMMON
/* CONFIG_FXMAC_PHY_YT is not set */
/* end of Eth Configuration */
#define CONFIG_USE_CAN

/* CAN Configuration */

#define CONFIG_USE_FCAN
#define CONFIG_FCAN_USE_CANFD
/* end of CAN Configuration */
#define CONFIG_USE_I2C
#define CONFIG_USE_FI2C
#define CONFIG_USE_TIMER

/* Hardware Timer Configuration */

#define CONFIG_ENABLE_TIMER_TACHO
/* end of Hardware Timer Configuration */
#define CONFIG_USE_MIO

/* Hardware Mio Configuration */

#define CONFIG_ENABLE_MIO
/* end of Hardware Mio Configuration */
#define CONFIG_USE_SDMMC
/* CONFIG_ENABLE_FSDMMC is not set */
#define CONFIG_ENABLE_FSDIO
#define CONFIG_USE_PCIE

/* Pcie Configuration */

#define CONFIG_ENABLE_F_PCIE
/* end of Pcie Configuration */
#define CONFIG_USE_WDT

/* FWDT Configuration */

#define CONFIG_USE_FWDT
/* end of FWDT Configuration */
#define CONFIG_USE_DMA
#define CONFIG_ENABLE_FGDMA
#define CONFIG_ENABLE_FDDMA
#define CONFIG_USE_NAND

/* NAND Configuration */

/* FNAND ip config */

#define CONFIG_ENABLE_FNAND
#define CONFIG_FNAND_COMMON_DEBUG_EN
/* CONFIG_FNAND_DMA_DEBUG_EN is not set */
/* CONFIG_FNAND_TOGGLE_DEBUG_EN is not set */
/* CONFIG_FNAND_ONFI_DEBUG_EN is not set */
/* end of FNAND ip config */
/* end of NAND Configuration */
/* CONFIG_USE_RTC is not set */
#define CONFIG_USE_SATA

/* FSATA Configuration */

#define CONFIG_ENABLE_FSATA
/* end of FSATA Configuration */
#define CONFIG_USE_USB
#define CONFIG_ENABLE_USB_FXHCI
#define CONFIG_USE_ADC

/* ADC Configuration */

#define CONFIG_USE_FADC
/* end of ADC Configuration */
#define CONFIG_USE_PWM

/* FPWM Configuration */

#define CONFIG_USE_FPWM
/* end of FPWM Configuration */
#define CONFIG_USE_IPC
#define CONFIG_ENABLE_FSEMAPHORE
#define CONFIG_USE_MEDIA

/* Media Configuration */

#define CONFIG_ENABLE_FDC_DP
/* CONFIG_ENABLE_FDC_DP_USE_LIB is not set */
/* end of Media Configuration */
#define CONFIG_USE_SCMI_MHU

/* Scmi Configuration */

#define CONFIG_ENABLE_SCMI_MHU
/* end of Scmi Configuration */
/* end of Components Configuration */
/* end of Platform Setting */

/* Building Option */

/* CONFIG_LOG_VERBOS is not set */
/* CONFIG_LOG_DEBUG is not set */
/* CONFIG_LOG_INFO is not set */
/* CONFIG_LOG_WARN is not set */
#define CONFIG_LOG_ERROR
/* CONFIG_LOG_NONE is not set */
#define CONFIG_USE_DEFAULT_INTERRUPT_CONFIG
#define CONFIG_INTERRUPT_ROLE_MASTER
/* CONFIG_INTERRUPT_ROLE_SLAVE is not set */
/* CONFIG_LOG_EXTRA_INFO is not set */
/* CONFIG_LOG_DISPALY_CORE_NUM is not set */
/* CONFIG_BOOTUP_DEBUG_PRINTS is not set */

/* Linker Options */

/* CONFIG_AARCH32_RAM_LD is not set */
#define CONFIG_AARCH64_RAM_LD
/* CONFIG_USER_DEFINED_LD is not set */
#define CONFIG_LINK_SCRIPT_ROM
#define CONFIG_ROM_START_UP_ADDR 0x80100000
#define CONFIG_ROM_SIZE_MB 1
#define CONFIG_LINK_SCRIPT_RAM
#define CONFIG_RAM_START_UP_ADDR 0x81000000
#define CONFIG_RAM_SIZE_MB 64
#define CONFIG_HEAP_SIZE 2
#define CONFIG_STACK_SIZE 0x400
#define CONFIG_FPU_STACK_SIZE 0x1000
/* end of Linker Options */

/* Compiler Options */

/* Cross-Compiler Setting */

#define CONFIG_GCC_OPTIMIZE_LEVEL 0
/* CONFIG_USE_EXT_COMPILER is not set */
/* CONFIG_USE_KLIN_SYS is not set */
/* end of Cross-Compiler Setting */
#define CONFIG_OUTPUT_BINARY
/* end of Compiler Options */
/* CONFIG_AUTO_GENERATE_MK_DEP is not set */
#define CONFIG_COMPILE_DRIVER_ONLY
/* end of Building Option */

/* Library Configuration */

#define CONFIG_USE_NEW_LIBC
/* end of Library Configuration */

/* Third-Party Configuration */

/* CONFIG_USE_LWIP is not set */
/* CONFIG_USE_LETTER_SHELL is not set */
/* CONFIG_USE_AMP is not set */
/* CONFIG_USE_SDMMC_CMD is not set */
/* CONFIG_USE_YMODEM is not set */
/* CONFIG_USE_SFUD is not set */
/* CONFIG_USE_BACKTRACE is not set */
/* CONFIG_USE_FATFS_0_1_4 is not set */
/* CONFIG_USE_TLSF is not set */
/* CONFIG_USE_SPIFFS is not set */
/* CONFIG_USE_LITTLE_FS is not set */
/* CONFIG_USE_LVGL is not set */
/* CONFIG_USE_FREEMODBUS is not set */
/* end of Third-Party Configuration */

/* PC Console Configuration */

#define CONFIG_CONSOLE_PORT "/dev/ttyS3"
#define CONFIG_CONSOLE_YMODEM_RECV_DEST "./"
#define CONFIG_CONSOLE_BAUD_115200B
/* CONFIG_CONSOLE_BAUD_230400B is not set */
/* CONFIG_CONSOLE_BAUD_921600B is not set */
/* CONFIG_CONSOLE_BAUD_2MB is not set */
/* CONFIG_CONSOLE_BAUD_OTHER is not set */
#define CONFIG_CONSOLE_BAUD_OTHER_VAL 115200
#define CONFIG_CONSOLE_BAUD 115200
/* CONFIG_CONSOLE_UPLOAD_TFTP is not set */
#define CONFIG_CONSOLE_UPLOAD_YMODEM
#define CONFIG_CONSOLE_UPLOAD_DIR "/mnt/d/tftboot"
#define CONFIG_CONSOLE_UPLOAD_IMAGE_NAME "baremetal"
/* end of PC Console Configuration */

#endif
