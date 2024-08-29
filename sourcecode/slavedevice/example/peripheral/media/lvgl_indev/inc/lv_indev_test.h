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
 * FilePath: lv_indev_test.h
 * Date: 2023-02-05 18:27:47
 * LastEditTime: 2023-07-06 11:02:47
 * Description:  This file is for providing the lvgl test config
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 *  1.0  Wangzq     2023//07/06  Modify the format and establish the version
 *  1.1  Wangzq     2023/07/07  change the third-party and driver relation 
 */

#ifndef LV_INDEV_TEST_H
#define LV_INDEV_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "ftypes.h"


/*the lvgl demo*/
void lv_demo_indev(void);

/*deinit the media */
void FFreeRTOSMediaChannelDeinit(u32 id);

/*handle the hpd event*/
void FFreeRTOSMediaHpdHandle(u32 channel, u32 width, u32 height, u32 multi_mode, u32 color_depth, u32 refresh_rate);

/*init the media */
void FFreeRTOSMediaDeviceInit(u32 channel, u32 width, u32 height, u32 multi_mode, u32 color_depth, u32 refresh_rate);
#ifdef __cplusplus
} /*extern "C"*/
#endif
#endif