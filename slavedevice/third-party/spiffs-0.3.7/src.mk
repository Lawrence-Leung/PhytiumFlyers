

ifdef CONFIG_USE_SPIFFS

	CSRCS_RELATIVE_FILES += $(wildcard ports/*.c)
	CSRCS_RELATIVE_FILES += $(wildcard src/*.c)

	ifdef CONFIG_SPIFFS_ON_FSPIM_SFUD
		CSRCS_RELATIVE_FILES += $(wildcard ports/fspim/*.c)
	endif #CONFIG_SPIFFS_ON_FSPIM_SFUD

	ifdef CONFIG_SPIFFS_ON_FQSPI_SFUD
		CSRCS_RELATIVE_FILES += $(wildcard ports/fqspi/*.c)
	endif #CONFIG_SPIFFS_ON_FQSPI_SFUD

endif