/*
 * Copyright (c) 2022, sakumisu
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "usbh_core.h"
#include "usbh_hid.h"

#define DEV_FORMAT "/usb%d/input%d"
#define KBD_FORMAT "/usb%d/kbd%d"
#define MOUSE_FORMAT "/usb%d/mouse%d"

static uint32_t g_devinuse = 0;

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t g_hid_buf[128];

static int usbh_hid_devno_alloc(struct usbh_hid *hid_class)
{
    int devno;

    for (devno = 0; devno < 32; devno++) {
        uint32_t bitno = 1 << devno;
        if ((g_devinuse & bitno) == 0) {
            g_devinuse |= bitno;
            hid_class->minor = devno;
            return 0;
        }
    }

    return -EMFILE;
}

static void usbh_hid_devno_free(struct usbh_hid *hid_class)
{
    int devno = hid_class->minor;

    if (devno >= 0 && devno < 32) {
        g_devinuse &= ~(1 << devno);
    }
}

static int usbh_hid_get_report_descriptor(struct usbh_hid *hid_class, uint8_t *buffer)
{
    struct usb_setup_packet *setup = hid_class->hport->setup;
    int ret;

    setup->bmRequestType = USB_REQUEST_DIR_IN | USB_REQUEST_STANDARD | USB_REQUEST_RECIPIENT_INTERFACE;
    setup->bRequest = USB_REQUEST_GET_DESCRIPTOR;
    setup->wValue = HID_DESCRIPTOR_TYPE_HID_REPORT << 8;
    setup->wIndex = hid_class->intf;
    setup->wLength = 128;

    ret = usbh_control_transfer(hid_class->hport->ep0, setup, g_hid_buf);
    if (ret < 0) {
        return ret;
    }
    memcpy(buffer, g_hid_buf, ret - 8);
    return ret;
}

int usbh_hid_set_idle(struct usbh_hid *hid_class, uint8_t report_id, uint8_t duration)
{
    struct usb_setup_packet *setup = hid_class->hport->setup;

    setup->bmRequestType = USB_REQUEST_DIR_OUT | USB_REQUEST_CLASS | USB_REQUEST_RECIPIENT_INTERFACE;
    setup->bRequest = HID_REQUEST_SET_IDLE;
    setup->wValue = (duration << 8) | report_id;
    setup->wIndex = hid_class->intf;
    setup->wLength = 0;

    return usbh_control_transfer(hid_class->hport->ep0, setup, NULL);
}

int usbh_hid_get_idle(struct usbh_hid *hid_class, uint8_t *buffer)
{
    struct usb_setup_packet *setup = hid_class->hport->setup;
    int ret;

    setup->bmRequestType = USB_REQUEST_DIR_IN | USB_REQUEST_CLASS | USB_REQUEST_RECIPIENT_INTERFACE;
    setup->bRequest = HID_REQUEST_GET_IDLE;
    setup->wValue = 0;
    setup->wIndex = hid_class->intf;
    setup->wLength = 1;

    ret = usbh_control_transfer(hid_class->hport->ep0, setup, g_hid_buf);
    if (ret < 0) {
        return ret;
    }
    memcpy(buffer, g_hid_buf, 1);
    return ret;
}

int usbh_hid_set_protocol(struct usbh_hid *hid_class, uint8_t protocol)
{
    struct usb_setup_packet *setup = hid_class->hport->setup;

    setup->bmRequestType = USB_REQUEST_DIR_OUT | USB_REQUEST_CLASS | USB_REQUEST_RECIPIENT_INTERFACE;
    setup->bRequest = HID_REQUEST_SET_PROTOCOL;
    setup->wValue = protocol;
    setup->wIndex = 0;
    setup->wLength = 0;

    return usbh_control_transfer(hid_class->hport->ep0, setup, NULL);
}

int usbh_hid_connect(struct usbh_hubport *hport, uint8_t intf)
{
    struct usb_endpoint_descriptor *ep_desc;
    struct usbh_bus *usb = usbh_get_bus_of_port(hport);
    int ret;

    struct usbh_hid *hid_class = usb_malloc(sizeof(struct usbh_hid));
    if (hid_class == NULL) {
        USB_LOG_ERR("Fail to alloc hid_class\r\n");
        return -ENOMEM;
    }

    memset(hid_class, 0, sizeof(struct usbh_hid));
    usbh_hid_devno_alloc(hid_class);
    hid_class->hport = hport;
    hid_class->intf = intf;

    hport->config.intf[intf].priv = hid_class;

    // /* 0x0 = boot protocol, 0x1 = report protocol */
    // ret = usbh_hid_set_protocol(hid_class, 0x1);
    // if (ret < 0) {
    //     return ret;
    // }

    ret = usbh_hid_set_idle(hid_class, 0, 0);
    if (ret < 0) {
        USB_LOG_WRN("Do not support set idle\r\n");
    }

    ret = usbh_hid_get_report_descriptor(hid_class, hid_class->report_desc);
    if (ret < 0) {
        return ret;
    }

    for (uint8_t i = 0; i < hport->config.intf[intf].altsetting[0].intf_desc.bNumEndpoints; i++) {
        ep_desc = &hport->config.intf[intf].altsetting[0].ep[i].ep_desc;
        if (ep_desc->bEndpointAddress & 0x80) {
            usbh_hport_activate_epx(&hid_class->intin, hport, ep_desc);
        } else {
            usbh_hport_activate_epx(&hid_class->intout, hport, ep_desc);
        }
    }

    /* register hid device with different name */
    if (HID_PROTOCOL_KEYBOARD == hport->config.intf[intf].altsetting[0].intf_desc.bInterfaceProtocol)    
        snprintf(hport->config.intf[intf].devname, CONFIG_USBHOST_DEV_NAMELEN, KBD_FORMAT, usb->id, hid_class->minor);
    else if (HID_PROTOCOL_MOUSE == hport->config.intf[intf].altsetting[0].intf_desc.bInterfaceProtocol)
        snprintf(hport->config.intf[intf].devname, CONFIG_USBHOST_DEV_NAMELEN, MOUSE_FORMAT, usb->id, hid_class->minor);
    else
        snprintf(hport->config.intf[intf].devname, CONFIG_USBHOST_DEV_NAMELEN, DEV_FORMAT, usb->id, hid_class->minor);

    USB_LOG_RAW("Register HID Class:%s for USB-%d\r\n", hport->config.intf[intf].devname, usb->id);

    usbh_hid_run(hid_class);
    return ret;
}

int usbh_hid_disconnect(struct usbh_hubport *hport, uint8_t intf)
{
    int ret = 0;

    struct usbh_hid *hid_class = (struct usbh_hid *)hport->config.intf[intf].priv;

    if (hid_class) {
        usbh_hid_devno_free(hid_class);

        if (hid_class->intin) {
            usbh_pipe_free(hid_class->intin);
        }

        if (hid_class->intout) {
            usbh_pipe_free(hid_class->intout);
        }

        usbh_hid_stop(hid_class);
        memset(hid_class, 0, sizeof(struct usbh_hid));
        usb_free(hid_class);

        if (hport->config.intf[intf].devname[0] != '\0')
            USB_LOG_RAW("Unregister HID Class:%s\r\n", hport->config.intf[intf].devname);
    }

    return ret;
}

__WEAK void usbh_hid_run(struct usbh_hid *hid_class)
{

}

__WEAK void usbh_hid_stop(struct usbh_hid *hid_class)
{

}

const struct usbh_class_driver hid_class_driver = {
    .driver_name = "hid",
    .connect = usbh_hid_connect,
    .disconnect = usbh_hid_disconnect
};

CLASS_INFO_DEFINE const struct usbh_class_info hid_custom_class_info = {
    .match_flags = USB_CLASS_MATCH_INTF_CLASS,
    .class = USB_DEVICE_CLASS_HID,
    .subclass = 0x00,
    .protocol = 0x00,
    .vid = 0x00,
    .pid = 0x00,
    .class_driver = &hid_class_driver
};
