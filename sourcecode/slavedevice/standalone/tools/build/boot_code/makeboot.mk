include $(PROJECT_DIR)/sdkconfig

CURRENT_DIR := 
BUILD_SUBDIRECTORY_PATH := boot

BOOT_CSRC := main.c

CSRCS_RELATIVE_FILES := $(PROJECT_DIR)/main.c

BUILD_INC_PATH_DIR += $(USER_INCLUDE_PATH)

include $(SDK_DIR)/tools/build/compiler.mk
