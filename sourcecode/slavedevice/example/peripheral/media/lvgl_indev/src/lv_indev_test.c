/*
 * Copyright : (C) 2023 Phytium Information Technology, Inc.
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
 * FilePath: lv_indev_test.c
 * Created Date: 2023-07-06 14:36:43
 * Last Modified: 2023-07-07 18:38:45
 * Description:  This file is for config the test
 *
 * Modify History:
 *   Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 *  1.0  Wangzq     2023/07/06  Modify the format and establish the version
 */

#include <stdio.h>
#include <stdbool.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "fassert.h"
#include "fcpu_info.h"

#include "fdcdp.h"
#include "fdc.h"
#include "fdp.h"
#include "fdp_hw.h"
#include "fdc_hw.h"
#include "fdcdp_multi_display.h"
#include "fmedia_os.h"
#include "fdcdp_multi_display.h"

#include "lv_indev_port.h"
#include "lv_port_disp.h"

/************************** Variable Definitions *****************************/
#define FMEDIA_EVT_INTR(index)             BIT(index)
#define FMEDIA_CHANNEL_0                    0
#define FMEDIA_CHANNEL_1                    1

static FFreeRTOSMedia *os_media;

static EventGroupHandle_t media_event = NULL;

static void FFreeRTOSMediaSendEvent(u32 evt_bits)
{
    FASSERT(media_event);

    BaseType_t x_result = pdFALSE;
    BaseType_t xhigher_priority_task_woken = pdFALSE;
    /*set the irq event for the task*/
    x_result = xEventGroupSetBitsFromISR(media_event, evt_bits, &xhigher_priority_task_woken);

}

static boolean FFreeRTOSMediaWaitEvent(u32 evt_bits, TickType_t wait_delay)
{
    FASSERT(media_event);

    EventBits_t event;
    event = xEventGroupWaitBits(media_event, evt_bits,
                                pdTRUE, pdFALSE, wait_delay);/*wait the irq event for the task*/
    if (event & evt_bits)
    {
        return TRUE;
    }
    return FALSE;
}

static boolean FFreeRTOSMediaClearEvent(EventGroupHandle_t pvEventGroup, const uint32_t ulBitsToClear)
{
    FASSERT(media_event);

    EventBits_t event;
    event = xEventGroupClearBits(pvEventGroup, ulBitsToClear);/*clear the intr bits*/
    return TRUE;
}


/**
 * @name: FFreeRTOSMediaHpdConnectCallback
 * @msg:  the hpd connect event
 * @param  {void} *args is the instance of dcdp
 * @param  {u32} index is the channel
 * @return Null
 */
static void FFreeRTOSMediaHpdConnectCallback(void *args, u32 index)
{
    FASSERT(args != NULL);
    FDcDp *instance_p = (FDcDp *)args;
    FDpChannelRegRead(instance_p->dp_instance_p[index].config.dp_channe_base_addr, FDP_TX_INTERRUPT); /*clear interrupt*/
    FFreeRTOSMediaSendEvent(FMEDIA_EVT_INTR(index));
    instance_p->connect_flg[index] = 1;
    printf("Dp:%d connect\r\n", index);

}


/**
 * @name: FFreeRTOSMediaHpdBreakCallback
 * @msg:  the hpd disconnect event
 * @param  {void} *args is the instance of dcdp
 * @param  {u32} index is the channel
 * @return Null
 */
static void FFreeRTOSMediaHpdBreakCallback(void *args, u32 index)
{
    FASSERT(args != NULL);
    FDcDp *instance_p = (FDcDp *)args;
    FDpChannelRegRead(instance_p->dp_instance_p[index].config.dp_channe_base_addr, FDP_TX_INTERRUPT); /*clear interrupt*/
    instance_p->connect_flg[index] = 0;
    FFreeRTOSMediaSendEvent(FMEDIA_EVT_INTR(index));
    printf("Dp:%d disconnect\r\n", index);
}

/**
 * @name: FFreeRTOSMediaAuxTimeoutCallback
 * @msg:  the aux timeout  event
 * @param  {void} *args is the instance of dcdp
 * @param  {u32} index is the channel
 * @return Null
 */
static void FFreeRTOSMediaAuxTimeoutCallback(void *args, u32 index)
{
    FASSERT(args != NULL);
    FDcDp *instance_p = (FDcDp *)args;
    FDpChannelRegRead(instance_p->dp_instance_p[index].config.dp_channe_base_addr, FDP_TX_INTERRUPT); /*clear interrupt*/
    printf("Dp:%d aux connect timeout\r\n", index);
}

/**
 * @name: FFreeRTOSMediaAuxErrorCallback
 * @msg:  the aux error  event
 * @param  {void} *args is the instance of dcdp
 * @param  {u32} index is the channel
 * @return Null
 */
static void FFreeRTOSMediaAuxErrorCallback(void *args, u32 index)
{
    FASSERT(args != NULL);
    FDcDp *instance_p = (FDcDp *)args;
    FDpChannelRegRead(instance_p->dp_instance_p[index].config.dp_channe_base_addr, FDP_TX_INTERRUPT); /*clear interrupt*/
    printf("Dp:%d aux connect error\r\n", index);
}

/**
 * @name: FFreeRTOSMediaIrqSet
 * @msg:  set the irq event and instance
 * @param {FDcDp} *instance_p is the instance of dcdp
 * @return Null
 */
static void FFreeRTOSMediaIrqSet(FDcDp *instance_p)
{
    FASSERT(instance_p != NULL);
    u32 cpu_id;
    u32 index;
    FMediaIntrConfig intr_config;
    memset(&intr_config, 0, sizeof(intr_config));

    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(instance_p->dp_instance_p[0].config.irq_num, cpu_id);/*the dc0 and dc1 have the same num of irq_num*/

    FDcDpRegisterHandler(instance_p, FDCDP_HPD_IRQ_CONNECTED, FFreeRTOSMediaHpdConnectCallback, (void *)instance_p);
    FDcDpRegisterHandler(instance_p, FDCDP_HPD_IRQ_DISCONNECTED, FFreeRTOSMediaHpdBreakCallback, (void *)instance_p);
    FDcDpRegisterHandler(instance_p, FDCDP_AUX_REPLY_TIMEOUT, FFreeRTOSMediaAuxTimeoutCallback, (void *)instance_p);
    FDcDpRegisterHandler(instance_p, FDCDP_AUX_REPLY_ERROR, FFreeRTOSMediaAuxErrorCallback, (void *)instance_p);

    InterruptSetPriority(instance_p->dp_instance_p[0].config.irq_num, FREERTOS_MEDIA_IRQ_PRIORITY);/*dp0 and dp1 have the same irq_num*/
    InterruptInstall(instance_p->dp_instance_p[0].config.irq_num, FDcDpInterruptHandler, instance_p, "media");
    InterruptUmask(instance_p->dp_instance_p[0].config.irq_num);
}

/**
 * @name: FFreeRTOSMediaIrqAllEnable
 * @msg:  enable the irq
 * @param  {FDcDp} *instance_p is the instance of dcdp
 * @return Null
 */
static void FFreeRTOSMediaIrqAllEnable(FDcDp *instance_p)
{
    int index = 0;
    FDcDpIntrEventType event_type = FDCDP_HPD_IRQ_CONNECTED;
    for (index = 0; index < FDCDP_INSTANCE_NUM; index++)
    {
        for (event_type = 0; event_type < FDCDP_INSTANCE_NUM; event_type++)
        {
            FDcDpIrqEnable(instance_p, index, event_type);
        }
    }
}

/**
 * @name: FFreeRTOSMediaDeviceInit
 * @msg:  enable the Dc and Dp
 * @param  {u32} channel is the dc channel
 * @param  {u32} width is the width
 * @param  {u32} height is the height
 * @param  {u32} multi_mode is multi display mode,0:clone,1:hor,2:ver
 * @param  {u32} color_depth is the color depth
 * @param  {u32} refresh_rate is the refresh rate of screen
 * @return Null
 */
void FFreeRTOSMediaDeviceInit(u32 channel, u32 width, u32 height, u32 multi_mode, u32 color_depth, u32 refresh_rate)
{
    os_media = FFreeRTOSMediaHwInit(channel, width, height, multi_mode, color_depth, refresh_rate);
    FASSERT_MSG(NULL == media_event, "Event group exists.");
    FASSERT_MSG((media_event = xEventGroupCreate()) != NULL, "Create event group failed.");
    FFreeRTOSMediaIrqSet(&os_media->dcdp_ctrl);
    FFreeRTOSMediaIrqAllEnable(&os_media->dcdp_ctrl);
}

/**
 * @name: FFreeRTOSMediaChannelDeinit
 * @msg:  deinit the media
 * @param  {u32} id is the channel of dcdp
 * @return Null
 */
void FFreeRTOSMediaChannelDeinit(u32 id)
{
    taskENTER_CRITICAL();
    vEventGroupDelete(media_event);
    media_event = NULL;
    FDcDpDeInitialize(&os_media->dcdp_ctrl, id);
    taskEXIT_CRITICAL(); /* allow schedule after deinit */
    return ;
}

/**
 * @name: FFreeRTOSMediaHpdHandle
 * @msg:  handle the hpd event
 * @param  {u32} channel is the dc channel
 * @param  {u32} width is the width
 * @param  {u32} height is the height
 * @param  {u32} multi_mode is multi display mode,0:clone,1:hor,2:ver
 * @param  {u32} color_depth is the color depth
 * @param  {u32} refresh_rate is the refresh rate of screen
 * @return Null
 */
void FFreeRTOSMediaHpdHandle(u32 channel, u32 width, u32 height, u32 multi_mode, u32 color_depth, u32 refresh_rate)
{
    u32 index;
    u32 ret = FMEDIA_DP_SUCCESS;
    u32 start_index;
    u32 end_index;//ensure the channel number

    if (channel == FDCDP_INSTANCE_NUM)
    {
        start_index = 0;
        end_index = channel;
    }
    else
    {
        start_index = channel;
        end_index = channel + 1;
    }
    FFreeRTOSMediaWaitEvent(FMEDIA_EVT_INTR(FMEDIA_CHANNEL_0) | FMEDIA_EVT_INTR(FMEDIA_CHANNEL_1), portMAX_DELAY);

    for (;;)
    {
        for (index = start_index; index < end_index; index++)
        {
            if (os_media->dcdp_ctrl.connect_flg[index] == 1)
            {
                ret = FFreeRTOSMediaHpdReInit(index, width, height, multi_mode, color_depth, refresh_rate);
                FFreeRTOSMediaClearEvent(media_event, FMEDIA_EVT_INTR(index));
                if (ret == FMEDIA_DP_SUCCESS)
                {
                    printf("Hpd task finish ,  reinit the dp success.\r\n");
                }
                os_media->dcdp_ctrl.connect_flg[index] == 0;
            }
        }
        vTaskDelay(200);
    }
}



