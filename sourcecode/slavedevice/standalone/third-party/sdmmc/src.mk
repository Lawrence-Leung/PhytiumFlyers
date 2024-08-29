
CSRCS_RELATIVE_FILES += $(wildcard  *.c port/*.c)

ifdef CONFIG_USE_BAREMETAL
	CSRCS_RELATIVE_FILES += $(wildcard osal/*.c)

	ifdef CONFIG_SDMMC_USE_FSDMMC
	
	CSRCS_RELATIVE_FILES += $(wildcard port/fsdmmc/*.c)

	endif #CONFIG_SDMMC_USE_FSDMMC

	ifdef CONFIG_SDMMC_USE_FSDIO
	CSRCS_RELATIVE_FILES += $(wildcard port/fsdio/*.c)
	
	endif #CONFIG_SDMMC_USE_FSDIO

endif #CONFIG_USE_BAREMETAL

