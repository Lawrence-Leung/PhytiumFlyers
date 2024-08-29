/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
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
 * FilePath: fmedia_os.h
 * Date: 2022-08-24 16:42:19
 * LastEditTime: 2022-08-26 17:59:12
 * Description:  This file is for defining the media config and function
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  Wangzq     2022/12/20  Modify the format and establish the version
 */

#ifndef FMEDIA_OS_H
#define FMEDIA_OS_H

#include <FreeRTOS.h>
#include <semphr.h>
#include "fdcdp.h"
#include "ftypes.h"
#include "fparameters.h"
#include "event_groups.h"

#ifdef __cplusplus
extern "C"
{
#endif


/* freertos media interrupt priority */
#define FREERTOS_MEDIA_IRQ_PRIORITY IRQ_PRIORITY_VALUE_14

typedef struct
{
    FDcDp dcdp_ctrl;
    EventGroupHandle_t media_event;
} FFreeRTOSMedia;

/*init the media and return the meidia instance*/
FFreeRTOSMedia *FFreeRTOSMediaHwInit(u32 channel, u32 width, u32 height, u32 multi_mode,u32 color_depth,u32 refresh_rate);

/*hpd event to check and reinit the media*/
FError FFreeRTOSMediaHpdReInit(u32 channel, u32 width, u32 height, u32 multi_mode,u32 color_depth,u32 refresh_rate);

#ifdef __cplusplus
}
#endif

#endif