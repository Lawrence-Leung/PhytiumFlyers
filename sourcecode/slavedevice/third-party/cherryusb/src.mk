
ifdef CONFIG_USE_CHERRY_USB

	CSRCS_RELATIVE_FILES += $(wildcard core/*.c)\
                            $(wildcard common/*.c) \
							$(wildcard osal/*.c) \


	ifdef CONFIG_CHERRY_USB_PORT_XHCI
		CSRCS_RELATIVE_FILES += $(wildcard port/xhci/*.c)
	endif #CONFIG_CHERRY_USB_PORT_XHCI

	ifdef CONFIG_CHERRYUSB_HOST

		ifdef CONFIG_CHERRY_USB_HOST_HUB
			CSRCS_RELATIVE_FILES += class/hub/usbh_hub.c
		endif #CONFIG_CHERRY_USB_HOST_HUB

		ifdef CONFIG_CHERRY_USB_HOST_MSC
			CSRCS_RELATIVE_FILES += class/msc/usbh_msc.c
		endif #CONFIG_CHERRY_USB_HOST_MSC

		ifdef CONFIG_CHERRY_USB_HOST_HID
			CSRCS_RELATIVE_FILES += class/hid/usbh_hid.c
		endif #CONFIG_CHERRY_USB_HOST_HID

	endif #CONFIG_CHERRYUSB_HOST
endif #CONFIG_USE_CHERRY_USB