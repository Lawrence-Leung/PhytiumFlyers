


ifdef CONFIG_USE_SPIFFS

THIRDP_CUR_DIR := $(FREERTOS_SDK_DIR)/third-party

# src files
BUILD_INC_PATH_DIR +=  $(THIRDP_CUR_DIR)/spiffs-0.3.7/inc \
			$(THIRDP_CUR_DIR)/spiffs-0.3.7/ports 


ifdef CONFIG_SPIFFS_ON_FSPIM_SFUD
	BUILD_INC_PATH_DIR += $(THIRDP_CUR_DIR)/spiffs-0.3.7/ports/fspim
endif #CONFIG_SPIFFS_ON_FSPIM_SFUD

ifdef CONFIG_SPIFFS_ON_FQSPI_SFUD
	BUILD_INC_PATH_DIR += $(THIRDP_CUR_DIR)/spiffs-0.3.7/ports/fqspi
endif #CONFIG_SPIFFS_ON_FSPIM_SFUD

endif






