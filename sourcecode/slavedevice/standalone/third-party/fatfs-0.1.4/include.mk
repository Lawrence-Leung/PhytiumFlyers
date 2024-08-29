
ifdef CONFIG_USE_FATFS_0_1_4

    BUILD_INC_PATH_DIR +=  $(SDK_DIR)/third-party/fatfs-0.1.4 \
			    $(SDK_DIR)/third-party/fatfs-0.1.4/port \
			    $(SDK_DIR)/third-party/fatfs-0.1.4/utils 

ifdef CONFIG_FATFS_RAM_DISK
	BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/fatfs-0.1.4/port/ram
endif #CONFIG_FATFS_RAM_DISK

ifdef CONFIG_FATFS_SDMMC
	BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/fatfs-0.1.4/port/sdmmc
endif #CONFIG_FATFS_SDMMC

ifdef CONFIG_USE_BAREMETAL
	BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/fatfs-0.1.4/osal
	ifdef CONFIG_FATFS_USB
		BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/fatfs-0.1.4/port/fusb
	endif

	ifdef CONFIG_FATFS_FSATA
		BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/fatfs-0.1.4/port/fsata_controller
	endif 

	ifdef CONFIG_FATFS_FSATA_PCIE
		BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/fatfs-0.1.4/port/fsata_pcie
	endif 
endif #CONFIG_USE_BAREMETAL
endif