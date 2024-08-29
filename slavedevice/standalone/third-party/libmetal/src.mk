ifdef CONFIG_USE_LIBMETAL


	CSRCS_RELATIVE_FILES += $(wildcard metal/*.c)
	CSRCS_RELATIVE_FILES += $(wildcard metal/system/generic/ft_platform/*.c)


ifdef CONFIG_TARGET_ARMV8_AARCH32
	CSRCS_RELATIVE_FILES += $(wildcard metal/system/generic/*.c)
endif

ifdef CONFIG_TARGET_ARMV8_AARCH64
	CSRCS_RELATIVE_FILES += $(wildcard metal/system/generic/*.c)
endif

endif #CONFIG_USE_LIBMETAL