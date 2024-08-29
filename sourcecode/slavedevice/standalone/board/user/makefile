include $(PROJECT_DIR)/sdkconfig

CURRENT_DIR := 
BUILD_SUBDIRECTORY_PATH := usr

CSRCS_RELATIVE_FILES := $(foreach file, $(USER_CSRC), $(abspath $(wildcard $(PROJECT_DIR)/$(file))))
ASRCS_RELATIVE_FILES := $(foreach file, $(USER_ASRC), $(abspath $(wildcard $(PROJECT_DIR)/$(file))))
CXXSRCS_RELATIVE_FILES := $(foreach file, $(USER_CXXSRC), $(abspath $(wildcard $(PROJECT_DIR)/$(file))))

ABSOLUTE_CFILES := $(foreach file, $(EXTEND_CSRC), $(abspath $(file)))
ABSOLUTE_AFILES := $(foreach file, $(EXTEND_ASRC), $(abspath $(file)))
ABSOLUTE_CXXFILES := $(foreach file, $(EXTEND_CXXSRC), $(abspath $(file)))

BUILD_INC_PATH_DIR += $(USER_INCLUDE_PATH)

include $(SDK_DIR)/tools/build/compiler.mk