/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
 * All Rights Reserved.
 *
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,
 * either version 1.0 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details.
 *
 * FilePath: lv_indev_port.c
 * Date: 2023-02-05 18:27:47
 * LastEditTime: 2023-07-07 11:02:47
 * Description:  This file is for providing the lvgl indev config
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 *  1.0  Wangzq     2023/04/20  Modify the format and establish the version
 *  1.1  Wangzq     2023/07/07  change the third-party and driver relation 
 */
/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "fsleep.h"
#include "fcache.h"
#include "fmemory_pool.h"
#include "fcpu_info.h"
#include "fassert.h"

#include "lv_indev_port.h"
#include "lv_port_disp.h"
#include "lvgl.h"
#include "usbh_core.h"
#include "usbh_hid.h"

#define HID_KEYCODE_TO_ASCII    \
    {0     , 0      }, /* 0x00 */ \
    {0     , 0      }, /* 0x01 */ \
    {0     , 0      }, /* 0x02 */ \
    {0     , 0      }, /* 0x03 */ \
    {'a'   , 'A'    }, /* 0x04 */ \
    {'b'   , 'B'    }, /* 0x05 */ \
    {'c'   , 'C'    }, /* 0x06 */ \
    {'d'   , 'D'    }, /* 0x07 */ \
    {'e'   , 'E'    }, /* 0x08 */ \
    {'f'   , 'F'    }, /* 0x09 */ \
    {'g'   , 'G'    }, /* 0x0a */ \
    {'h'   , 'H'    }, /* 0x0b */ \
    {'i'   , 'I'    }, /* 0x0c */ \
    {'j'   , 'J'    }, /* 0x0d */ \
    {'k'   , 'K'    }, /* 0x0e */ \
    {'l'   , 'L'    }, /* 0x0f */ \
    {'m'   , 'M'    }, /* 0x10 */ \
    {'n'   , 'N'    }, /* 0x11 */ \
    {'o'   , 'O'    }, /* 0x12 */ \
    {'p'   , 'P'    }, /* 0x13 */ \
    {'q'   , 'Q'    }, /* 0x14 */ \
    {'r'   , 'R'    }, /* 0x15 */ \
    {'s'   , 'S'    }, /* 0x16 */ \
    {'t'   , 'T'    }, /* 0x17 */ \
    {'u'   , 'U'    }, /* 0x18 */ \
    {'v'   , 'V'    }, /* 0x19 */ \
    {'w'   , 'W'    }, /* 0x1a */ \
    {'x'   , 'X'    }, /* 0x1b */ \
    {'y'   , 'Y'    }, /* 0x1c */ \
    {'z'   , 'Z'    }, /* 0x1d */ \
    {'1'   , '!'    }, /* 0x1e */ \
    {'2'   , '@'    }, /* 0x1f */ \
    {'3'   , '#'    }, /* 0x20 */ \
    {'4'   , '$'    }, /* 0x21 */ \
    {'5'   , '%'    }, /* 0x22 */ \
    {'6'   , '^'    }, /* 0x23 */ \
    {'7'   , '&'    }, /* 0x24 */ \
    {'8'   , '*'    }, /* 0x25 */ \
    {'9'   , '('    }, /* 0x26 */ \
    {'0'   , ')'    }, /* 0x27 */ \
    {'\r'  , '\r'   }, /* 0x28 */ \
    {'\x1b', '\x1b' }, /* 0x29 */ \
    {'\b'  , '\b'   }, /* 0x2a */ \
    {'\t'  , '\t'   }, /* 0x2b */ \
    {' '   , ' '    }, /* 0x2c */ \
    {'-'   , '_'    }, /* 0x2d */ \
    {'='   , '+'    }, /* 0x2e */ \
    {'['   , '{'    }, /* 0x2f */ \
    {']'   , '}'    }, /* 0x30 */ \
    {'\\'  , '|'    }, /* 0x31 */ \
    {'#'   , '~'    }, /* 0x32 */ \
    {';'   , ':'    }, /* 0x33 */ \
    {'\''  , '\"'   }, /* 0x34 */ \
    {'`'   , '~'    }, /* 0x35 */ \
    {','   , '<'    }, /* 0x36 */ \
    {'.'   , '>'    }, /* 0x37 */ \
    {'/'   , '?'    }, /* 0x38 */ \
                                  \
    {0     , 0      }, /* 0x39 */ \
    {0     , 0      }, /* 0x3a */ \
    {0     , 0      }, /* 0x3b */ \
    {0     , 0      }, /* 0x3c */ \
    {0     , 0      }, /* 0x3d */ \
    {0     , 0      }, /* 0x3e */ \
    {0     , 0      }, /* 0x3f */ \
    {0     , 0      }, /* 0x40 */ \
    {0     , 0      }, /* 0x41 */ \
    {0     , 0      }, /* 0x42 */ \
    {0     , 0      }, /* 0x43 */ \
    {0     , 0      }, /* 0x44 */ \
    {0     , 0      }, /* 0x45 */ \
    {0     , 0      }, /* 0x46 */ \
    {0     , 0      }, /* 0x47 */ \
    {0     , 0      }, /* 0x48 */ \
    {0     , 0      }, /* 0x49 */ \
    {0     , 0      }, /* 0x4a */ \
    {0     , 0      }, /* 0x4b */ \
    {0     , 0      }, /* 0x4c */ \
    {0     , 0      }, /* 0x4d */ \
    {0     , 0      }, /* 0x4e */ \
    {0     , 0      }, /* 0x4f */ \
    {0     , 0      }, /* 0x50 */ \
    {0     , 0      }, /* 0x51 */ \
    {0     , 0      }, /* 0x52 */ \
    {0     , 0      }, /* 0x53 */ \
                                  \
    {'/'   , '/'    }, /* 0x54 */ \
    {'*'   , '*'    }, /* 0x55 */ \
    {'-'   , '-'    }, /* 0x56 */ \
    {'+'   , '+'    }, /* 0x57 */ \
    {'\r'  , '\r'   }, /* 0x58 */ \
    {'1'   , '1'    }, /* 0x59 */ \
    {'2'   , '2'    }, /* 0x5a */ \
    {'3'   , '3'    }, /* 0x5b */ \
    {'4'   , '4'    }, /* 0x5c */ \
    {'5'   , '5'    }, /* 0x5d */ \
    {'6'   , '6'    }, /* 0x5e */ \
    {'7'   , '7'    }, /* 0x5f */ \
    {'8'   , '8'    }, /* 0x60 */ \
    {'9'   , '9'    }, /* 0x61 */ \
    {'0'   , 0      }, /* 0x62 */ \
    {'0'   , 0      }, /* 0x63 */ \
    {'='   , '='    }, /* 0x67 */ \


/************************** Variable Definitions *****************************/
#define FUSB_MEMP_TOTAL_SIZE     SZ_1M

static char *keyborad_name = "/usb0/kbd0";
static char *mouse_name = "/usb1/mouse1";

static FMemp memp;
static u8 memp_buf[FUSB_MEMP_TOTAL_SIZE];
static struct usbh_bus usb[FUSB3_NUM];
static struct usbh_urb kbd_intin_urb;
static bool botton_press_state = false;

static struct usbh_urb mouse_intin_urb;
static uint8_t mouse_buffer[128] __attribute__((aligned(sizeof(unsigned long)))) = {0};
static uint8_t kbd_buffer[128] __attribute__((aligned(sizeof(unsigned long)))) = {0};
static u8 const keycode2ascii[128][2] =  { HID_KEYCODE_TO_ASCII };

static f32 mouse_xdisp;
static f32 mouse_ydisp;
static uint8_t mouse_wdisp;/*reserved,w is the botton of mouse*/
static u8 keyborad_code;

lv_indev_t *indev_mouse;
lv_indev_t *indev_keypad;

static void mouse_init(u32 id);
static void mouse_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);
static bool mouse_is_pressed(void);
static void mouse_get_xy(lv_coord_t *x, lv_coord_t *y);
static void keypad_init(u32 id);
static void keypad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);
static uint32_t keypad_get_key(void);

/************************** Function Prototypes ******************************/
extern void USBH_IRQHandler(void *);

static void UsbHcInterrruptHandler(s32 vector, void *param)
{
    USBH_IRQHandler(param);
}

static u8 UsbInputGetInterfaceProtocol(struct usbh_hid *hid_class)
{
    struct usbh_hubport *hport = hid_class->hport;
    u8 intf = hid_class->intf;
    const struct usb_interface_descriptor *intf_desc = &hport->config.intf->altsetting[intf].intf_desc;

    return intf_desc->bInterfaceProtocol;
}

static void UsbHcSetupInterrupt(u32 id)
{
    u32 cpu_id;
    u32 irq_num = (id == FUSB3_ID_0) ? FUSB3_0_IRQ_NUM : FUSB3_1_IRQ_NUM;
    u32 irq_priority = 13U;

    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(irq_num, cpu_id);

    InterruptSetPriority(irq_num, irq_priority);

    /* register intr callback */
    InterruptInstall(irq_num,
                     UsbHcInterrruptHandler,
                     &usb[id],
                     NULL);

    /* enable irq */
    InterruptUmask(irq_num);
}

/*********************************hardware configure********************************************/
void UsbHcSetupMemp(void)
{
    if (FT_COMPONENT_IS_READY != memp.is_ready)
    {
        USB_ASSERT(FT_SUCCESS == FMempInit(&memp, &memp_buf[0], &memp_buf[0] + FUSB_MEMP_TOTAL_SIZE));
    }
}

/* implement cherryusb weak functions */
void usb_hc_low_level_init(uint32_t id)
{
    UsbHcSetupMemp();
    UsbHcSetupInterrupt(id);
}

unsigned long usb_hc_get_register_base(uint32_t id)
{
    if (FUSB3_ID_0 == id)
    {
        return FUSB3_0_BASE_ADDR + FUSB3_XHCI_OFFSET;
    }
    else
    {
        return FUSB3_1_BASE_ADDR + FUSB3_XHCI_OFFSET;
    }
}

void *usb_hc_malloc(size_t size)
{
    return usb_hc_malloc_align(sizeof(void *), size);
}

void *usb_hc_malloc_align(size_t align, size_t size)
{
    void *result = FMempMallocAlign(&memp, size, align);

    if (result)
    {
        memset(result, 0U, size);
    }
    return result;
}

void usb_hc_free(void *ptr)
{
    if (NULL != ptr)
    {
        FMempFree(&memp, ptr);
    }
}

void usb_assert(const char *filename, int linenum)
{
    FAssert(filename, linenum, 0xff);
}

void usb_hc_dcache_invalidate(void *addr, unsigned long len)
{
    FCacheDCacheInvalidateRange((uintptr)addr, len);
}

/*********************************usb indev function********************************************/
/* look up new key in previous keys */
static inline boolean FindKeyInPrevInput(const struct usb_hid_kbd_report *report, u8 keycode)
{
    for (u8 i = 0; i < 6; i++)
    {
        if (report->key[i] == keycode)
        {
            return TRUE;
        }
    }
    return FALSE;
}

static void UsbKeyBoardHandleInput(struct usb_hid_kbd_report *input)
{
    static struct usb_hid_kbd_report prev_input = { 0, 0, {0} }; /* previous report to check key released */

    /* ------------- example code ignore control (non-printable) key affects ------------- */
    for (u8 i = 0; i < 6; i++)
    {
        if (input->key[i])
        {
            if (FindKeyInPrevInput(&prev_input, input->key[i]))
            {
                /* exist in previous report means the current key is holding */
            }
            else
            {
                /* not existed in previous report means the current key is pressed */
                boolean is_shift = input->modifier & (HID_MODIFER_LSHIFT | HID_MODIFER_RSHIFT);
                keyborad_code = keycode2ascii[input->key[i]][is_shift ? 1 : 0];
                fflush(stdout); /* flush right away, else libc will wait for newline */
            }
        }
    }
    prev_input = *input;
}

static void UsbKeyboardCallback(void *arg, int nbytes)
{
    u8 intf_protocol;
    struct usbh_hid *kbd_class = (struct usbh_hid *)arg;
    intf_protocol = UsbInputGetInterfaceProtocol(kbd_class);

    if (HID_PROTOCOL_KEYBOARD == intf_protocol) /* handle input from keyboard */
    {
        if (nbytes < (int)sizeof(struct usb_hid_kbd_report))
        {
            printf("nbytes = %d", nbytes);
        }
        else
        {
            UsbKeyBoardHandleInput((struct usb_hid_kbd_report *)kbd_buffer);
        }
    }
    else
    {
        printf("mismatch callback for protocol-%d", intf_protocol);
        return;
    }
}

static inline void UsbMouseLeftButtonCB(void)
{
    printf("<-\r\n");
    botton_press_state = true;
}

static inline void UsbMouseRightButtonCB(void)
{
    printf("->\r\n");
    botton_press_state = true;
}

static inline void UsbMouseMiddleButtonCB(void)
{
    printf("C\r\n");
    botton_press_state = true;
}

static void UsbMouseHandleInput(struct usb_hid_mouse_report *input)
{
    /*------------- button state  -------------*/
    if (input->buttons & HID_MOUSE_INPUT_BUTTON_LEFT)
    {
        UsbMouseLeftButtonCB();
    }

    if (input->buttons & HID_MOUSE_INPUT_BUTTON_MIDDLE)
    {
        UsbMouseMiddleButtonCB();
    }

    if (input->buttons & HID_MOUSE_INPUT_BUTTON_RIGHT)
    {
        UsbMouseRightButtonCB();
    }

    mouse_xdisp = (f32)(LV_HOR_RES_MAX / 256.0) * input->xdisp + mouse_xdisp;/*relative to last position*/
    mouse_ydisp = (f32)(LV_VER_RES_MAX / 256.0) * input->ydisp + mouse_ydisp;

    if (mouse_xdisp < 0)
    {
        mouse_xdisp = 0;/*boundary of the screen*/
    }
    if (mouse_ydisp < 0)
    {
        mouse_ydisp = 0;
    }
    if (mouse_xdisp > LV_HOR_RES_MAX)
    {
        mouse_xdisp = LV_HOR_RES_MAX;
    }
    if (mouse_ydisp > LV_VER_RES_MAX)
    {
        mouse_ydisp = LV_VER_RES_MAX;
    }
}

static void UsbMouseCallback(void *arg, int nbytes)
{
    u8 intf_protocol;
    struct usbh_hid *mouse_class = (struct usbh_hid *)arg;
    intf_protocol = UsbInputGetInterfaceProtocol(mouse_class);

    if (HID_PROTOCOL_MOUSE == intf_protocol) /* handle input from mouse */
    {
        if (nbytes < (int)sizeof(struct usb_hid_mouse_report))
        {
            printf("nbytes = %d", nbytes);
        }
        else
        {
            UsbMouseHandleInput((struct usb_hid_mouse_report *)mouse_buffer);
        }
    }
    else
    {
        printf("mismatch callback for protocol-%d", intf_protocol);
        return;
    }
}

/*********************************usb indev init********************************************/

void lv_port_kb_init(u32 id)
{
    static lv_indev_drv_t indev_drv_keyborad;
    keypad_init(id);
    /*Register a keypad input device*/
    lv_indev_drv_init(&indev_drv_keyborad);
    indev_drv_keyborad.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv_keyborad.read_cb = keypad_read;
    indev_keypad = lv_indev_drv_register(&indev_drv_keyborad);
}

void lv_port_ms_init(u32 id)
{
    static  lv_indev_drv_t indev_drv_mouse;
    LV_IMG_DECLARE(lv_shubiao);
    mouse_init(id);
    /*Register a mouse input device*/
    lv_indev_drv_init(&indev_drv_mouse);
    indev_drv_mouse.type = LV_INDEV_TYPE_POINTER;
    indev_drv_mouse.read_cb = mouse_read;
    indev_mouse = lv_indev_drv_register(&indev_drv_mouse);
    /*Set cursor. For simplicity set a HOME symbol now.*/
    lv_obj_t *mouse_cursor = lv_img_create(lv_scr_act());
    lv_img_set_src(mouse_cursor, &lv_shubiao);
    lv_img_set_angle(mouse_cursor, 0);
    lv_img_set_zoom(mouse_cursor, 128);
    lv_indev_set_cursor(indev_mouse, mouse_cursor);
}

/*Initialize your mouse*/
static void mouse_init(u32 id)
{
    u32 ret = 0;
    memset(&usb[id], 0, sizeof(usb[id]));
    ret = usbh_initialize(id, &usb[id]);
    if (0 != ret)
    {
        printf("init usb failed \r\n");
    }
    else
    {
        printf("init usb successd \r\n");
    }
}

/*Will be called by the library to read the mouse*/
static void mouse_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    /*Get the current x and y coordinates*/
    mouse_get_xy(&data->point.x, &data->point.y);

    /*Get whether the mouse button is pressed or released*/
    if (mouse_is_pressed())
    {
        data->state = LV_INDEV_STATE_PR;
        botton_press_state = false; /*reserved, add event to control your demo*/
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }
}

/*Return true is the mouse button is pressed*/
static bool mouse_is_pressed(void)
{
    if (true == botton_press_state)
    {
        printf("botton_press_state \r\n");
        return true;
    }
    else
    {
        return false;
    }
}

/*Get the x and y coordinates if the mouse is pressed*/
static void mouse_get_xy(lv_coord_t *x, lv_coord_t *y)
{
    struct usbh_hid *mouse_class;
    u8 intf_protocol;
    mouse_class = (struct usbh_hid *)usbh_find_class_instance(mouse_name);

    if (mouse_class == NULL)
    {
        printf("mouse_class null \r\n");
        vTaskDelete(NULL);
        return;
    }
    else
    {
        usbh_int_urb_fill(&mouse_intin_urb, mouse_class->intin, mouse_buffer, 8, 0, UsbMouseCallback, mouse_class);
        usbh_submit_urb(&mouse_intin_urb);
        (*x) = mouse_xdisp;
        (*y) = mouse_ydisp;
    }
}

/*Initialize your keypad*/
static void keypad_init(u32 id)
{
    u32 ret = 0;
    memset(&usb[id], 0, sizeof(usb[id]));
    ret = usbh_initialize(id, &usb[id]);
    if (0 != ret)
    {
        printf("init usb failed \r\n");
    }
    else
    {
        printf("init usb successd \r\n");
    }
}

/*Will be called by the library to read the mouse*/
static void keypad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    static uint32_t last_key = 0;

    /*Get whether the a key is pressed and save the pressed key*/
    uint32_t act_key = keypad_get_key();
    keyborad_code = 0;
    if (act_key != 0)
    {
        data->state = LV_INDEV_STATE_PR;
        /*Translate the keys to LVGL control characters according to your key definitions*/
        last_key = act_key;
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }
    data->key = last_key;
}

/*Get the currently being pressed key.  0 if no key is pressed*/
static uint32_t keypad_get_key(void)
{
    /*Your code comes here*/
    struct usbh_hid *kbd_class;
    u8 intf_protocol;

    kbd_class = (struct usbh_hid *)usbh_find_class_instance(keyborad_name);
    if (kbd_class == NULL)
    {
        printf("kbd_class null \r\n");
        vTaskDelete(NULL);
        return 0;
    }
    else
    {
        usbh_int_urb_fill(&kbd_intin_urb, kbd_class->intin, kbd_buffer, 8, 0, UsbKeyboardCallback, kbd_class);
        usbh_submit_urb(&kbd_intin_urb);
        return keyborad_code;
    }

    return 0;
}

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
