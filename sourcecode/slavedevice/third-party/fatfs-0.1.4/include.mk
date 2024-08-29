
ifdef CONFIG_USE_FATFS_0_1_4

FATFS_BM_DIR := $(SDK_DIR)/third-party/fatfs-0.1.4

BUILD_INC_PATH_DIR += $(FATFS_BM_DIR) \
		   $(FATFS_BM_DIR)/port \
		   $(FATFS_BM_DIR)/utils

endif