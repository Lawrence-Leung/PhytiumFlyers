ifdef CONFIG_USE_LIBMETAL


	BUILD_INC_PATH_DIR +=  $(SDK_DIR)/third-party/libmetal \
				$(SDK_DIR)/third-party/libmetal/metal/compiler/gcc \
				$(SDK_DIR)/third-party/libmetal/metal/system/generic/ft_platform


ifdef CONFIG_TARGET_ARMV8_AARCH32
	BUILD_INC_PATH_DIR +=  $(SDK_DIR)/third-party/libmetal/metal/processor/arm 
endif

ifdef CONFIG_TARGET_ARMV8_AARCH64
	BUILD_INC_PATH_DIR +=  $(SDK_DIR)/third-party/libmetal/metal/processor/aarch64 
endif

endif #CONFIG_USE_LIBMETAL