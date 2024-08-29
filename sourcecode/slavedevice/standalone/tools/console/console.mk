.PHONY: image tftp_upload serial_upload monitor

CONSOLE_PY := $(SDK_DIR)/tools/console/main.py
TARGET_BOOT_ELF := $(CONFIG_CONSOLE_UPLOAD_DIR)/$(CONFIG_CONSOLE_UPLOAD_IMAGE_NAME).elf
TARGET_BOOT_BIN := $(CONFIG_CONSOLE_UPLOAD_DIR)/$(CONFIG_CONSOLE_UPLOAD_IMAGE_NAME).bin
USER_BIN_BOOT_ADDR ?= $(CONFIG_IMAGE_LOAD_ADDRESS)

# upload image to board by tftp protocol
tftp_upload:
	@echo "flashing elf image: " $(TARGET_BOOT_ELF) 
	@echo "sending cmd to serial port: " $(CONFIG_CONSOLE_PORT)
	@echo "boot image name :" $(TARGET_BOOT_ELF)
	@if [ -e $(TARGET_BOOT_ELF) ]; then\
			 $(CONSOLE_PY) \
			 	--load 0 \
				--port $(CONFIG_CONSOLE_PORT) \
				--baud $(CONFIG_CONSOLE_BAUD) \
				--elf_file $(TARGET_BOOT_ELF) \
				--boardip $(CONFIG_UBOOT_BOARD_IP) \
				--hostip $(CONFIG_UBOOT_HOST_IP) \
				--gatewayip $(CONFIG_UBOOT_GATEWAY_IP) \
				--bootaddr $(CONFIG_UBOOT_ELF_BOOT_ADDR); \
	else\
		@echo "failed!!! target image" $(TARGET_BOOT_ELF) "no found";\
	fi

# upload image to board by ymodem protocol
serial_upload:
ifdef CONFIG_OUTPUT_BINARY
	@$(SDK_DIR)/tools/console/serial_access.py \
		-p "$(CONFIG_CONSOLE_PORT)" \
		-b "$(CONFIG_CONSOLE_BAUD)" \
		-c "loadx $(USER_BIN_BOOT_ADDR)" \
		-r "" \
		-t "0.5"

	$(SDK_DIR)/tools/console/ymodem_flash.py \
		-p "$(CONFIG_CONSOLE_PORT)" \
		-b "$(CONFIG_CONSOLE_BAUD)" \
		-f $(TARGET_BOOT_BIN)


	$(SDK_DIR)/tools/console/serial_access.py \
		-p "$(CONFIG_CONSOLE_PORT)" \
		-b "$(CONFIG_CONSOLE_BAUD)" \
		-c "\r\n" \
		-r "" \
		-t "0.5"

	@$(SDK_DIR)/tools/console/serial_access.py \
		-p "$(CONFIG_CONSOLE_PORT)" \
		-b "$(CONFIG_CONSOLE_BAUD)" \
		-c "go $(USER_BIN_BOOT_ADDR)" \
		-r "phytium:/" \
		-t "0.5"

endif

# connect to board with serial port
monitor:
	@echo $(TARGET_BOOT_ELF)
	@if [ -e $(TARGET_BOOT_ELF) ]; then\
			 $(CONSOLE_PY) \
			 	--load 1 \
				--monitor 1 \
				--port $(CONFIG_CONSOLE_PORT) \
				--baud $(CONFIG_CONSOLE_BAUD) \
				--elf_file $(TARGET_BOOT_ELF) \
				--ymodeldest $(CONFIG_CONSOLE_YMODEM_RECV_DEST);\
	else\
		@echo "failed!!! target image" $(TARGET_BOOT_ELF) "no found";\
	fi


