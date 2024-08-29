PROJECT_DIR = $(CURDIR)
FREERTOS_SDK_DIR = $(CURDIR)/../../../../../

USER_CSRC := $(wildcard  ./*.c)

USER_INCLUDE := $(PROJECT_DIR)

include $(FREERTOS_SDK_DIR)/tools/makeall.mk

# 用户定义的编译目标文件上传路径 
ifeq ($(OS),Windows_NT)
	USR_BOOT_DIR		?= $(subst \,/, $(PHYTIUM_DEV_PATH))/tftp
else
	USR_BOOT_DIR		?= /mnt/d/tftboot
endif

# 设置启动镜像名
USER_BOOT_IMAGE    ?= openamp_core1

# 完成编译
image:
	$(MAKE) clean
	$(MAKE) all -j
	@cp ./$(IMAGE_OUT_NAME).elf $(USR_BOOT_DIR)/$(USER_BOOT_IMAGE).elf
ifdef CONFIG_OUTPUT_BINARY
	@cp ./$(IMAGE_OUT_NAME).bin $(USR_BOOT_DIR)/$(USER_BOOT_IMAGE).bin
endif
	@ls $(USR_BOOT_DIR)/$(USER_BOOT_IMAGE).* -l
