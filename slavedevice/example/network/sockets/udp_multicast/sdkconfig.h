#ifndef SDK_CONFIG_H__
#define SDK_CONFIG_H__

/* Example Configuration */

#define CONFIG_EXAMPLE_IPV4_V6
/* CONFIG_EXAMPLE_IPV4_ONLY is not set */
#define CONFIG_EXAMPLE_IPV4
#define CONFIG_EXAMPLE_IPV6
#define CONFIG_EXAMPLE_MULTICAST_IPV4_ADDR "232.10.12.10"
#define CONFIG_EXAMPLE_MULTICAST_IPV6_ADDR "FF02::FD"
#define CONFIG_EXAMPLE_PORT 6750
/* CONFIG_EXAMPLE_LOOPBACK is not set */
#define CONFIG_EXAMPLE_MULTICAST_TTL 255
/* CONFIG_EXAMPLE_MULTICAST_LISTEN_ALL_IF is not set */
#define CONFIG_EXAMPLE_MULTICAST_LISTEN_DEFAULT_IF
/* end of Example Configuration */
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

#define CONFIG_TARGET_NAME "udp_multicast"
/* end of Build project name */
/* end of Board Configuration */

/* Sdk common configuration */

/* CONFIG_LOG_VERBOS is not set */
/* CONFIG_LOG_DEBUG is not set */
#define CONFIG_LOG_INFO
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
#define CONFIG_USE_ETH

/* Eth Configuration */

#define CONFIG_ENABLE_FXMAC
/* CONFIG_ENABLE_FGMAC is not set */
#define CONFIG_FXMAC_PHY_COMMON
/* CONFIG_FXMAC_PHY_YT is not set */
/* end of Eth Configuration */
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
/* CONFIG_USE_MEDIA is not set */
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
#define CONFIG_IMAGE_MAX_LENGTH 0x1000000
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

#define CONFIG_FREERTOS_USE_XMAC
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

/* CONFIG_FREERTOS_USE_MEDIA is not set */
/* end of Freertos Media Drivers */
/* end of Component Configuration */

/* Third-party configuration */

#define CONFIG_USE_LWIP

/* LWIP Freertos Port Configuration */

/* LWIP Configuration */

/* LWIP Port Configuration */

#define CONFIG_LWIP_FXMAC
/* CONFIG_LWIP_FGMAC is not set */
/* CONFIG_LWIP_RX_POLL is not set */
/* end of LWIP Port Configuration */
/* CONFIG_LWIP_NO_SYS is not set */
#define CONFIG_LWIP_LOCAL_HOSTNAME "phytium"

/* LWIP_APP */

/* CONFIG_USE_LWIP_APP_LWIPERF is not set */
/* CONFIG_USE_LWIP_APP_PING is not set */
/* CONFIG_USE_LWIP_APP_TFTP is not set */
/* end of LWIP_APP */

/* Memory configuration */

/* CONFIG_LWIP_USE_MEM_POOL is not set */
#define CONFIG_LWIP_USE_MEM_HEAP
/* CONFIG_LWIP_USE_MEM_HEAP_DEBUG is not set */
#define CONFIG_MEM_SIZE 1
#define CONFIG_MEM_ALIGNMENT 64
/* end of Memory configuration */

/* Pbuf options */

#define CONFIG_PBUF_POOL_BUFSIZE 2
#define CONFIG_PBUF_POOL_SIZE 1
/* end of Pbuf options */

/* ARP */

#define CONFIG_ARP_QUEUEING_EN
/* end of ARP */

/* IPV4 */

/* CONFIG_USE_IPV4_ONLY is not set */
#define CONFIG_LWIP_IP4_REASSEMBLY
#define CONFIG_LWIP_IP4_FRAG
/* CONFIG_LWIP_IP_FORWARD is not set */
#define CONFIG_IP_REASS_MAX_PBUFS 45
/* end of IPV4 */

/* ICMP */

#define CONFIG_LWIP_ICMP
#define CONFIG_LWIP_MULTICAST_PING
/* CONFIG_LWIP_BROADCAST_PING is not set */
/* end of ICMP */

/* LWIP RAW API */

#define CONFIG_LWIP_RAW_API_EN
#define CONFIG_LWIP_MAX_RAW_PCBS 16
/* end of LWIP RAW API */

/* DHCP */

#define CONFIG_LWIP_DHCP_ENABLE
/* CONFIG_LWIP_DHCP_DOES_ARP_CHECK is not set */
/* CONFIG_LWIP_DHCP_GET_NTP_SRV is not set */
/* CONFIG_LWIP_DHCP_DISABLE_CLIENT_ID is not set */
/* CONFIG_LWIP_DHCP_RESTORE_LAST_IP is not set */
#define CONFIG_LWIP_DHCP_OPTIONS_LEN 68
#define CONFIG_LWIP_DHCP_DISABLE_VENDOR_CLASS_ID
/* end of DHCP */

/* AUTOIP */

/* CONFIG_LWIP_AUTOIP is not set */
/* end of AUTOIP */

/* IGMP */

#define CONFIG_LWIP_IGMP_EN
/* end of IGMP */

/* DNS */

#define CONFIG_LWIP_DNS_SUPPORT_MDNS_QUERIES
/* end of DNS */

/* UDP */

#define CONFIG_LWIP_MAX_UDP_PCBS 16
#define CONFIG_LWIP_UDP_RECVMBOX_SIZE 6
/* CONFIG_LWIP_NETBUF_RECVINFO is not set */
/* end of UDP */

/* TCP */

#define CONFIG_LWIP_TCP_WND_DEFAULT 5744
#define CONFIG_LWIP_TCP_MAXRTX 12
#define CONFIG_LWIP_TCP_SYNMAXRTX 12
#define CONFIG_LWIP_TCP_QUEUE_OOSEQ
/* CONFIG_LWIP_TCP_SACK_OUT is not set */
#define CONFIG_LWIP_TCP_MSS 1440
#define CONFIG_LWIP_TCP_SND_BUF_DEFAULT 5744
#define CONFIG_LWIP_TCP_OVERSIZE_MSS
/* CONFIG_LWIP_TCP_OVERSIZE_QUARTER_MSS is not set */
/* CONFIG_LWIP_TCP_OVERSIZE_DISABLE is not set */
#define CONFIG_LWIP_TCP_TMR_INTERVAL 250
#define CONFIG_LWIP_TCP_MSL 60000
/* CONFIG_LWIP_WND_SCALE is not set */
#define CONFIG_LWIP_TCP_RTO_TIME 1500
#define CONFIG_LWIP_MAX_ACTIVE_TCP 16
#define CONFIG_LWIP_MAX_LISTENING_TCP 16
#define CONFIG_LWIP_TCP_HIGH_SPEED_RETRANSMISSION
#define CONFIG_LWIP_TCP_RECVMBOX_SIZE 6
/* end of TCP */

/* Network_Interface */

/* CONFIG_LWIP_NETIF_API is not set */
/* CONFIG_LWIP_NETIF_STATUS_CALLBACK is not set */
/* end of Network_Interface */

/* LOOPIF */

#define CONFIG_LWIP_NETIF_LOOPBACK
#define CONFIG_LWIP_LOOPBACK_MAX_PBUFS 8
/* end of LOOPIF */

/* SLIPIF */

/* CONFIG_LWIP_SLIP_SUPPORT is not set */
/* end of SLIPIF */
#define CONFIG_LWIP_TCPIP_CORE_LOCKING

/* Socket */

#define CONFIG_LWIP_MAX_SOCKETS 10
/* CONFIG_LWIP_SO_LINGER is not set */
#define CONFIG_LWIP_SO_REUSE
#define CONFIG_LWIP_SO_REUSE_RXTOALL
/* end of Socket */
/* CONFIG_LWIP_STATS is not set */

/* PPP */

/* CONFIG_LWIP_PPP_SUPPORT is not set */
#define CONFIG_LWIP_IPV6_MEMP_NUM_ND6_QUEUE 3
#define CONFIG_LWIP_IPV6_ND6_NUM_NEIGHBORS 5
/* end of PPP */

/* Checksums */

/* CONFIG_LWIP_CHECKSUM_CHECK_IP is not set */
/* CONFIG_LWIP_CHECKSUM_CHECK_UDP is not set */
#define CONFIG_LWIP_CHECKSUM_CHECK_ICMP
/* end of Checksums */

/* IPV6 */

#define CONFIG_LWIP_IPV6
/* CONFIG_LWIP_IPV6_AUTOCONFIG is not set */
#define CONFIG_LWIP_IPV6_NUM_ADDRESSES 3
/* CONFIG_LWIP_IPV6_FORWARD is not set */
#define CONFIG_LWIP_IP6_FRAG
#define CONFIG_LWIP_IP6_REASSEMBLY
/* end of IPV6 */
#define CONFIG_LWIP_DEBUG
/* CONFIG_LWIP_DEBUG_ESP_LOG is not set */
#define CONFIG_LWIP_NETIF_DEBUG
/* CONFIG_LWIP_PBUF_DEBUG is not set */
/* CONFIG_LWIP_ETHARP_DEBUG is not set */
/* CONFIG_LWIP_API_LIB_DEBUG is not set */
/* CONFIG_LWIP_SOCKETS_DEBUG is not set */
/* CONFIG_LWIP_IP_DEBUG is not set */
/* CONFIG_LWIP_ICMP_DEBUG is not set */
/* CONFIG_LWIP_DHCP_STATE_DEBUG is not set */
/* CONFIG_LWIP_DHCP_DEBUG is not set */
/* CONFIG_LWIP_IP6_DEBUG is not set */
/* CONFIG_LWIP_ICMP6_DEBUG is not set */
/* CONFIG_LWIP_TCP_DEBUG is not set */
/* CONFIG_LWIP_UDP_DEBUG is not set */
/* CONFIG_LWIP_SNTP_DEBUG is not set */
/* CONFIG_LWIP_DNS_DEBUG is not set */
/* end of LWIP Configuration */

/* Tcp/ip task resource configuration */

#define CONFIG_LWIP_TCPIP_TASK_STACK_SIZE 3072
#define CONFIG_LWIP_TCPIP_TASK_PRIO 5
#define CONFIG_LWIP_TCPIP_RECVMBOX_SIZE 32
/* end of Tcp/ip task resource configuration */

/* lwip port thread Configuration */

#define CONFIG_LWIP_PORT_USE_RECEIVE_THREAD
#define CONFIG_LWIP_PORT_RECEIVE_THREAD_STACKSIZE 2048
#define CONFIG_LWIP_PORT_RECEIVE_THREAD_PRIORITY 5
#define CONFIG_LWIP_PORT_USE_LINK_DETECT_THREAD
#define CONFIG_LWIP_PORT_LINK_DETECT_STACKSIZE 2048
#define CONFIG_LWIP_PORT_LINK_DETECT_PRIORITY 5
#define CONFIG_LWIP_PORT_DHCP_THREAD
#define CONFIG_LWIP_PORT_DHCP_STACKSIZE 4096
#define CONFIG_LWIP_PORT_DHCP_PRIORITY 5
/* end of lwip port thread Configuration */
/* end of LWIP Freertos Port Configuration */
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
#define CONFIG_USE_TLSF
/* CONFIG_USE_SPIFFS is not set */
/* CONFIG_USE_LITTLE_FS is not set */
/* CONFIG_USE_LVGL is not set */
/* CONFIG_USE_FREEMODBUS is not set */
/* CONFIG_USE_CHERRY_USB is not set */
/* end of Third-party configuration */

/* Kernel Configuration */

#define CONFIG_FREERTOS_OPTIMIZED_SCHEDULER
#define CONFIG_FREERTOS_HZ 1000
#define CONFIG_FREERTOS_MAX_PRIORITIES 32
#define CONFIG_FREERTOS_KERNEL_INTERRUPT_PRIORITIES 11
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
