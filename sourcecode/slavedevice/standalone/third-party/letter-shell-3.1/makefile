include $(PROJECT_DIR)/sdkconfig

include src.mk

# CSRCS_RELATIVE_FILES := $(foreach file, $(SHELL_CSRCS), $(wildcard */**/$(file)))
CSRCS_RELATIVE_FILES := $(foreach file, $(SHELL_CSRCS), $(wildcard */**/$(file) **/$(file)))

#LIBS_NAME ?= lib_letter_shell

include $(SDK_DIR)/tools/build/compiler.mk
