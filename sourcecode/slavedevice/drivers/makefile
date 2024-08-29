include $(PROJECT_DIR)/sdkconfig

include adc/src.mk
include can/src.mk
include dma/src.mk
include eth/src.mk
include gpio/src.mk
include i2c/src.mk
include media/src.mk
include pwm/src.mk
include qspi/src.mk
include serial/src.mk
include spi/src.mk
include timer/src.mk
include wdt/src.mk

CSRCS_RELATIVE_FILES := $(foreach file, $(DRIVERS_CSRCS), $(file))
ASRCS_RELATIVE_FILES := $(foreach file, $(DRIVERS_ASRCS),$(file))

include $(SDK_DIR)/tools/build/compiler.mk
