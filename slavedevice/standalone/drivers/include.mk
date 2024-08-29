
DRV_CUR_DIR := $(SDK_DIR)/drivers

##########################drivers####################################
# gic
# ifdef CONFIG_ENABLE_GICV3
BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/gic/fgic
# endif

# timer
ifdef CONFIG_ENABLE_TIMER_TACHO
	BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/timer/ftimer_tacho
endif

# mio
ifdef CONFIG_ENABLE_MIO
	BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/mio/fmio
endif

# qspi
ifdef CONFIG_USE_FQSPI
	BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/qspi/fqspi
endif #CONFIG_USE_NOR_QSPI

# usart
ifdef CONFIG_ENABLE_Pl011_UART
	BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/serial/fpl011
endif #CONFIG_ENABLE_Pl011_UART

# gpio
ifdef CONFIG_USE_GPIO
	ifdef CONFIG_ENABLE_FGPIO
		BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/pin/fgpio
	endif
endif

# spi
ifdef CONFIG_USE_FSPIM
	BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/spi/fspim
endif


ifdef CONFIG_ENABLE_FGMAC
	BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/eth/fgmac \
				$(DRV_CUR_DIR)/eth/fgmac/phy

	ifdef CONFIG_FGMAC_PHY_AR803X
		BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/eth/fgmac/phy/ar803x
	endif #CONFIG_FGMAC_PHY_AR803X	
endif #CONFIG_ENABLE_FGMAC


ifdef CONFIG_ENABLE_FXMAC
	BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/eth/fxmac \
				$(DRV_CUR_DIR)/eth/fxmac/phy

	ifdef CONFIG_FXMAC_PHY_YT
		BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/eth/fxmac/phy/yt
	endif
endif

# can
ifdef CONFIG_USE_FCAN
	BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/can/fcan
endif

# pcie
ifdef CONFIG_ENABLE_F_PCIE
	BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/pcie/fpcie_n
endif

ifdef CONFIG_ENABLE_FPCIE_ECAM
	BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/pcie/fpcie_ecam
endif


ifdef CONFIG_ENABLE_FPCIEC
	BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/pcie/fpciec
endif


# i2c
ifdef CONFIG_USE_FI2C
	BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/i2c/fi2c
endif

# fsdio
ifdef CONFIG_ENABLE_FSDIO
	BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/mmc/fsdio
endif

# fsdmmc
ifdef CONFIG_ENABLE_FSDMMC
	BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/mmc/fsdmmc
endif

#wdt
ifdef CONFIG_USE_FWDT
	BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/watchdog/fwdt
endif

# dma
ifdef CONFIG_ENABLE_FGDMA
	BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/dma/fgdma
endif

ifdef CONFIG_ENABLE_FDDMA
	BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/dma/fddma
endif

#nand
ifdef CONFIG_ENABLE_FNAND
	BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/nand/fnand
	BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/nand/fnand/manufacturer
endif

#iopad
ifdef CONFIG_ENABLE_IOPAD 
	BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/iomux/fiopad
endif

#ioctrl
ifdef CONFIG_ENABLE_IOCTRL
	BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/iomux/fioctrl
endif

#rtc
ifdef CONFIG_ENABLE_FRTC
	BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/rtc/frtc
endif

#sata
ifdef CONFIG_ENABLE_FSATA
	BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/sata/fsata
endif

#usb
ifdef CONFIG_USE_USB

BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/usb

ifdef CONFIG_ENABLE_USB_FXHCI
	BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/usb/fxhci
endif
endif

#adc
ifdef CONFIG_USE_FADC
	BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/adc/fadc
endif

#pwm
ifdef CONFIG_USE_FPWM
	BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/pwm/fpwm
endif

#semaphore
ifdef CONFIG_ENABLE_FSEMAPHORE
	BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/ipc/fsemaphore
endif

#scmi_mhu
ifdef CONFIG_ENABLE_SCMI_MHU
	BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/scmi/fscmi_mhu
endif

ifdef CONFIG_USE_MEDIA

	ifdef CONFIG_ENABLE_FDC_DP
		BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/media/fdcdp/Dc \
					$(DRV_CUR_DIR)/media/fdcdp/Dp \
					$(DRV_CUR_DIR)/media/fdcdp
	else
	    ifdef CONFIG_TARGET_ARMV8_AARCH64
			BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/media/fdcdp_lib/inc 
		else
			BUILD_INC_PATH_DIR += $(DRV_CUR_DIR)/media/fdcdp_lib/inc 
		endif
		
	endif

endif
