
SOC_CSRCS += fcpu_info.c \
			fearly_uart.c
			
SOC_ASRCS += fcpu_asm.S

ifdef CONFIG_USE_SPINLOCK
SOC_CSRCS += fsmp.c
endif
