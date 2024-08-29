ifdef CONFIG_ENABLE_FGMAC
DRIVERS_CSRCS += \
    fgmac.c\
    fgmac_dma.c\
    fgmac_g.c\
    fgmac_hw.c\
    fgmac_intr.c\
    fgmac_sinit.c

DRIVERS_CSRCS += phy/fgmac_phy.c 

ifdef CONFIG_FGMAC_PHY_AR803X
    DRIVERS_CSRCS += phy/ar803x/fgmac_ar803x.c
endif

endif

ifdef CONFIG_ENABLE_FXMAC
DRIVERS_CSRCS += \
    fxmac.c\
    fxmac_bdring.c\
    fxmac_debug.c\
    fxmac_g.c\
    fxmac_intr.c\
    fxmac_options.c\
    fxmac_phy.c\
    fxmac_sinit.c
endif

