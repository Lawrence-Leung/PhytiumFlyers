# 定义环境变量用于承接sdk的路径
SDK_DIR ?= $(CURDIR)/../../..
PROJECT_DIR ?= $(CURDIR)
# 用户自定义的变量与路径
IMAGE_NAME ?= boot

# menuconfig 的路径
SDK_TOOLS_DIR ?= $(SDK_DIR)/tools/build
SDK_KCONFIG_DIR ?= $(SDK_DIR)/tools/build/Kconfiglib
# 编译中间文件输出的路径
BUILD_OUT_PATH ?= $(PROJECT_DIR)/build


IMAGE_OUTPUT := $(AMP_PROJECT_PATH)
CONFIGS_OUTPUT_PATH := $(AMP_PROJECT_PATH)

# AMP 镜像打包相关路径
AMP_PACK_FILE = ./amp_pack.c


# export BUILD_OUT_PATH
export SDK_DIR
ifneq ($(wildcard $(SDK_DIR)/tools/build_baremetal.mk),)
$(error SDK_DIR/tools/build_baremetal.mk dose not exist)
endif

all: boot_memory_check $(IMAGE_NAME).elf

# boot code
$(BUILD_OUT_PATH)/libboot.a: libboot.a
libboot.a:
	$(call invoke_make_in_directory,/tools/build/boot_code,makeboot.mk,all, USER_INCLUDE_PATH="$(USER_INCLUDE)")
libboot_debug:
	$(call invoke_make_in_directory,/tools/build/boot_code,makeboot.mk,debug, USER_INCLUDE_PATH="$(USER_INCLUDE)")
libboot_info:
	$(call invoke_make_in_directory,/tools/build/boot_code,makeboot.mk,compiler_info, USER_INCLUDE_PATH="$(USER_INCLUDE)")
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/libboot.a

boot_memory_check:
	@python3 $(SDK_DIR)/tools/build/py_tools/boot_memory_check.py

# 明确需要编译多少.a 文件
include $(PROJECT_DIR)/sdkconfig
ifneq ($(CONFIG_ARCH_NAME)/$(CONFIG_ARCH_EXECUTION_STATE),/)
include $(SDK_DIR)/arch/$(subst ",,$(CONFIG_ARCH_NAME)/$(CONFIG_ARCH_EXECUTION_STATE))/arch_compiler.mk
endif


AMP_PACK_OBJS :=  $(AMP_PACK_FILE:.c=.o)
EXTRA_OBJS := $(BUILD_OUT_PATH)/$(AMP_PACK_OBJS)
$(EXTRA_OBJS):$(AMP_PACK_FILE) $(AMP_IMG_EXPORT_IMG)
	@rm -rf $(EXTRA_OBJS)
	echo EXTRA_OBJS: $(EXTRA_OBJS)
	@mkdir -p $(BUILD_OUT_PATH)/
	$(CC) $(CFLAGS) $(INCLUDE_PATH) -c   $(AMP_PACK_FILE) -o $(EXTRA_OBJS)
	$(OBJCOPY) --add-section .my_amp_img=$(AMP_IMG_EXPORT_IMG) --set-section-flags .my_amp_img=alloc,load,readonly $(EXTRA_OBJS)


include $(SDK_DIR)/tools/build/build.mk

# make menuconfig 工具包含
include $(SDK_DIR)/tools/build/menuconfig/preconfig.mk
include $(SDK_DIR)/tools/build/menuconfig/menuconfig.mk


