include $(PROJECT_DIR)/sdkconfig

include src.mk

CSRCS_RELATIVE_FILES := $(foreach file, $(SHELL_CSRCS), $(wildcard */**/$(file) **/$(file)))

include $(SDK_DIR)/tools/build/compiler.mk
