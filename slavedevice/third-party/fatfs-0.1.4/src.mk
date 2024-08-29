


CSRCS_RELATIVE_FILES += $(wildcard osal/*.c)

ifdef CONFIG_FATFS_USB
	CSRCS_RELATIVE_FILES += $(wildcard port/fusb/*.c)
endif

# ifdef CONFIG_FATFS_FSATA
# 	CSRCS_RELATIVE_FILES += $(wildcard port/fsata_controller/*.c)
# endif 

# ifdef CONFIG_FATFS_FSATA_PCIE
# 	CSRCS_RELATIVE_FILES += $(wildcard port/fsata_pcie/*.c)
# endif

FATFS_RT_C_DIR = $(SDK_DIR)/third-party/fatfs-0.1.4

ABSOLUTE_CFILES += $(wildcard $(FATFS_RT_C_DIR)/utils/*.c)
ABSOLUTE_CFILES += $(wildcard $(FATFS_RT_C_DIR)/port/*.c)
ABSOLUTE_CFILES += $(wildcard $(FATFS_RT_C_DIR)/*.c)

# ifdef CONFIG_FATFS_USB
# 	ABSOLUTE_CFILES += $(wildcard $(FATFS_RT_C_DIR)/port/fusb/*.c)
# endif

ifdef CONFIG_FATFS_FSATA
	ABSOLUTE_CFILES += $(wildcard $(FATFS_RT_C_DIR)/port/fsata_controller/*.c)
endif 

ifdef CONFIG_FATFS_FSATA_PCIE
	ABSOLUTE_CFILES += $(wildcard $(FATFS_RT_C_DIR)/port/fsata_pcie/*.c)
endif

ifdef CONFIG_FATFS_RAM_DISK
	ABSOLUTE_CFILES += $(wildcard $(FATFS_RT_C_DIR)/port/ram/*.c)
endif 

ifdef CONFIG_FATFS_SDMMC
	ABSOLUTE_CFILES += $(wildcard $(FATFS_RT_C_DIR)/port/sdmmc/*.c)
endif 