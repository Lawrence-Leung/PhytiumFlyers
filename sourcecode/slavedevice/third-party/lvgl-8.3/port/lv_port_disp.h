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
 * FilePath: lv_port_disp.h
 * Date: 2022-09-05 17:38:05
 * LastEditTime: 2023-07-07  12:11:05
 * Description:  This file is for providing the interface of lvgl test
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 *  1.0  Wangzq     2023/03/20  Modify the format and establish the version
 *  1.1  Wangzq     2023/07/07  change the third-party and driver relation 
 * 
 */
/**
 * @file lv_port_disp.h
 *
 */

/*Copy this file as "lv_port_disp.h" and set this value to "1" to enable content*/

#ifndef LV_PORT_DISP_H
#define LV_PORT_DISP_H

#define LV_HOR_RES_MAX (640)
#define LV_VER_RES_MAX (480)

#include "ftypes.h"
#include "fparameters.h"
#include "fdcdp_multi_display.h"
#ifdef __cplusplus
extern "C"
{
#endif

/*********************
 *      INCLUDES
 *********************/
#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "lvgl-8.3/lvgl.h"
#endif


/**********************
 * GLOBAL PROTOTYPES
 **********************/
/* Enable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void FFreeRTOSDispdEnableUpdate(void);

/* Disable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void FFreeRTOSDispdDisableUpdate(void);

/* Framebuffer config*/
void FMediaDispFramebuffer(disp_parm *disp_config);

/*init the lv config and set the instance*/
void FFreeRTOSPortInit(disp_parm *disp_config);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_PORT_DISP_TEMPL_H*/

