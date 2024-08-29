include $(PROJECT_DIR)/sdkconfig

include src.mk

CSRCS_RELATIVE_FILES := $(foreach file, $(CSRCS_RELATIVE_FILES), $(wildcard */**/$(file)))

#LIBS_NAME ?= lib_littlefs

include $(SDK_DIR)/tools/build/compiler.mk