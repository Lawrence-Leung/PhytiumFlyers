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
 * FilePath: lv_port_disp.c
 * Date: 2023-02-05 18:27:47
 * LastEditTime: 2023-02-10 11:02:47
 * Description:  This file is for providing the port of lvgl config and display
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 *  1.0  Wangzq     2023/02/10  Modify the format and establish the version
 */

/**
 * @file lv_port_disp.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "ftypes.h"
#include "fparameters.h"
#include "ferror_code.h"
#include "fassert.h"
#include "finterrupt.h"
#include "fcpu_info.h"

#include "lv_port_disp.h"
#include "lv_conf.h"
#include "../lvgl.h"

#include "fdcdp_multi_display.h"



#if LV_USE_DEMO_BENCHMARK
    #include "../benchmark/lv_demo_benchmark.h"

#endif

#if LV_USE_DEMO_STRESS
    #include "../stress/lv_demo_stress.h"
#endif

#if LV_USE_DEMO_WIDGETS
    #include "../widgets/lv_demo_widgets.h"
#endif

/*********************
 *      DEFINES
 *********************/
static lv_color_int_t *rtt_fbp[FDCDP_INSTANCE_NUM] ;
static u32 multi_mode;

#define LV_HOR_RES_MAX (640) /* default value 320 */
#define LV_VER_RES_MAX (480) /* default value 240*/

static void FMediaDispFlush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);

/************************** Function Prototypes ******************************/
/**
 * @name: FMediaDispFramebuffer
 * @msg:  set the lvgl framebuffer addr and ensure the connected dp have the correct addr
 * @return null
 */
void FMediaDispFramebuffer(disp_parm *disp_config)
{
    u32 index;
    u32 start_index;
    u32 end_index;
    if (disp_config->channel == FDCDP_INSTANCE_NUM)
    {
        start_index = 0;
        end_index = FDCDP_INSTANCE_NUM;
    }
    else
    {
        start_index = disp_config->channel;
        end_index = disp_config->channel + 1;
    }
    for (index = start_index; index < end_index; index ++)
    {
        if (disp_config->connect[index] == 0)
        {
            if ((rtt_fbp[0] == NULL))
            {
                rtt_fbp[0] = (lv_color_int_t *)disp_config->fb_config[index];
            }
            else
            {
                rtt_fbp[1] = (lv_color_int_t *)disp_config->fb_config[index];
            }
        }
        else
        {
            printf("the dp %d is not conneted ,the screen can not open\r\n",index);
        }

    }

}

void FMediaLvgldispInit(disp_parm *disp_config)
{
    static lv_disp_draw_buf_t draw_buf_dsc_1;
    static lv_color_t buf_1[LV_HOR_RES_MAX * 10];                             /*A buffer for 10 rows*/
    lv_disp_draw_buf_init(&draw_buf_dsc_1, buf_1, NULL, LV_HOR_RES_MAX * 10); /*Initialize the display buffer*/
    static lv_disp_drv_t disp_drv; /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);   /*Basic initialization*/
    /*Set the resolution of the display*/
    disp_drv.hor_res = LV_HOR_RES_MAX;
    disp_drv.ver_res =  LV_VER_RES_MAX;
    multi_mode = disp_config->multi_mode;
    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = FMediaDispFlush;
    /*Set a display buffer*/
    disp_drv.draw_buf = &draw_buf_dsc_1;
    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);

    FMediaDispFramebuffer(disp_config);

    return;
}

volatile bool disp_flush_enabled = true;

/**
 * @name: disp_enable_update
 * @msg:  Enable updating the screen (the flushing process) when FMediaDispFlush() is called by LVGL
 * @return null
 */
void disp_enable_update(void)
{
    disp_flush_enabled = true;
}

/**
 * @name: disp_disable_update
 * @msg:  Disable updating the screen (the flushing process) when FMediaDispFlush() is called by LVGL
 * @return null
 */
void disp_disable_update(void)
{
    disp_flush_enabled = false;
}


/**
 * @name: FMediaDispFlush
 * @msg:  flush the framebuffer
 * @param {lv_disp_drv_t *} disp_drv is the Display Driver structure to be registered by HAL
 * @param {const lv_area_t *} area is the specific area on the display you want to flush
 * @param {lv_color_t *} color_p is the image pixel of
 * @return null
 */
static void FMediaDispFlush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    long int location = 0;
    int32_t x;
    int32_t y;
    if (disp_flush_enabled)
    {
        if (multi_mode == 0)
        {

            for (y = area->y1; y <= area->y2; y++)
            {
                for (x = area->x1; x <= area->x2; x++)
                {
                    location = (x) + (y) * LV_HOR_RES_MAX;
                    rtt_fbp[0][location] = color_p->full;
                    color_p++;
                }
            }
        }
        else if (multi_mode == 1)
        {
            for (y = area->y1; y <= area->y2; y++)
            {
                for (x = area->x1; x <= (area->x2); x++)
                {
                    if (x < area->x2 / 2)
                    {

                        location = (x) + (y) * (LV_HOR_RES_MAX);
                        rtt_fbp[0][location  * 2] = (color_p->full);
                        rtt_fbp[0][location  * 2 + 1] = (color_p->full);
                    }
                    else
                    {
                        location = (x) + (y) * (LV_HOR_RES_MAX);
                        rtt_fbp[1][(location - area->x2 / 2) * 2 ] = (color_p->full);
                        rtt_fbp[1] [(location - area->x2 / 2) * 2 + 1 ] = (color_p->full);
                    }
                    color_p++;
                }
            }
        }
        else
        {
            for (y = area->y1; y <= area->y2; y++)
            {
                for (x = area->x1; x <= (area->x2); x++)
                {
                    if (y < LV_VER_RES_MAX / 2)
                    {
                        location = (x) + (y) * (LV_HOR_RES_MAX) * 2;
                        rtt_fbp[0][location] = (color_p->full);
                        rtt_fbp[0][(location) + LV_HOR_RES_MAX] = (color_p->full);
                    }
                    else
                    {
                        location = (x) + (y - LV_VER_RES_MAX / 2) * (LV_HOR_RES_MAX) * 2  ;
                        rtt_fbp[1][location] = (color_p->full);
                        rtt_fbp[1][(location)  + LV_HOR_RES_MAX] = (color_p->full);
                    }
                    color_p++;
                }
            }
        }
    }
    lv_disp_flush_ready(disp_drv);
}


