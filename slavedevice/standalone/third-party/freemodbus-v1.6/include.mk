ifdef CONFIG_USE_FREEMODBUS
	BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/freemodbus-v1.6/include \
			   $(SDK_DIR)/third-party/freemodbus-v1.6/port

	ifdef CONFIG_USE_MODBUS_ASCII
		BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/freemodbus-v1.6/ascii 
	endif
	ifdef CONFIG_USE_MODBUS_RTU
		BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/freemodbus-v1.6/rtu
	endif
	ifdef CONFIG_USE_MODBUS_TCP
		BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/freemodbus-v1.6/tcp 
	endif

endif