ifdef CONFIG_ENABLE_FSDIO
DRIVERS_CSRCS += \
    fsdio.c\
    fsdio_cmd.c\
    fsdio_dma.c\
    fsdio_g.c\
    fsdio_intr.c\
    fsdio_pio.c\
    fsdio_selftest.c\
    fsdio_sinit.c
endif

ifdef CONFIG_ENABLE_FSDMMC
DRIVERS_CSRCS += \
    fsdmmc.c\
    fsdmmc_dma.c\
    fsdmmc_g.c\
    fsdmmc_hw.c\
    fsdmmc_intr.c\
    fsdmmc_sinit.c
endif

