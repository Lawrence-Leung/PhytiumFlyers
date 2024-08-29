
FREERTOS_SDK_DIR ?= $(SDK_DIR)/..

# drivers
include $(FREERTOS_SDK_DIR)/drivers/include.mk

# freertos sdk
include $(FREERTOS_SDK_DIR)/third-party/include.mk
