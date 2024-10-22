#ifndef SDK_CONFIG_H__
#define SDK_CONFIG_H__

#define CONFIG_USE_FREERTOS

/* Arch configuration */

#define CONFIG_TARGET_ARMv8
#define CONFIG_ARCH_NAME "armv8"

/* Arm architecture configuration */

#define CONFIG_ARCH_ARMV8_AARCH64
/* CONFIG_ARCH_ARMV8_AARCH32 is not set */

/* Compiler configuration */

#define CONFIG_ARM_GCC_SELECT
/* CONFIG_ARM_CLANG_SELECT is not set */
#define CONFIG_TOOLCHAIN_NAME "gcc"
#define CONFIG_TARGET_ARMV8_AARCH64
#define CONFIG_ARCH_EXECUTION_STATE "aarch64"

/* Fpu configuration */

#define CONFIG_ARM_NEON
/* end of Fpu configuration */
#define CONFIG_ARM_CRC
#define CONFIG_ARM_CRYPTO
#define CONFIG_ARM_FLOAT_POINT
/* CONFIG_GCC_CODE_MODEL_TINY is not set */
#define CONFIG_GCC_CODE_MODEL_SMALL
/* CONFIG_GCC_CODE_MODEL_LARGE is not set */
/* end of Compiler configuration */
#define CONFIG_USE_CACHE
#define CONFIG_USE_MMU
/* CONFIG_BOOT_WITH_FLUSH_CACHE is not set */
/* CONFIG_MMU_DEBUG_PRINTS is not set */
/* end of Arm architecture configuration */
/* end of Arch configuration */

/* Soc configuration */

#define CONFIG_TARGET_PHYTIUMPI
/* CONFIG_TARGET_E2000Q is not set */
/* CONFIG_TARGET_E2000D is not set */
/* CONFIG_TARGET_E2000S is not set */
/* CONFIG_TARGET_FT2004 is not set */
/* CONFIG_TARGET_D2000 is not set */
#define CONFIG_SOC_NAME "phytiumpi"
#define CONFIG_SOC_CORE_NUM 4
#define CONFIG_F32BIT_MEMORY_ADDRESS 0x80000000
#define CONFIG_F32BIT_MEMORY_LENGTH 0x80000000
#define CONFIG_F64BIT_MEMORY_ADDRESS 0x2000000000
#define CONFIG_F64BIT_MEMORY_LENGTH 0x800000000
#define CONFIG_TARGET_E2000
/* CONFIG_USE_SPINLOCK is not set */
#define CONFIG_DEFAULT_DEBUG_PRINT_UART1
/* CONFIG_DEFAULT_DEBUG_PRINT_UART0 is not set */
/* CONFIG_DEFAULT_DEBUG_PRINT_UART2 is not set */
/* end of Soc configuration */

/* Board Configuration */

#define CONFIG_BOARD_NAME "firefly"
/* CONFIG_USE_SPI_IOPAD is not set */
/* CONFIG_USE_GPIO_IOPAD is not set */
/* CONFIG_USE_CAN_IOPAD is not set */
/* CONFIG_USE_QSPI_IOPAD is not set */
/* CONFIG_USE_PWM_IOPAD is not set */
/* CONFIG_USE_MIO_IOPAD is not set */
/* CONFIG_USE_TACHO_IOPAD is not set */
/* CONFIG_USE_UART_IOPAD is not set */
/* CONFIG_USE_THIRD_PARTY_IOPAD is not set */
#define CONFIG_FIREFLY_DEMO_BOARD

/* IO mux configuration when board start up */

/* end of IO mux configuration when board start up */
/* CONFIG_CUS_DEMO_BOARD is not set */

/* Build project name */

#define CONFIG_TARGET_NAME "media"
/* end of Build project name */
/* end of Board Configuration */

/* Sdk common configuration */

/* CONFIG_LOG_VERBOS is not set */
#define CONFIG_LOG_DEBUG
/* CONFIG_LOG_INFO is not set */
/* CONFIG_LOG_WARN is not set */
/* CONFIG_LOG_ERROR is not set */
/* CONFIG_LOG_NONE is not set */
/* CONFIG_LOG_EXTRA_INFO is not set */
/* CONFIG_LOG_DISPALY_CORE_NUM is not set */
/* CONFIG_BOOTUP_DEBUG_PRINTS is not set */
#define CONFIG_USE_DEFAULT_INTERRUPT_CONFIG
#define CONFIG_INTERRUPT_ROLE_MASTER
/* CONFIG_INTERRUPT_ROLE_SLAVE is not set */
/* end of Sdk common configuration */

/* Image information configuration */

/* CONFIG_IMAGE_INFO is not set */
/* end of Image information configuration */

/* Drivers configuration */

#define CONFIG_USE_IOMUX
/* CONFIG_ENABLE_IOCTRL is not set */
#define CONFIG_ENABLE_IOPAD
/* CONFIG_USE_SPI is not set */
/* CONFIG_USE_QSPI is not set */
#define CONFIG_USE_GIC
#define CONFIG_ENABLE_GICV3
#define CONFIG_USE_SERIAL

/* Usart Configuration */

#define CONFIG_ENABLE_Pl011_UART
/* end of Usart Configuration */
/* CONFIG_USE_GPIO is not set */
/* CONFIG_USE_ETH is not set */
/* CONFIG_USE_CAN is not set */
/* CONFIG_USE_I2C is not set */
/* CONFIG_USE_TIMER is not set */
/* CONFIG_USE_MIO is not set */
/* CONFIG_USE_SDMMC is not set */
/* CONFIG_USE_PCIE is not set */
/* CONFIG_USE_WDT is not set */
/* CONFIG_USE_DMA is not set */
/* CONFIG_USE_NAND is not set */
/* CONFIG_USE_RTC is not set */
/* CONFIG_USE_SATA is not set */
/* CONFIG_USE_USB is not set */
/* CONFIG_USE_ADC is not set */
/* CONFIG_USE_PWM is not set */
/* CONFIG_USE_IPC is not set */
#define CONFIG_USE_MEDIA

/* Media Configuration */

#define CONFIG_ENABLE_FMEDIA
/* CONFIG_ENABLE_FDC_DP is not set */
#define CONFIG_ENABLE_FDC_DP_USE_LIB
/* end of Media Configuration */
/* CONFIG_USE_SCMI_MHU is not set */
/* end of Drivers configuration */

/* Build setup */

#define CONFIG_CHECK_DEPS
#define CONFIG_OUTPUT_BINARY

/* Optimization options */

/* CONFIG_DEBUG_NOOPT is not set */
/* CONFIG_DEBUG_CUSTOMOPT is not set */
#define CONFIG_DEBUG_FULLOPT
/* CONFIG_DEBUG_ENABLE_ALL_WARNING is not set */
#define CONFIG_DEBUG_OPT_UNUSED_SECTIONS
#define CONFIG_DEBUG_LINK_MAP
/* CONFIG_CCACHE is not set */
/* CONFIG_ARCH_COVERAGE is not set */
/* CONFIG_LTO_FULL is not set */
/* end of Optimization options */

/* Debug options */

/* CONFIG_WALL_WARNING_ERROR is not set */
/* CONFIG_STRICT_PROTOTYPES is not set */
/* CONFIG_DEBUG_SYMBOLS is not set */
/* CONFIG_FRAME_POINTER is not set */
/* CONFIG_OUTPUT_ASM_DIS is not set */
/* CONFIG_ENABLE_WSHADOW is not set */
/* CONFIG_ENABLE_WUNDEF is not set */
#define CONFIG_DOWNGRADE_DIAG_WARNING
/* end of Debug options */

/* Lib */

#define CONFIG_USE_COMPILE_CHAIN
/* CONFIG_USB_USER_DEFINED is not set */
/* end of Lib */
/* CONFIG_ENABLE_CXX is not set */

/* Linker Options */

#define CONFIG_DEFAULT_LINKER_SCRIPT
/* CONFIG_USER_DEFINED_LD is not set */
#define CONFIG_IMAGE_LOAD_ADDRESS 0x80100000
#define CONFIG_IMAGE_MAX_LENGTH 0x100000000
#define CONFIG_HEAP_SIZE 1
#define CONFIG_STACK_SIZE 0x400
#define CONFIG_FPU_STACK_SIZE 0x1000
/* end of Linker Options */
/* end of Build setup */

/* Component Configuration */

/* Freertos Uart Drivers */

#define CONFIG_FREERTOS_USE_UART
/* end of Freertos Uart Drivers */

/* Freertos Pwm Drivers */

/* CONFIG_FREERTOS_USE_PWM is not set */
/* end of Freertos Pwm Drivers */

/* Freertos Qspi Drivers */

/* CONFIG_FREERTOS_USE_QSPI is not set */
/* end of Freertos Qspi Drivers */

/* Freertos Wdt Drivers */

/* CONFIG_FREERTOS_USE_WDT is not set */
/* end of Freertos Wdt Drivers */

/* Freertos Eth Drivers */

/* CONFIG_FREERTOS_USE_XMAC is not set */
/* CONFIG_FREERTOS_USE_GMAC is not set */
/* end of Freertos Eth Drivers */

/* Freertos Gpio Drivers */

/* CONFIG_FREERTOS_USE_GPIO is not set */
/* end of Freertos Gpio Drivers */

/* Freertos Spim Drivers */

/* CONFIG_FREERTOS_USE_FSPIM is not set */
/* end of Freertos Spim Drivers */

/* Freertos DMA Drivers */

/* CONFIG_FREERTOS_USE_FDDMA is not set */
/* CONFIG_FREERTOS_USE_FGDMA is not set */
/* end of Freertos DMA Drivers */

/* Freertos Adc Drivers */

/* CONFIG_FREERTOS_USE_ADC is not set */
/* end of Freertos Adc Drivers */

/* Freertos Can Drivers */

/* CONFIG_FREERTOS_USE_CAN is not set */
/* end of Freertos Can Drivers */

/* Freertos I2c Drivers */

/* CONFIG_FREERTOS_USE_I2C is not set */
/* end of Freertos I2c Drivers */

/* Freertos Mio Drivers */

/* CONFIG_FREERTOS_USE_MIO is not set */
/* end of Freertos Mio Drivers */

/* Freertos Timer Drivers */

/* CONFIG_FREERTOS_USE_TIMER is not set */
/* end of Freertos Timer Drivers */

/* Freertos Media Drivers */

#define CONFIG_FREERTOS_USE_MEDIA
/* end of Freertos Media Drivers */
/* end of Component Configuration */

/* Third-party configuration */

/* CONFIG_USE_LWIP is not set */
#define CONFIG_USE_LETTER_SHELL

/* Letter Shell Configuration */

#define CONFIG_LS_PL011_UART
#define CONFIG_DEFAULT_LETTER_SHELL_USE_UART1
/* CONFIG_DEFAULT_LETTER_SHELL_USE_UART0 is not set */
/* CONFIG_DEFAULT_LETTER_SHELL_USE_UART2 is not set */
/* end of Letter Shell Configuration */
/* CONFIG_USE_AMP is not set */
/* CONFIG_USE_SDMMC_CMD is not set */
/* CONFIG_USE_YMODEM is not set */
/* CONFIG_USE_SFUD is not set */
#define CONFIG_USE_BACKTRACE
/* CONFIG_USE_FATFS_0_1_4 is not set */
/* CONFIG_USE_TLSF is not set */
/* CONFIG_USE_SPIFFS is not set */
/* CONFIG_USE_LITTLE_FS is not set */
#define CONFIG_USE_LVGL

/* Lvgl configuration */

/* CONFIG_LV_CONF_SKIP is not set */
#define CONFIG_LV_CONF_MINIMAL

/* Color settings */

#define CONFIG_LV_COLOR_DEPTH_32
/* CONFIG_LV_COLOR_DEPTH_16 is not set */
/* CONFIG_LV_COLOR_DEPTH_8 is not set */
/* CONFIG_LV_COLOR_DEPTH_1 is not set */
#define CONFIG_LV_COLOR_DEPTH 32
#define CONFIG_LV_COLOR_MIX_ROUND_OFS 128
#define CONFIG_LV_COLOR_CHROMA_KEY_HEX 0x00FF00
/* end of Color settings */

/* Memory settings */

/* CONFIG_LV_MEM_CUSTOM is not set */
#define CONFIG_LV_MEM_SIZE_KILOBYTES 32
#define CONFIG_LV_MEM_ADDR 0x0
#define CONFIG_LV_MEM_BUF_MAX_NUM 16
/* CONFIG_LV_MEMCPY_MEMSET_STD is not set */
/* end of Memory settings */

/* Hal settings */

/* CONFIG_LV_TICK_CUSTOM is not set */
#define CONFIG_LV_DPI_DEF 130
/* end of Hal settings */

/* Feature configuration */

/* Drawing */

#define CONFIG_LV_DRAW_COMPLEX
#define CONFIG_LV_SHADOW_CACHE_SIZE 0
#define CONFIG_LV_CIRCLE_CACHE_SIZE 4
#define CONFIG_LV_LAYER_SIMPLE_BUF_SIZE 24576
#define CONFIG_LV_IMG_CACHE_DEF_SIZE 0
#define CONFIG_LV_GRADIENT_MAX_STOPS 2
#define CONFIG_LV_GRAD_CACHE_DEF_SIZE 0
/* CONFIG_LV_DITHER_GRADIENT is not set */
#define CONFIG_LV_DISP_ROT_MAX_BUF 10240
/* end of Drawing */

/* Gpu */

/* CONFIG_LV_USE_GPU_ARM2D is not set */
/* CONFIG_LV_USE_GPU_STM32_DMA2D is not set */
/* CONFIG_LV_USE_GPU_SWM341_DMA2D is not set */
/* CONFIG_LV_USE_GPU_NXP_PXP is not set */
/* CONFIG_LV_USE_GPU_NXP_VG_LITE is not set */
/* CONFIG_LV_USE_GPU_SDL is not set */
/* end of Gpu */

/* Logging */

/* CONFIG_LV_USE_LOG is not set */
/* end of Logging */

/* Asserts */

#define CONFIG_LV_USE_ASSERT_NULL
#define CONFIG_LV_USE_ASSERT_MALLOC
/* CONFIG_LV_USE_ASSERT_STYLE is not set */
/* CONFIG_LV_USE_ASSERT_MEM_INTEGRITY is not set */
/* CONFIG_LV_USE_ASSERT_OBJ is not set */
#define CONFIG_LV_ASSERT_HANDLER_INCLUDE "assert.h"
/* end of Asserts */

/* Others */

/* CONFIG_LV_USE_PERF_MONITOR is not set */
/* CONFIG_LV_USE_MEM_MONITOR is not set */
/* CONFIG_LV_USE_REFR_DEBUG is not set */
/* CONFIG_LV_SPRINTF_CUSTOM is not set */
/* CONFIG_LV_SPRINTF_USE_FLOAT is not set */
#define CONFIG_LV_USE_USER_DATA
/* CONFIG_LV_ENABLE_GC is not set */
/* end of Others */

/* Compiler settings */

/* CONFIG_LV_BIG_ENDIAN_SYSTEM is not set */
#define CONFIG_LV_ATTRIBUTE_MEM_ALIGN_SIZE 1
/* CONFIG_LV_ATTRIBUTE_FAST_MEM_USE_IRAM is not set */
/* CONFIG_LV_USE_LARGE_COORD is not set */
/* end of Compiler settings */
/* end of Feature configuration */

/* Font usage */

/* Enable built-in fonts */

/* CONFIG_LV_FONT_MONTSERRAT_8 is not set */
/* CONFIG_LV_FONT_MONTSERRAT_10 is not set */
/* CONFIG_LV_FONT_MONTSERRAT_12 is not set */
#define CONFIG_LV_FONT_MONTSERRAT_14
/* CONFIG_LV_FONT_MONTSERRAT_16 is not set */
/* CONFIG_LV_FONT_MONTSERRAT_18 is not set */
/* CONFIG_LV_FONT_MONTSERRAT_20 is not set */
/* CONFIG_LV_FONT_MONTSERRAT_22 is not set */
/* CONFIG_LV_FONT_MONTSERRAT_24 is not set */
/* CONFIG_LV_FONT_MONTSERRAT_26 is not set */
/* CONFIG_LV_FONT_MONTSERRAT_28 is not set */
/* CONFIG_LV_FONT_MONTSERRAT_30 is not set */
/* CONFIG_LV_FONT_MONTSERRAT_32 is not set */
/* CONFIG_LV_FONT_MONTSERRAT_34 is not set */
/* CONFIG_LV_FONT_MONTSERRAT_36 is not set */
/* CONFIG_LV_FONT_MONTSERRAT_38 is not set */
/* CONFIG_LV_FONT_MONTSERRAT_40 is not set */
/* CONFIG_LV_FONT_MONTSERRAT_42 is not set */
/* CONFIG_LV_FONT_MONTSERRAT_44 is not set */
/* CONFIG_LV_FONT_MONTSERRAT_46 is not set */
/* CONFIG_LV_FONT_MONTSERRAT_48 is not set */
/* CONFIG_LV_FONT_MONTSERRAT_12_SUBPX is not set */
/* CONFIG_LV_FONT_MONTSERRAT_28_COMPRESSED is not set */
/* CONFIG_LV_FONT_DEJAVU_16_PERSIAN_HEBREW is not set */
/* CONFIG_LV_FONT_SIMSUN_16_CJK is not set */
/* CONFIG_LV_FONT_UNSCII_8 is not set */
/* CONFIG_LV_FONT_UNSCII_16 is not set */
/* end of Enable built-in fonts */
/* CONFIG_LV_FONT_DEFAULT_MONTSERRAT_8 is not set */
/* CONFIG_LV_FONT_DEFAULT_MONTSERRAT_12 is not set */
#define CONFIG_LV_FONT_DEFAULT_MONTSERRAT_14
/* CONFIG_LV_FONT_DEFAULT_MONTSERRAT_16 is not set */
/* CONFIG_LV_FONT_DEFAULT_MONTSERRAT_18 is not set */
/* CONFIG_LV_FONT_DEFAULT_MONTSERRAT_20 is not set */
/* CONFIG_LV_FONT_DEFAULT_MONTSERRAT_22 is not set */
/* CONFIG_LV_FONT_DEFAULT_MONTSERRAT_24 is not set */
/* CONFIG_LV_FONT_DEFAULT_MONTSERRAT_26 is not set */
/* CONFIG_LV_FONT_DEFAULT_MONTSERRAT_28 is not set */
/* CONFIG_LV_FONT_DEFAULT_MONTSERRAT_30 is not set */
/* CONFIG_LV_FONT_DEFAULT_MONTSERRAT_32 is not set */
/* CONFIG_LV_FONT_DEFAULT_MONTSERRAT_34 is not set */
/* CONFIG_LV_FONT_DEFAULT_MONTSERRAT_36 is not set */
/* CONFIG_LV_FONT_DEFAULT_MONTSERRAT_38 is not set */
/* CONFIG_LV_FONT_DEFAULT_MONTSERRAT_40 is not set */
/* CONFIG_LV_FONT_DEFAULT_MONTSERRAT_42 is not set */
/* CONFIG_LV_FONT_DEFAULT_MONTSERRAT_44 is not set */
/* CONFIG_LV_FONT_DEFAULT_MONTSERRAT_46 is not set */
/* CONFIG_LV_FONT_DEFAULT_MONTSERRAT_48 is not set */
/* CONFIG_LV_FONT_DEFAULT_MONTSERRAT_12_SUBPX is not set */
/* CONFIG_LV_FONT_DEFAULT_MONTSERRAT_28_COMPRESSED is not set */
/* CONFIG_LV_FONT_DEFAULT_DEJAVU_16_PERSIAN_HEBREW is not set */
/* CONFIG_LV_FONT_DEFAULT_SIMSUN_16_CJK is not set */
/* CONFIG_LV_FONT_DEFAULT_UNSCII_8 is not set */
/* CONFIG_LV_FONT_DEFAULT_UNSCII_16 is not set */
/* CONFIG_LV_FONT_FMT_TXT_LARGE is not set */
/* CONFIG_LV_USE_FONT_COMPRESSED is not set */
/* CONFIG_LV_USE_FONT_SUBPX is not set */
#define CONFIG_LV_USE_FONT_PLACEHOLDER
/* end of Font usage */

/* Text settings */

#define CONFIG_LV_TXT_ENC_UTF8
/* CONFIG_LV_TXT_ENC_ASCII is not set */
#define CONFIG_LV_TXT_BREAK_CHARS " ,.;:-_"
#define CONFIG_LV_TXT_LINE_BREAK_LONG_LEN 0
#define CONFIG_LV_TXT_COLOR_CMD "#"
/* CONFIG_LV_USE_BIDI is not set */
/* CONFIG_LV_USE_ARABIC_PERSIAN_CHARS is not set */
/* end of Text settings */

/* Widget usage */

#define CONFIG_LV_USE_ARC
#define CONFIG_LV_USE_BAR
#define CONFIG_LV_USE_BTN
#define CONFIG_LV_USE_BTNMATRIX
#define CONFIG_LV_USE_CANVAS
#define CONFIG_LV_USE_CHECKBOX
#define CONFIG_LV_USE_DROPDOWN
#define CONFIG_LV_USE_IMG
#define CONFIG_LV_USE_LABEL
#define CONFIG_LV_LABEL_TEXT_SELECTION
#define CONFIG_LV_LABEL_LONG_TXT_HINT
#define CONFIG_LV_USE_LINE
#define CONFIG_LV_USE_ROLLER
#define CONFIG_LV_ROLLER_INF_PAGES 7
#define CONFIG_LV_USE_SLIDER
#define CONFIG_LV_USE_SWITCH
#define CONFIG_LV_USE_TEXTAREA
#define CONFIG_LV_TEXTAREA_DEF_PWD_SHOW_TIME 1500
#define CONFIG_LV_USE_TABLE
/* end of Widget usage */

/* Extra widgets */

#define CONFIG_LV_USE_ANIMIMG
#define CONFIG_LV_USE_CALENDAR
/* CONFIG_LV_CALENDAR_WEEK_STARTS_MONDAY is not set */
#define CONFIG_LV_USE_CALENDAR_HEADER_ARROW
#define CONFIG_LV_USE_CALENDAR_HEADER_DROPDOWN
#define CONFIG_LV_USE_CHART
#define CONFIG_LV_USE_COLORWHEEL
#define CONFIG_LV_USE_IMGBTN
#define CONFIG_LV_USE_KEYBOARD
#define CONFIG_LV_USE_LED
#define CONFIG_LV_USE_LIST
#define CONFIG_LV_USE_MENU
#define CONFIG_LV_USE_METER
#define CONFIG_LV_USE_MSGBOX
#define CONFIG_LV_USE_SPAN
#define CONFIG_LV_SPAN_SNIPPET_STACK_SIZE 64
#define CONFIG_LV_USE_SPINBOX
#define CONFIG_LV_USE_SPINNER
#define CONFIG_LV_USE_TABVIEW
#define CONFIG_LV_USE_TILEVIEW
#define CONFIG_LV_USE_WIN
/* end of Extra widgets */

/* Themes */

#define CONFIG_LV_USE_THEME_DEFAULT
/* CONFIG_LV_THEME_DEFAULT_DARK is not set */
#define CONFIG_LV_THEME_DEFAULT_GROW
#define CONFIG_LV_THEME_DEFAULT_TRANSITION_TIME 80
#define CONFIG_LV_USE_THEME_BASIC
/* CONFIG_LV_USE_THEME_MONO is not set */
/* end of Themes */

/* Layouts */

#define CONFIG_LV_USE_FLEX
#define CONFIG_LV_USE_GRID
/* end of Layouts */

/* 3rd party libraries */

/* CONFIG_LV_USE_FS_STDIO is not set */
/* CONFIG_LV_USE_FS_POSIX is not set */
/* CONFIG_LV_USE_FS_WIN32 is not set */
/* CONFIG_LV_USE_FS_FATFS is not set */
/* CONFIG_LV_USE_PNG is not set */
/* CONFIG_LV_USE_BMP is not set */
/* CONFIG_LV_USE_SJPG is not set */
/* CONFIG_LV_USE_GIF is not set */
/* CONFIG_LV_USE_QRCODE is not set */
/* CONFIG_LV_USE_FREETYPE is not set */
/* CONFIG_LV_USE_RLOTTIE is not set */
/* CONFIG_LV_USE_FFMPEG is not set */
/* end of 3rd party libraries */

/* Others */

#define CONFIG_LV_USE_SNAPSHOT
/* CONFIG_LV_USE_MONKEY is not set */
/* CONFIG_LV_USE_GRIDNAV is not set */
/* CONFIG_LV_USE_FRAGMENT is not set */
/* CONFIG_LV_USE_IMGFONT is not set */
/* CONFIG_LV_USE_MSG is not set */
/* CONFIG_LV_USE_IME_PINYIN is not set */
/* end of Others */

/* Examples */

#define CONFIG_LV_BUILD_EXAMPLES
/* end of Examples */

/* Demos */

/* CONFIG_LV_USE_DEMO_WIDGETS is not set */
/* CONFIG_LV_USE_DEMO_KEYPAD_AND_ENCODER is not set */
#define CONFIG_LV_USE_DEMO_BENCHMARK
/* CONFIG_LV_DEMO_BENCHMARK_RGB565A8 is not set */
/* CONFIG_LV_USE_DEMO_STRESS is not set */
/* CONFIG_LV_USE_DEMO_MUSIC is not set */
/* end of Demos */
/* end of Lvgl configuration */
/* CONFIG_USE_FREEMODBUS is not set */
/* CONFIG_USE_CHERRY_USB is not set */
/* end of Third-party configuration */

/* Kernel Configuration */

#define CONFIG_FREERTOS_OPTIMIZED_SCHEDULER
#define CONFIG_FREERTOS_HZ 1000
#define CONFIG_FREERTOS_MAX_PRIORITIES 32
#define CONFIG_FREERTOS_KERNEL_INTERRUPT_PRIORITIES 13
#define CONFIG_FREERTOS_MAX_API_CALL_INTERRUPT_PRIORITIES 11
#define CONFIG_FREERTOS_THREAD_LOCAL_STORAGE_POINTERS 1
#define CONFIG_FREERTOS_MINIMAL_TASK_STACKSIZE 1024
#define CONFIG_FREERTOS_MAX_TASK_NAME_LEN 32
#define CONFIG_FREERTOS_TIMER_TASK_PRIORITY 1
#define CONFIG_FREERTOS_TIMER_TASK_STACK_DEPTH 2048
#define CONFIG_FREERTOS_TIMER_QUEUE_LENGTH 10
#define CONFIG_FREERTOS_QUEUE_REGISTRY_SIZE 0
#define CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS
#define CONFIG_FREERTOS_USE_TRACE_FACILITY
#define CONFIG_FREERTOS_USE_STATS_FORMATTING_FUNCTIONS
/* CONFIG_FREERTOS_USE_TICKLESS_IDLE is not set */
#define CONFIG_FREERTOS_TOTAL_HEAP_SIZE 10240
#define CONFIG_FREERTOS_TASK_FPU_SUPPORT 1
/* end of Kernel Configuration */

#endif
