ifdef CONFIG_USE_LIBMETAL

THIRDP_CUR_DIR := $(FREERTOS_SDK_DIR)/third-party

	BUILD_INC_PATH_DIR += $(THIRDP_CUR_DIR)/libmetal \
				$(THIRDP_CUR_DIR)/libmetal/metal/compiler/gcc \
				$(THIRDP_CUR_DIR)/libmetal/metal/system/freertos/ft_platform


ifdef CONFIG_TARGET_ARMV8_AARCH32
	BUILD_INC_PATH_DIR +=  $(THIRDP_CUR_DIR)/libmetal/metal/processor/arm 
endif

ifdef CONFIG_TARGET_ARMV8_AARCH64
	BUILD_INC_PATH_DIR +=  $(THIRDP_CUR_DIR)/libmetal/metal/processor/aarch64 
endif

endif #CONFIG_USE_LIBMETAL
