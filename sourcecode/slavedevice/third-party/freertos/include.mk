
ifdef CONFIG_USE_FREERTOS

THIRDP_CUR_DIR := $(FREERTOS_SDK_DIR)/third-party

# src files
BUILD_INC_PATH_DIR += $(THIRDP_CUR_DIR)/freertos/include \
			$(THIRDP_CUR_DIR)/freertos/portable/GCC/ft_platform 
			
	ifdef CONFIG_TARGET_ARMV8_AARCH64
		BUILD_INC_PATH_DIR += $(THIRDP_CUR_DIR)/freertos/portable/GCC/ft_platform/aarch64
	endif #CONFIG_TARGET_ARMV8_AARCH64

	ifdef CONFIG_TARGET_ARMV8_AARCH32
		BUILD_INC_PATH_DIR += $(THIRDP_CUR_DIR)/freertos/portable/GCC/ft_platform/aarch32
	endif

endif