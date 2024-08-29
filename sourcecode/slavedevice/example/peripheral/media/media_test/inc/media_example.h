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
 * FilePath: media_example.h
 * Date: 2022-08-25 16:22:40
 * LastEditTime: 2022-07-07 15:40:40
 * Description:  This file is for defining the config and  functions
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 * 1.0  Wangzq     2022/12/20  Modify the format and establish the version
 * 1.1  Wangzq     2023/07/07  change the third-party and driver relation 
 */

#ifndef MEDIA_EXAMPLE_H
#define MEDIA_EXAMPLE_H

#include "ftypes.h"
#include "fdcdp.h"

#ifdef __cplusplus
extern "C"
{
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

typedef struct
{
    u8 Blue;
    u8 Green;
    u8 Red;
    u8 reserve;
} GraphicsTest;

/* return the input config */
InputParm *InputParaReturn(void);

FError FMediaDisplayDemo(void);

/*create the media init task*/
BaseType_t FFreeRTOSMediaCreate(void *args);

/*deinit the media*/
void FFreeRTOSMediaChannelDeinit(u32 id);

#ifdef __cplusplus
}
#endif

#endif // !