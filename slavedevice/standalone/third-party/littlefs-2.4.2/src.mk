# little-fs
ifdef CONFIG_USE_LITTLE_FS

CSRCS_RELATIVE_FILES += $(wildcard /src \/*.c)
CSRCS_RELATIVE_FILES += $(wildcard /ports \/*.c)

# real block device
ifdef CONFIG_LITTLE_FS_ON_FSPIM_SFUD
	CSRCS_RELATIVE_FILES += $(wildcard /ports/fspim/*.c)
endif #CONFIG_LITTLE_FS_ON_FSPIM_SFUD

# emulated block device (BD)
ifdef CONFIG_LITTLE_FS_DRY_RUN

	CSRCS_RELATIVE_FILES += $(wildcard /dry_run \/*.c)
					$(wildcard /dry_run/ram \/*.c)
					$(wildcard /dry_run/file \/*.c)
					$(wildcard /ports/dry_run \/*.c)
endif #CONFIG_LITTLE_FS_DRY_RUN

endif #CONFIG_USE_LITTLE_FS
