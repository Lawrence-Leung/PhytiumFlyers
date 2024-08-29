ifdef CONFIG_USE_FREEMODBUS

	CSRCS_RELATIVE_FILES += $(wildcard *.c) 
	CSRCS_RELATIVE_FILES += $(wildcard functions/*.c)
	CSRCS_RELATIVE_FILES += $(wildcard port/*.c)
	
	ifdef CONFIG_USE_MODBUS_ASCII
		CSRCS_RELATIVE_FILES += $(wildcard ascii/*.c)
	endif
	ifdef CONFIG_USE_MODBUS_RTU
		CSRCS_RELATIVE_FILES += $(wildcard rtu/*.c)
	endif
	ifdef CONFIG_USE_MODBUS_TCP
		CSRCS_RELATIVE_FILES += $(wildcard tcp /*.c)
	endif
endif