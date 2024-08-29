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
 *
 * FilePath: lv_indev_port.h
 * Date: 2022-04-20 14:22:40
 * LastEditTime: 2023-07-06 15:40:40
 * Description:  This file is for providing the indev config
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 *  1.0  Wangzq     2023/07/06  add the device
 *  1.1  Wangzq     2023/07/07  change the third-party and driver relation 
 */

#ifndef LV_INDEV_PORT_H
#define LV_INDEV_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "ftypes.h"


/**********************
 * GLOBAL PROTOTYPES
 **********************/
void lv_port_kb_init(u32 id);

void lv_port_ms_init(u32 id);
/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif


#endif /*Disable/Enable content*/
