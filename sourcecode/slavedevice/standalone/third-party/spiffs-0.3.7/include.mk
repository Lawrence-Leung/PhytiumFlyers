# spiffs
ifdef CONFIG_USE_SPIFFS

BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/spiffs-0.3.7/inc \
			$(SDK_DIR)/third-party/spiffs-0.3.7/ports


ifdef CONFIG_SPIFFS_ON_FSPIM_SFUD
	BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/spiffs-0.3.7/ports/fspim
endif #CONFIG_SPIFFS_ON_FSPIM_SFUD

ifdef CONFIG_SPIFFS_ON_FQSPI_SFUD
	BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/spiffs-0.3.7/ports/fqspi
endif #CONFIG_SPIFFS_ON_FSPIM_SFUD

endif #CONFIG_USE_SPIFFS