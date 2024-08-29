
FREERTOS_SDK_DIR ?= $(SDK_DIR)/..

ifdef CONFIG_USE_BACKTRACE
include $(FREERTOS_SDK_DIR)/third-party/backtrace/include.mk
endif

ifdef CONFIG_USE_LETTER_SHELL
include $(FREERTOS_SDK_DIR)/third-party/letter-shell-3.1/include.mk
endif

include $(FREERTOS_SDK_DIR)/third-party/freertos/include.mk

ifdef CONFIG_USE_TLSF
include $(FREERTOS_SDK_DIR)/third-party/tlsf-3.1.0/include.mk
endif

ifdef CONFIG_USE_FATFS_0_1_4
include $(FREERTOS_SDK_DIR)/third-party/fatfs-0.1.4/include.mk
endif

ifdef CONFIG_USE_LWIP
include $(FREERTOS_SDK_DIR)/third-party/lwip-2.1.2/include.mk
endif

ifdef CONFIG_USE_SPIFFS
include $(FREERTOS_SDK_DIR)/third-party/spiffs-0.3.7/include.mk
endif

ifdef CONFIG_USE_SFUD
include $(FREERTOS_SDK_DIR)/third-party/sfud-1.1.0/include.mk
endif

ifdef CONFIG_USE_LVGL
include $(FREERTOS_SDK_DIR)/third-party/lvgl-8.3/include.mk
endif

ifdef CONFIG_USE_CHERRY_USB
include $(FREERTOS_SDK_DIR)/third-party/cherryusb/include.mk
endif

# fsdio
ifdef CONFIG_USE_SDMMC
include $(FREERTOS_SDK_DIR)/third-party/sdmmc-1.0/include.mk
endif

ifdef CONFIG_USE_LIBMETAL
include $(FREERTOS_SDK_DIR)/third-party/libmetal/include.mk
endif

ifdef CONFIG_USE_OPENAMP
include $(FREERTOS_SDK_DIR)/third-party/openamp/include.mk
endif