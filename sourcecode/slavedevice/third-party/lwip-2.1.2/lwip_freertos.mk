LWIP_FREERTOS_CUR_DIR := $(FREERTOS_SDK_ROOT)/third-party

ifdef CONFIG_USE_LWIP

include $(STANDALONE_DIR)/third-party/lwip-2.1.2/lwip.mk

ifdef CONFIG_LWIP_FGMAC # src code of ports
	INC_DIR +=  $(LWIP_FREERTOS_CUR_DIR)/lwip-2.1.2/ports/fgmac \
				$(LWIP_FREERTOS_CUR_DIR)/lwip-2.1.2/ports

	SRC_DIR +=  $(LWIP_FREERTOS_CUR_DIR)/lwip-2.1.2/ports/fgmac \
				$(LWIP_FREERTOS_CUR_DIR)/lwip-2.1.2/ports
endif #CONFIG_LWIP_FGMAC

ifdef CONFIG_LWIP_FXMAC
	INC_DIR +=  $(LWIP_FREERTOS_CUR_DIR)/lwip-2.1.2/ports/fxmac \
				$(LWIP_FREERTOS_CUR_DIR)/lwip-2.1.2/ports
	SRC_DIR +=  $(LWIP_FREERTOS_CUR_DIR)/lwip-2.1.2/ports/fxmac \
				$(LWIP_FREERTOS_CUR_DIR)/lwip-2.1.2/ports
endif

INC_DIR +=  $(LWIP_FREERTOS_CUR_DIR)/lwip-2.1.2/ports/arch
SRC_DIR +=  $(LWIP_FREERTOS_CUR_DIR)/lwip-2.1.2/ports/arch

endif
