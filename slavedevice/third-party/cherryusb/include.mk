ifdef CONFIG_USE_CHERRY_USB

BUILD_INC_PATH_DIR += $(FREERTOS_SDK_DIR)/third-party/cherryusb/core \
					  $(FREERTOS_SDK_DIR)/third-party/cherryusb/common \
					  $(FREERTOS_SDK_DIR)/third-party/cherryusb/osal
ifdef CONFIG_CHERRY_USB_PORT_XHCI
	BUILD_INC_PATH_DIR += $(FREERTOS_SDK_DIR)/third-party/cherryusb/port/xhci
endif #CONFIG_CHERRY_USB_PORT_XHCI

ifdef CONFIG_CHERRYUSB_HOST

	ifdef CONFIG_CHERRY_USB_HOST_HUB
		BUILD_INC_PATH_DIR += $(FREERTOS_SDK_DIR)/third-party/cherryusb/class/hub
	endif #CONFIG_CHERRY_USB_HOST_HUB

	ifdef CONFIG_CHERRY_USB_HOST_MSC
		BUILD_INC_PATH_DIR += $(FREERTOS_SDK_DIR)/third-party/cherryusb/class/msc
	endif #CONFIG_CHERRY_USB_HOST_MSC

	ifdef CONFIG_CHERRY_USB_HOST_HID
		BUILD_INC_PATH_DIR += $(FREERTOS_SDK_DIR)/third-party/cherryusb/class/hid
	endif #CONFIG_CHERRY_USB_HOST_HID

	ifdef CONFIG_CHERRY_USB_HOST_VEDIO
		BUILD_INC_PATH_DIR += $(FREERTOS_SDK_DIR)/third-party/cherryusb/class/vedio
	endif #CONFIG_CHERRY_USB_HOST_VEDIO

	ifdef CONFIG_CHERRY_USB_HOST_RNDIS_WIRELESS
		BUILD_INC_PATH_DIR += $(FREERTOS_SDK_DIR)/third-party/cherryusb/class/wireless
	endif #CONFIG_CHERRY_USB_HOST_RNDIS_WIRELESS

	ifdef CONFIG_CHERRY_USB_HOST_CDC
		BUILD_INC_PATH_DIR += $(FREERTOS_SDK_DIR)/third-party/cherryusb/class/cdc
	endif #CONFIG_CHERRY_USB_HOST_CDC

endif #CONFIG_CHERRYUSB_HOST

endif