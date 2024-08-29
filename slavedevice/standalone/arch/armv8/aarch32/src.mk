
TOOL_PATH = $(subst ",,$(CONFIG_TOOLCHAIN_NAME))

ARCH_CSRCS := fcache.c \
				fexception.c \
				fgeneric_timer.c \
				fmmu.c \
				fsmcc.c

ARCH_ASRCS += $(TOOL_PATH)/fboot.S \
				$(TOOL_PATH)/fcrt0.S \
				$(TOOL_PATH)/fsmccc_call.S \
				$(TOOL_PATH)/fvectors.S \
				$(TOOL_PATH)/ftrace_uart.S 
