ifdef CONFIG_USE_SFUD

	CSRCS_RELATIVE_FILES += $(wildcard ports/*.c)

	ifdef CONFIG_SFUD_CTRL_FSPIM
		CSRCS_RELATIVE_FILES += $(wildcard ports/fspim/*.c)
	endif

	ifdef CONFIG_SFUD_CTRL_FQSPI
		CSRCS_RELATIVE_FILES += $(wildcard ports/fqspi/*.c)
	endif

SFUD_RT_C_DIR = $(SDK_DIR)/third-party/sfud-1.1.0

ABSOLUTE_CFILES += $(wildcard $(SFUD_RT_C_DIR)/src/*.c)
				
endif #CONFIG_USE_SFUD