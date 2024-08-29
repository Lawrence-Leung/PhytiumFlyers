
ifdef CONFIG_USE_FREERTOS

	CSRCS_RELATIVE_FILES += $(wildcard ports/arch/*.c)
		
	ifdef CONFIG_LWIP_FGMAC # src code of ports
		CSRCS_RELATIVE_FILES += $(wildcard ports/fgmac/*.c)
	endif #CONFIG_LWIP_FGMAC

	ifdef CONFIG_LWIP_FXMAC
		CSRCS_RELATIVE_FILES += $(wildcard ports/fxmac/*.c)
	endif

endif


LWIP_RT_C_DIR = $(SDK_DIR)/third-party/lwip-2.1.2

# src code of lwip
ABSOLUTE_CFILES  += $(wildcard $(LWIP_RT_C_DIR)/api/*.c) \
				$(wildcard $(LWIP_RT_C_DIR)/core/*.c) \
				$(wildcard $(LWIP_RT_C_DIR)/core/ipv4/*.c) \
				$(wildcard $(LWIP_RT_C_DIR)/core/ipv6/*.c) \
				$(wildcard $(LWIP_RT_C_DIR)/apps/if/*.c)

ABSOLUTE_CFILES  += $(LWIP_RT_C_DIR)/netif/bridgeif.c \
				$(LWIP_RT_C_DIR)/netif/bridgeif_fdb.c \
				$(LWIP_RT_C_DIR)/netif/ethernet.c \
				$(LWIP_RT_C_DIR)/netif/lowpan6.c \
				$(LWIP_RT_C_DIR)/netif/lowpan6_ble.c \
				$(LWIP_RT_C_DIR)/netif/lowpan6_common.c \
				$(LWIP_RT_C_DIR)/netif/zepif.c
	
ifdef CONFIG_USE_LWIP_APP_TFTP # src code of tftp app
	ABSOLUTE_CFILES  += $(wildcard $(LWIP_RT_C_DIR)/apps/tftp/*.c)
endif #CONFIG_USE_LWIP_APP_TFTP

ifdef CONFIG_USE_LWIP_APP_PING # src code of ping app
	ABSOLUTE_CFILES  += $(wildcard $(LWIP_RT_C_DIR)/apps/ping/*.c)
endif #CONFIG_USE_LWIP_APP_PING

ifdef CONFIG_USE_LWIP_APP_LWIPERF # src code of lwiperf app
	ABSOLUTE_CFILES  += $(wildcard $(LWIP_RT_C_DIR)/apps/lwiperf/*.c)
endif #CONFIG_USE_LWIP_APP_LWIPERF

ABSOLUTE_CFILES  += $(wildcard $(LWIP_RT_C_DIR)/ports/*.c)


