ifdef CONFIG_USE_SDMMC_CMD
BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/sdmmc/include \
		   			$(SDK_DIR)/third-party/sdmmc/port 

ifdef CONFIG_USE_BAREMETAL

	BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/sdmmc/osal
	
	ifdef CONFIG_SDMMC_USE_FSDMMC
	
	BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/sdmmc/port/fsdmmc
	endif #CONFIG_SDMMC_USE_FSDMMC

	ifdef CONFIG_SDMMC_USE_FSDIO

	BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/sdmmc/port/fsdio
	endif #CONFIG_SDMMC_USE_FSDIO

endif #CONFIG_USE_BAREMETAL

endif
