# little-fs
ifdef CONFIG_USE_LITTLE_FS
BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/littlefs-2.4.2/inc \
		   			$(SDK_DIR)/third-party/littlefs-2.4.2/ports

# real block device
ifdef CONFIG_LITTLE_FS_ON_FSPIM_SFUD
	BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/littlefs-2.4.2/ports/fspim
endif #CONFIG_LITTLE_FS_ON_FSPIM_SFUD

# emulated block device (BD)
ifdef CONFIG_LITTLE_FS_DRY_RUN
	BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/littlefs-2.4.2/dry_run \
			   $(SDK_DIR)/third-party/littlefs-2.4.2/dry_run/ram \
			   $(SDK_DIR)/third-party/littlefs-2.4.2/dry_run/file \
			   $(SDK_DIR)/third-party/littlefs-2.4.2/ports/dry_run
endif #CONFIG_LITTLE_FS_DRY_RUN

endif #CONFIG_USE_LITTLE_FS
