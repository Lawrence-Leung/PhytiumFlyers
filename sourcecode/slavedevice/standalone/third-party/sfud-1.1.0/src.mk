ifdef CONFIG_USE_SFUD

	CSRCS_RELATIVE_FILES += $(wildcard *.c \
						src/*.c \
						ports/*.c)

	ifdef CONFIG_SFUD_CTRL_FSPIM
		CSRCS_RELATIVE_FILES += $(wildcard ports/fspim/*.c)
	endif

	ifdef CONFIG_SFUD_CTRL_FQSPI
		CSRCS_RELATIVE_FILES += $(wildcard ports/fqspi/*.c)
	endif

endif #CONFIG_USE_SFUD