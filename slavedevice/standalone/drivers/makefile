include $(PROJECT_DIR)/sdkconfig

include adc/src.mk
include can/src.mk
include dma/src.mk
include eth/src.mk
include gic/src.mk
include i2c/src.mk
include iomux/src.mk
include ipc/src.mk
include media/src.mk
include mio/src.mk
include mmc/src.mk
include nand/src.mk
include pcie/src.mk
include pin/src.mk
include pwm/src.mk
include qspi/src.mk
include rtc/src.mk
include sata/src.mk
include scmi/src.mk
include serial/src.mk
include spi/src.mk
include timer/src.mk
include usb/src.mk
include watchdog/src.mk


CSRCS_RELATIVE_FILES := $(foreach file, $(DRIVERS_CSRCS), $(wildcard */**/$(file)))
ASRCS_RELATIVE_FILES := $(foreach file, $(DRIVERS_ASRCS), $(wildcard */**/$(file)))

#LIBS_NAME ?= libdrivers

include $(SDK_DIR)/tools/build/compiler.mk
