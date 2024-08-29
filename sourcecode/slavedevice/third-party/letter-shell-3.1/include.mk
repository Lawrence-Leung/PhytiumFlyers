
ifdef CONFIG_USE_LETTER_SHELL

THIRDP_CUR_DIR := $(FREERTOS_SDK_DIR)/third-party

# src files
BUILD_INC_PATH_DIR +=  $(THIRDP_CUR_DIR)/letter-shell-3.1/port \
			$(THIRDP_CUR_DIR)/letter-shell-3.1/src \
			$(THIRDP_CUR_DIR)/letter-shell-3.1

ifdef CONFIG_LS_PL011_UART
	BUILD_INC_PATH_DIR += $(THIRDP_CUR_DIR)/letter-shell-3.1/port/pl011
endif

endif






