ifdef CONFIG_ENABLE_IOPAD
DRIVERS_CSRCS += \
    fiopad.c\
    fiopad_sinit.c\
    fiopad_hw.c\
    fiopad_g.c
endif

ifdef CONFIG_ENABLE_IOCTRL
DRIVERS_CSRCS += \
    fioctrl.c\
    fioctrl_sinit.c\
    fioctrl_g.c
endif