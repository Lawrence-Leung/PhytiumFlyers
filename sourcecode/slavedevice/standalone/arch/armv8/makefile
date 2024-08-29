include $(PROJECT_DIR)/sdkconfig

ARCH_EXECUTION_STATE :=  $(subst ",,$(CONFIG_ARCH_EXECUTION_STATE))
ARCH_NAME := $(subst ",,$(CONFIG_ARCH_NAME))


include  $(SDK_DIR)/arch/$(ARCH_NAME)/$(ARCH_EXECUTION_STATE)/src.mk
include common/src.mk

CSRCS_RELATIVE_FILES := $(foreach file, $(ARCH_CSRCS), $(wildcard $(ARCH_EXECUTION_STATE)/$(file) $(file)))
ASRCS_RELATIVE_FILES := $(foreach file, $(ARCH_ASRCS), $(wildcard $(ARCH_EXECUTION_STATE)/$(file) $(file)))


include $(SDK_DIR)/tools/build/compiler.mk
