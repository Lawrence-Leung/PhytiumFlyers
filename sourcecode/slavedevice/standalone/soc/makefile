include $(PROJECT_DIR)/sdkconfig

SOC_NAME := $(subst ",,$(CONFIG_SOC_NAME))

include $(SOC_NAME)/src.mk
include common/src.mk

CSRCS_RELATIVE_FILES := $(foreach file, $(SOC_CSRCS), $(wildcard $(SOC_NAME)/$(file) common/$(file)))
ASRCS_RELATIVE_FILES := $(foreach file, $(SOC_ASRCS), $(wildcard $(SOC_NAME)/$(file) common/$(file)))

#LIBS_NAME ?= libsoc

include $(SDK_DIR)/tools/build/compiler.mk