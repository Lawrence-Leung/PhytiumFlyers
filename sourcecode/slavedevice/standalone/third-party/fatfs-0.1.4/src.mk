
CSRCS_RELATIVE_FILES += $(wildcard  *.c)
CSRCS_RELATIVE_FILES += $(wildcard  port/*.c)
CSRCS_RELATIVE_FILES += $(wildcard  utils/*.c)


ifdef CONFIG_FATFS_RAM_DISK
	CSRCS_RELATIVE_FILES += $(wildcard port/ram/*.c)
endif #CONFIG_FATFS_RAM_DISK

ifdef CONFIG_FATFS_SDMMC
	CSRCS_RELATIVE_FILES += $(wildcard port/sdmmc/*.c)
endif #CONFIG_FATFS_SDMMC

ifdef CONFIG_USE_BAREMETAL
	CSRCS_RELATIVE_FILES += $(wildcard osal/*.c)

	ifdef CONFIG_FATFS_USB
		CSRCS_RELATIVE_FILES += $(wildcard port/fusb/*.c)
	endif

	ifdef CONFIG_FATFS_FSATA
		CSRCS_RELATIVE_FILES += $(wildcard port/fsata_controller/*.c)
	endif 

	ifdef CONFIG_FATFS_FSATA_PCIE
		CSRCS_RELATIVE_FILES += $(wildcard port/fsata_pcie/*.c)
	endif 
endif #CONFIG_USE_BAREMETAL
