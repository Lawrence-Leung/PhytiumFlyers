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
 * FilePath: media_example.c
 * Date: 2022-07-11 11:32:48
 * LastEditTime: 2022-07-07 10:25:48
 * Description:  This file is for testing light the screen
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  Wangzq     2022/12/20  Modify the format and establish the version
 * 1.1  Wangzq     2023/07/07  change the third-party and driver relation 
 */
#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include "task.h"
#include "ftypes.h"
#include "fassert.h"
#include "fdebug.h"
#include "fparameters_comm.h"
#include "finterrupt.h"
#include "fkernel.h"
#include "event_groups.h"
#include "fcpu_info.h"

#include "fmedia_os.h"
#include "fdcdp.h"
#include "fdp_hw.h"
#include "fdp.h"
#include"media_example.h"


/************************** Variable Definitions *****************************/
#define FMEDIA_EVT_INTR(index)             BIT(index)
#define FMEDIA_CHANNEL_0                    0
#define FMEDIA_CHANNEL_1                    1
/***************** Macros (Inline Functions) Definitions *********************/
static TaskHandle_t init_task;
static TaskHandle_t hpd_task ;

static FFreeRTOSMedia *os_media;
static InputParm *input_config;
static  EventGroupHandle_t media_event = NULL;
static GraphicsTest blt_buffer;
/************************** Function Prototypes ******************************/
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
 * @name: FDcDpIrqAllEnable
 * @msg:  enable the irq
 * @param  {FDcDp} *instance_p is the instance of dcdp
 * @return Null
 */
static void FDcDpIrqAllEnable(FDcDp *instance_p)
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
 * @name: FFreeRTOSMediaInitTask
 * @msg:  a task for init the media
 * @param {void} *pvParameters is config of instance of dcdp
 * @return Null
 */
static void FFreeRTOSMediaInitTask(void *pvParameters)
{
    input_config = (InputParm *)pvParameters;
    u32 channel_num = input_config->channel;
    u32 width = input_config->width;
    u32 height = input_config->height;
    u32 multi_mode = input_config->multi_mode;
    u32 color_depth = input_config->color_depth;
    u32 refresh_rate = input_config->refresh_rate;

    os_media = FFreeRTOSMediaHwInit(channel_num, width, height, multi_mode, color_depth, refresh_rate);
    FFreeRTOSMediaIrqSet(&os_media->dcdp_ctrl);
    FDcDpIrqAllEnable(&os_media->dcdp_ctrl);
    vTaskDelete(NULL);
}

/**
 * @name: FFreeRTOSMediaChannelDeinit
 * @msg:  deinit the media
 * @param {u32} id is channel of media
 * @return Null
 */
void FFreeRTOSMediaChannelDeinit(u32 id)
{
    taskENTER_CRITICAL();
    vEventGroupDelete(media_event);
    media_event = NULL;
    FDcDpDeInitialize(&os_media->dcdp_ctrl, id);
    taskEXIT_CRITICAL();
    return;
}


/**
 * @name: BltVideoToFill
 * @msg:  write the rgb to the dc
 * @param {FDcCtrl} *instance_p is the struct of dc
 * @param {uintptr} offset is the addr
 * @param {u32} length is the length of the pixel
 * @param {void*} config is rgb value
 * @return Null
 */
static void PhyFramebufferWrite(FDcCtrl *instance_p, uintptr offset, u32 length,  void *config)
{
    u32 Index;
    for (Index = 0; Index < length; Index++)
    {
        FtOut32(instance_p->fdc_current_config.framebuffer.framebuffer_p + offset + Index * 4, *((u32 *)(config + Index * 4)));
    }
}

/**
 * @name: BltVideoToFill
 * @msg:  fill the rgb into the dc
 * @param {FDcCtrl} *instance_p is the struct of dc
 * @param {GraphicsTest} config is the RGB value
 * @param {u32} width is the width of screen
 * @return Null
 */
static void BltVideoToFill(FDcCtrl *instance_p, GraphicsTest *config, u32 width, u32 height)
{
    FASSERT(instance_p  != NULL);
    FASSERT(config  != NULL);

    u32  ResWidth;
    u32  ResHeight;
    u32  Stride;
    u32  I;
    u32  J;
    u32  Blt;

    Stride = FDcWidthToStride(width, 32, 1);
    memcpy(&Blt, config, sizeof(GraphicsTest));
    for (I = 0; I < (height); I++)
    {
        for (J = 0; J < (width * 2); J++)
        {
            PhyFramebufferWrite(instance_p, I * Stride + J * 4, 1, &Blt);
        }
    }

}

/**
 * @name: FMediaDisplayDemo
 * @msg:  the demo for testing the media
 * @return Null
 */
FError FMediaDisplayDemo(void)
{
    FDcDp *instance_p = &os_media->dcdp_ctrl;
    InputParm *input_config = InputParaReturn();
    for (u32 index = 0; index < 2; index ++)
    {
        blt_buffer.Red = 0xff;
        blt_buffer.Green = 0xff;
        blt_buffer.Blue = 0x0;
        blt_buffer.reserve = 0;
        BltVideoToFill(&instance_p->dc_instance_p[index], &blt_buffer, input_config->width, input_config->height);
    }
}

static void FFreeRTOSMediaHpdTask(void *pvParameters)
{
    input_config = (InputParm *)pvParameters;
    u32 index;
    u32 ret = 0 ;
    u32 channel = input_config->channel;
    u32 multi_mode = input_config->multi_mode;
    u32 color_depth = input_config->color_depth;
    u32 refresh_rate = input_config->refresh_rate;
    u32 width = input_config->width;
    u32 height = input_config->height;
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

/* create media test, id is media module number */
BaseType_t FFreeRTOSMediaCreate(void *args)
{
    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为 pdPASS */
    FASSERT_MSG(NULL == media_event, "Event group exists.");
    FASSERT_MSG((media_event = xEventGroupCreate()) != NULL, "Create event group failed.");
    /* enter critical region */
    taskENTER_CRITICAL();
    /* Media init task */
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSMediaInitTask, /* 任务入口函数 */
                          (const char *)"FFreeRTOSMediaInitTask", /* 任务名字 */
                          (uint16_t)1024,                         /* 任务栈大小 */
                          (void *)args,                    /* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 2,                       /* 任务的优先级 */
                          (TaskHandle_t *)&init_task);

    /* HPD任务控制 */
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSMediaHpdTask, /* 任务入口函数 */
                          (const char *)"FFreeRTOSMediaHpdTask", /* 任务名字 */
                          (uint16_t)1024,                        /* 任务栈大小 */
                          (void *)args,                   /* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 1,                 /* 任务的优先级 */
                          (TaskHandle_t *)&hpd_task);
    /* exit critical region */
    taskEXIT_CRITICAL();
    return xReturn;
}
