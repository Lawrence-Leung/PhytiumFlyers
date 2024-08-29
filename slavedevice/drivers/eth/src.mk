ifdef CONFIG_ENABLE_FGMAC
DRIVERS_CSRCS += \
   eth/gmac/fgmac_os.c
endif

ifdef CONFIG_ENABLE_FXMAC
DRIVERS_CSRCS += \
    eth/xmac/fxmac_os.c
endif
