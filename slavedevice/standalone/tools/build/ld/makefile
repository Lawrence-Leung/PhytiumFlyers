
include $(PROJECT_DIR)/sdkconfig
include $(SDK_DIR)/tools/build/baremetal_include.mk
# for other sdk depends on standalone sdk
-include $(EXTERNAL_PATH)/include.mk
include $(SDK_DIR)/arch/$(subst ",,$(CONFIG_ARCH_NAME)/$(CONFIG_ARCH_EXECUTION_STATE))/arch_compiler.mk

LD_FILE_NAME := $(LD_NAME).ld
LDS := $(BUILD_OUT_PATH)/$(LD_NAME).ld.o
LDS_PRESRC := $(BUILD_OUT_PATH)/$(LD_NAME).ld.c

all:$(LDS) 


ifneq ($(wildcard ./$(LD_FILE_NAME)),)
# 文件存在的操作
		
else
# 文件不存在的操作
	$(error $(LD_FILE_NAME) dose not exist)
endif

$(LDS_PRESRC) :$(LD_FILE_NAME)
	@echo "ready to copy" $(LD_FILE_NAME)
	@mkdir -p $(BUILD_OUT_PATH)
	@cp -rf $(LD_FILE_NAME) $(LDS_PRESRC)

$(LDS): $(LDS_PRESRC)
	@$(CC) -P -E $< -o $@ $(patsubst %, -I %, $(BUILD_INC_PATH_DIR)) -I $(CURDIR)
	@rm -rf $(LDS_PRESRC)
	

