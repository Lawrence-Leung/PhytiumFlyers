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
 * FilePath: lv_indev_creat.h
 * Date: 2023-02-05 18:27:47
 * LastEditTime: 2023-07-07 11:02:47
 * Description:  This file is for providing the lvgl task
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 *  1.0  Wangzq     2023/03/20  Modify the format and establish the version
 *  1.1  Wangzq     2023/07/07  change the third-party and driver relation 
 */


#ifndef LV_INDEV_CREAT_H
#define LV_INDEV_CREAT_H

#ifdef __cplusplus
extern "C" {
#endif


/*********************
 *      INCLUDES
 *********************/
#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "lvgl-8.3/lvgl.h"
#endif

typedef struct
{
    u32 channel;
    u32 width;
    u32 height;
    u32 multi_mode;
    u32 color_depth;
    u32 refresh_rate;
} InputParm;

/*creat the media demo init task*/
BaseType_t FFreeRTOSDemoCreate(void);

/*init the keyboard*/
BaseType_t FFreeRTOSInitKbCreate(u32 id);

/*init the mouse*/
BaseType_t FFreeRTOSInitMsCreate(u32 id);

/*creat the media init task*/
BaseType_t FFreeRTOSMediaInitCreate(void *args);

/*creat the lvgl init task*/
BaseType_t FFreeRTOSlVGLConfigCreate(void *args);

/*list the usb device*/
BaseType_t FFreeRTOSListUsbDev(int argc, char *argv[]);



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif