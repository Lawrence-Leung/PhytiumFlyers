/*
 * Copyright (c) 2022, sakumisu
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef USBH_HUB_H
#define USBH_HUB_H

#include "usbh_core.h"
#include "usb_hub.h"

#define USBH_HUB_MAX_PORTS 4
/* Maximum size of an interrupt IN transfer */
#define USBH_HUB_INTIN_BUFSIZE ((USBH_HUB_MAX_PORTS + 8) >> 3)

#ifdef __cplusplus
extern "C" {
#endif

void usbh_roothub_thread_wakeup(uint32_t usb_id, uint8_t port);
int usbh_hub_initialize(struct usbh_bus *usb);
struct usbh_hubport *usbh_get_roothub_port (struct usbh_bus *bus, unsigned int port);

#ifdef __cplusplus
}
#endif

#endif /* USBH_HUB_H */
