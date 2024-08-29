ifdef CONFIG_USE_SFUD


	BUILD_INC_PATH_DIR +=  $(SDK_DIR)/third-party/sfud-1.1.0 \
				$(SDK_DIR)/third-party/sfud-1.1.0/inc \
				$(SDK_DIR)/third-party/sfud-1.1.0/ports 

	ifdef CONFIG_SFUD_CTRL_FSPIM
		BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/sfud-1.1.0/ports/fspim
	endif

	ifdef CONFIG_SFUD_CTRL_FQSPI
		BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/sfud-1.1.0/ports/fqspi
	endif

endif #CONFIG_USE_SFUD