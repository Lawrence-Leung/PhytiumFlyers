ifdef CONFIG_ENABLE_FDDMA
DRIVERS_CSRCS += \
    fddma.c\
    fddma_g.c\
    fddma_hw.c\
    fddma_intr.c\
    fddma_selftest.c\
    fddma_sinit.c
endif

ifdef CONFIG_ENABLE_FGDMA
DRIVERS_CSRCS += \
    fgdma.c\
    fgdma_g.c\
    fgdma_intr.c\
    fgdma_selftest.c\
    fgdma_sinit.c
endif

