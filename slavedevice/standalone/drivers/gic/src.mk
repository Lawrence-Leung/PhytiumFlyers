ifdef CONFIG_ENABLE_GICV3
DRIVERS_CSRCS += \
    fgic.c\
    fgic_g.c\
    fgic_sinit.c
DRIVERS_ASRCS += \
    fgic_cpu_interface.S
endif

