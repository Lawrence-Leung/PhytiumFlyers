ifdef CONFIG_ENABLE_FDDMA
DRIVERS_CSRCS += \
    dma/fddma/fddma_os.c
endif

ifdef CONFIG_ENABLE_FGDMA
DRIVERS_CSRCS += \
    dma/fgdma/fgdma_os.c
endif
