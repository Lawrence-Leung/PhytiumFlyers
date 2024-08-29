ifdef CONFIG_USE_FREERTOS
	INC_DIR +=  $(STANDALONE_DIR)/third-party/sfud-1.1.0 \
				$(THIRD_PARTY_CUR_DIR)/sfud-1.1.0 \
				$(STANDALONE_DIR)/third-party/sfud-1.1.0/inc \
				$(THIRD_PARTY_CUR_DIR)/sfud-1.1.0/ports
	SRC_DIR +=  $(STANDALONE_DIR)/third-party/sfud-1.1.0 \
				$(THIRD_PARTY_CUR_DIR)/sfud-1.1.0 \
				$(STANDALONE_DIR)/third-party/sfud-1.1.0/src \
				$(THIRD_PARTY_CUR_DIR)/sfud-1.1.0/ports

	ifdef CONFIG_SFUD_CTRL_FSPIM
		INC_DIR += $(THIRD_PARTY_CUR_DIR)/sfud-1.1.0/ports/fspim
		SRC_DIR += $(THIRD_PARTY_CUR_DIR)/sfud-1.1.0/ports/fspim	
	endif

	ifdef CONFIG_SFUD_CTRL_FQSPI
		INC_DIR += $(THIRD_PARTY_CUR_DIR)/sfud-1.1.0/ports/fqspi
		SRC_DIR += $(THIRD_PARTY_CUR_DIR)/sfud-1.1.0/ports/fqspi	
	endif
endif #CONFIG_USE_FREERTOS