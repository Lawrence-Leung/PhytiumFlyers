ifdef CONFIG_USE_FREERTOS

ABSOLUTE_CFILES += $(SDK_DIR)/third-party/sdmmc/sdmmc_cmd.c \
				   $(SDK_DIR)/third-party/sdmmc/sdmmc_common.c \
				   $(SDK_DIR)/third-party/sdmmc/sdmmc_init.c \
				   $(SDK_DIR)/third-party/sdmmc/sdmmc_io.c \
				   $(SDK_DIR)/third-party/sdmmc/sdmmc_mmc.c \
				   $(SDK_DIR)/third-party/sdmmc/sdmmc_sd.c

CSRCS_RELATIVE_FILES += $(wildcard osal/*.c)\
                        $(wildcard port/*.c) 
	ifdef CONFIG_SDMMC_USE_FSDIO
	CSRCS_RELATIVE_FILES +=	port/fsdio/fsdio_port.c	

	endif#	CONFIG_SDMMC_USE_FSDIO			
endif#CONFIG_USE_FREERTOS