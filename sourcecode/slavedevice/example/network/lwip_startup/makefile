PROJECT_DIR = $(CURDIR)
FREERTOS_SDK_DIR = $(CURDIR)/../../../

# # 设置启动镜像名
BOOT_IMG_NAME      ?= freertos

USER_CSRC := main.c
USER_CSRC += $(wildcard  src/*.c)

USER_INCLUDE := $(PROJECT_DIR)	\
				$(PROJECT_DIR)/inc

include $(FREERTOS_SDK_DIR)/tools/makeall.mk

# 用户定义的编译目标文件上传路径 
ifeq ($(OS),Windows_NT)
USR_BOOT_DIR		?= /d/tftboot
else
USR_BOOT_DIR		?= /mnt/d/tftboot
endif

image:
	$(MAKE) all -j
	@cp ./$(IMAGE_OUT_NAME).elf $(USR_BOOT_DIR)/$(BOOT_IMG_NAME).elf
ifdef CONFIG_OUTPUT_BINARY
	@cp ./$(IMAGE_OUT_NAME).bin $(USR_BOOT_DIR)/$(BOOT_IMG_NAME).bin
endif
	@ls $(USR_BOOT_DIR)/$(BOOT_IMG_NAME).* -l

jailhouse:
	scp ./$(CONFIG_TARGET_NAME).bin root@192.168.4.224:/root/gitee_jailhouse/phytium-jailhouse/src/jailhouse
