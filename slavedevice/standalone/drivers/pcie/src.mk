ifdef CONFIG_ENABLE_F_PCIE
DRIVERS_CSRCS += fpcie.c\
    fpcie_debug.c \
    fpcie_g.c \
    fpcie_sinit.c \
    fpcie_caps.c \
    fpcie_intx.c \
    fpcie_ep.c \
    fpcie_dma.c \
    fpcie_misc.c

endif


ifdef CONFIG_ENABLE_FPCIE_ECAM
DRIVERS_CSRCS += fpcie_ecam_caps.c \
            fpcie_ecam_debug.c \
            fpcie_ecam_g.c \
            fpcie_ecam_intx.c \
            fpcie_ecam.c \
            fpcie_ecam_sinit.c
endif

ifdef CONFIG_ENABLE_FPCIEC
DRIVERS_CSRCS += fpciec.c \
                fpciec_sinit.c \
                fpciec_misc.c \
                fpciec_g.c \
                fpciec_ep.c \
                fpciec_dma.c
endif

