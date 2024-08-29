

include $(PROJECT_DIR)/sdkconfig
ifdef CONFIG_USE_FREERTOS
CSRCS_RELATIVE_FILES += $(wildcard  *.c)
CSRCS_RELATIVE_FILES += $(wildcard  portable/MemMang/heap_4.c)
CSRCS_RELATIVE_FILES += $(wildcard  portable/*.c)


ifdef CONFIG_TARGET_ARMV8_AARCH64
	CSRCS_RELATIVE_FILES += $(wildcard  portable/GCC/ft_platform/aarch64/*.c)
	ASRCS_RELATIVE_FILES += $(wildcard  portable/GCC/ft_platform/aarch64/*.S)
endif #CONFIG_TARGET_ARMV8_AARCH64

ifdef CONFIG_TARGET_ARMV8_AARCH32
	CSRCS_RELATIVE_FILES += $(wildcard  portable/GCC/ft_platform/aarch32/*.c)
	ASRCS_RELATIVE_FILES += $(wildcard  portable/GCC/ft_platform/aarch32/*.S)
endif #CONFIG_TARGET_ARMV8_AARCH32
endif #CONFIG_USE_FREERTOS
