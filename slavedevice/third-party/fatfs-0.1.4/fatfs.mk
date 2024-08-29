FATFS_OS_DIR := $(FREERTOS_SDK_ROOT)/third-party/fatfs-0.1.4
FATFS_BM_DIR := $(STANDALONE_DIR)/third-party/fatfs-0.1.4

include $(FATFS_BM_DIR)/fatfs.mk

ifdef CONFIG_USE_FREERTOS
	INC_DIR += $(FATFS_OS_DIR)/osal
	SRC_DIR += $(FATFS_OS_DIR)/osal
	
	ifdef CONFIG_FATFS_FSATA
		INC_DIR += $(FATFS_OS_DIR)/port/fsata_controller
		SRC_DIR += $(FATFS_OS_DIR)/port/fsata_controller
	endif

	ifdef CONFIG_FATFS_FSATA_PCIE
		INC_DIR += $(FATFS_OS_DIR)/port/fsata_pcie
		SRC_DIR += $(FATFS_OS_DIR)/port/fsata_pcie
	endif

	ifdef CONFIG_FATFS_USB
		INC_DIR += $(FATFS_OS_DIR)/port/fusb
		SRC_DIR += $(FATFS_OS_DIR)/port/fusb
	endif
endif #CONFIG_USE_FREERTOS