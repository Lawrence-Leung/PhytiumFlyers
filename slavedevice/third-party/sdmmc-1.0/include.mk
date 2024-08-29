

ifdef CONFIG_USE_SDMMC_CMD

BUILD_INC_PATH_DIR +=  $(SDK_DIR)/third-party/sdmmc \
                       $(SDK_DIR)/third-party/sdmmc/include 
                     

ifdef CONFIG_USE_FREERTOS

BUILD_INC_PATH_DIR += $(FREERTOS_SDK_DIR)/third-party/sdmmc-1.0/osal \
                      $(FREERTOS_SDK_DIR)/third-party/sdmmc-1.0/port 

ifdef CONFIG_SDMMC_USE_FSDIO

BUILD_INC_PATH_DIR += $(FREERTOS_SDK_DIR)/third-party/sdmmc-1.0/port/fsdio

endif#CONFIG_SDMMC_USE_FSDIO

endif #CONFIG_USE_FREERTOS

endif #CONFIG_USE_SDMMC_CMD

