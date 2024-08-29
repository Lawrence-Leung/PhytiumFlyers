
PROJECT_DIR = $(CURDIR)
FREERTOS_SDK_DIR = $(CURDIR)/../../..


# # 设置启动镜像名
BOOT_IMG_NAME      ?= freertos

USER_CSRC := main.c
USER_CSRC += $(wildcard  src/*.c)
			
USER_ASRC := 
USER_CXXSRC := 

USER_INCLUDE := $(PROJECT_DIR)	\
				$(PROJECT_DIR)/inc 


include $(FREERTOS_SDK_DIR)/tools/makeall.mk

USR_BOOT_DIR		?= /mnt/d/tftboot


image:
	$(MAKE) clean
	$(MAKE) all -j
	@cp ./$(IMAGE_OUT_NAME).elf $(USR_BOOT_DIR)/freertos.elf
ifdef CONFIG_OUTPUT_BINARY
	@cp ./$(IMAGE_OUT_NAME).bin $(USR_BOOT_DIR)/freertos.bin
endif
	@ls $(USR_BOOT_DIR)/$(BOOT_IMG_NAME).* -l