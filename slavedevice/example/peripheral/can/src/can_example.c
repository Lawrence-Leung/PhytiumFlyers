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
 * FilePath: can_example.c
 * Date: 2022-07-11 11:32:48
 * LastEditTime: 2022-07-11 11:32:48
 * Description:  This file is for CAN task implementations 
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2022/09/23  first commit
 */
#include <string.h>
#include <stdio.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "fcan.h"
#include "fcan_os.h"
#include "fcpu_info.h"
#include "fio_mux.h"
#include "fassert.h"
#include "fdebug.h"

#define FCAN_TEST_DEBUG_TAG "FCAN_FREERTOS_TEST"
#define FCAN_TEST_DEBUG(format, ...) FT_DEBUG_PRINT_D(FCAN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCAN_TEST_INFO(format, ...) FT_DEBUG_PRINT_I(FCAN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCAN_TEST_WARN(format, ...) FT_DEBUG_PRINT_W(FCAN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCAN_TEST_ERROR(format, ...) FT_DEBUG_PRINT_E(FCAN_TEST_DEBUG_TAG, format, ##__VA_ARGS__)

/* The periods assigned to the one-shot timers. */
#define ONE_SHOT_TIMER_PERIOD       ( pdMS_TO_TICKS( 20000UL ))

/* can send period */
#define CAN_SEND_PERIOD             ( pdMS_TO_TICKS( 1000UL ))

/* can baudrate */
#define ARB_BAUD_RATE 1000000
#define DATA_BAUD_RATE 1000000

typedef struct
{
    u32 count;
    FFreeRTOSCan *os_can_p;
} FCanQueueData;

/* Declare a variable of type QueueHandle_t.  This is used to store the queue
that is accessed by all three tasks. */
static QueueHandle_t xQueue;

static xTaskHandle send_handle;
static xTaskHandle recv_handle;

static TimerHandle_t xOneShotTimer;

static FFreeRTOSCan *os_can_ctrl_p[FCAN_NUM];

static FCanFrame send_frame[FCAN_NUM];
static FCanFrame recv_frame[FCAN_NUM];

static void FFreeRTOSCanSendTask(void *pvParameters);
static void FFreeRTOSCanRecvTask(void *pvParameters);
static void FFreeRTOSCanDelete(void);

static void FCanTxIrqCallback(void *args)
{
    FFreeRTOSCan *os_can_p = (FFreeRTOSCan *)args;
    FCAN_TEST_DEBUG("Can%d irq send frame is ok.", os_can_p->can_ctrl.config.instance_id);
}

static void FCanRxIrqCallback(void *args)
{
    FFreeRTOSCan *os_can_p = (FFreeRTOSCan *)args;
    FCAN_TEST_DEBUG("Can%d irq recv frame callback.", os_can_p->can_ctrl.config.instance_id);

    static FCanQueueData xSendStructure;
    xSendStructure.os_can_p = os_can_p;

    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    xQueueSendToBackFromISR(xQueue, &xSendStructure, &xHigherPriorityTaskWoken);

    /* never call taskYIELD() form ISR! */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static FError FFreeRTOSCanIntrSet(FFreeRTOSCan *os_can_p)
{
    FError ret = FCAN_SUCCESS;

    FCanIntrEventConfig intr_event;
    memset(&intr_event, 0, sizeof(intr_event));

    intr_event.type = FCAN_INTR_EVENT_SEND;
    intr_event.handler = FCanTxIrqCallback;
    intr_event.param = (void *)os_can_p;
    ret = FFreeRTOSCanControl(os_can_p, FREERTOS_CAN_CTRL_INTR_SET, &intr_event);
    if (FCAN_SUCCESS != ret)
    {
        FCAN_TEST_ERROR("FFreeRTOSCanControl FCAN_INTR_EVENT_SEND failed.");
        return ret;
    }

    intr_event.type = FCAN_INTR_EVENT_RECV;
    intr_event.handler = FCanRxIrqCallback;
    intr_event.param = (void *)os_can_p;
    ret = FFreeRTOSCanControl(os_can_p, FREERTOS_CAN_CTRL_INTR_SET, &intr_event);
    if (FCAN_SUCCESS != ret)
    {
        FCAN_TEST_ERROR("FFreeRTOSCanControl FCAN_INTR_EVENT_RECV failed.");
        return ret;
    }

    u32 cpu_id;
    GetCpuId(&cpu_id);
    FCanCtrl *instance_p = &os_can_p->can_ctrl;
    InterruptSetTargetCpus(instance_p->config.irq_num, cpu_id);
    InterruptSetPriority(instance_p->config.irq_num, instance_p->config.irq_prority);
    InterruptInstall(instance_p->config.irq_num, FCanIntrHandler, instance_p, "can");
    InterruptUmask(instance_p->config.irq_num);

    return ret;
}


static FError FFreeRTOSCanBaudrateSet(FFreeRTOSCan *os_can_p)
{
    FError ret = FCAN_SUCCESS;

    FCanIntrEventConfig intr_event;
    memset(&intr_event, 0, sizeof(intr_event));

    FCanBaudrateConfig arb_segment_config;
    FCanBaudrateConfig data_segment_config;
    memset(&arb_segment_config, 0, sizeof(arb_segment_config));
    memset(&data_segment_config, 0, sizeof(data_segment_config));
    arb_segment_config.baudrate = ARB_BAUD_RATE;
    arb_segment_config.auto_calc = TRUE;
    arb_segment_config.segment = FCAN_ARB_SEGMENT;

    data_segment_config.baudrate = DATA_BAUD_RATE;
    data_segment_config.auto_calc = TRUE;
    data_segment_config.segment = FCAN_DATA_SEGMENT;

    ret = FFreeRTOSCanControl(os_can_p, FREERTOS_CAN_CTRL_BAUDRATE_SET, &arb_segment_config);
    if (FCAN_SUCCESS != ret)
    {
        FCAN_TEST_ERROR("FFreeRTOSCanControl arb_segment_config failed.");
        return ret;
    }

    ret = FFreeRTOSCanControl(os_can_p, FREERTOS_CAN_CTRL_BAUDRATE_SET, &data_segment_config);
    if (FCAN_SUCCESS != ret)
    {
        FCAN_TEST_ERROR("FFreeRTOSCanControl data_segment_config failed.");
        return ret;
    }
    return ret;
}


static FError FFreeRTOSCanIdMaskSet(FFreeRTOSCan *os_can_p)
{
    FError ret = FCAN_SUCCESS;

    FCanIdMaskConfig id_mask;
    memset(&id_mask, 0, sizeof(id_mask));
    for (int i = 0; i < FCAN_ACC_ID_REG_NUM; i++)
    {
        id_mask.filter_index = i;
        id_mask.id = 0;
        id_mask.mask = FCAN_ACC_IDN_MASK;
        ret = FFreeRTOSCanControl(os_can_p, FREERTOS_CAN_CTRL_ID_MASK_SET, &id_mask);
        if (FCAN_SUCCESS != ret)
        {
            FCAN_TEST_ERROR("FFreeRTOSCanControl FREERTOS_CAN_CTRL_ID_MASK_SET %d failed.", i);
            return ret;
        }
    }

    return ret;
}

static void FFreeRTOSCanInitTask(void *pvParameters)
{
    FError ret = FCAN_SUCCESS;
    BaseType_t xReturn = pdPASS;
    u32 can_id = FCAN0_ID;
    u32 tran_mode = FCAN_PROBE_NORMAL_MODE;

    for (can_id = FCAN0_ID; can_id < FCAN_NUM; can_id++)
    {
        FIOPadSetCanMux(can_id);
        
        /* init can controller */
        os_can_ctrl_p[can_id] = FFreeRTOSCanInit(can_id);
        if (os_can_ctrl_p[can_id] == NULL)
        {
            printf("FFreeRTOSCanInit %d failed!!!\r\n", can_id);
            goto can_init_exit;
        }

        /* set can baudrate */
        ret = FFreeRTOSCanBaudrateSet(os_can_ctrl_p[can_id]);
        if (FCAN_SUCCESS != ret)
        {
            printf("FFreeRTOSCanInit FFreeRTOSCanBaudrateSet failed!!!\r\n");
            goto can_init_exit;
        }

        /* set can id mask */
        ret = FFreeRTOSCanIdMaskSet(os_can_ctrl_p[can_id]);
        if (FCAN_SUCCESS != ret)
        {
            printf("FFreeRTOSCanInit FFreeRTOSCanIdMaskSet failed!!!\r\n");
            goto can_init_exit;
        }

        /* Identifier mask enable */
        ret = FFreeRTOSCanControl(os_can_ctrl_p[can_id], FREERTOS_CAN_CTRL_ID_MASK_ENABLE, NULL);
        if (FCAN_SUCCESS != ret)
        {
            FCAN_TEST_ERROR("FFreeRTOSCanControl FREERTOS_CAN_CTRL_ID_MASK_ENABLE failed.");
            goto can_init_exit;
        }

        /* init can interrupt handler */
        ret = FFreeRTOSCanIntrSet(os_can_ctrl_p[can_id]);
        if (FCAN_SUCCESS != ret)
        {
            FCAN_TEST_ERROR("FFreeRTOSCanInit FFreeRTOSCanIntrSet failed!!!");
            goto can_init_exit;
        }

        /* set can transfer mode */
        ret = FFreeRTOSCanControl(os_can_ctrl_p[can_id], FREERTOS_CAN_CTRL_MODE_SET, &tran_mode);
        if (FCAN_SUCCESS != ret)
        {
            FCAN_TEST_ERROR("FFreeRTOSCanControl FREERTOS_CAN_CTRL_MODE_SET failed.");
            goto can_init_exit;
        }

        /* enable can transfer */
        ret = FFreeRTOSCanControl(os_can_ctrl_p[can_id], FREERTOS_CAN_CTRL_ENABLE, NULL);
        if (FCAN_SUCCESS != ret)
        {
            FCAN_TEST_ERROR("FFreeRTOSCanControl FREERTOS_CAN_CTRL_ENABLE failed.");
            goto can_init_exit;
        }

    }

    printf("FFreeRTOSCanInitTask execute success !!!\r\n");

    /* can send task */
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSCanSendTask,  /* 任务入口函数 */
                          (const char *)"FFreeRTOSCanSendTask",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          NULL,/* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 5, /* 任务的优先级 */
                          (TaskHandle_t *)&send_handle); /* 任务控制 */
    if (xReturn != pdPASS)
    {
        printf("Create FFreeRTOSCanSendTask failed.\r\n");
        goto can_init_exit;
    }

    /* can recv task */
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSCanRecvTask,  /* 任务入口函数 */
                          (const char *)"FFreeRTOSCanRecvTask",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          NULL,/* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 5, /* 任务的优先级 */
                          (TaskHandle_t *)&recv_handle); /* 任务控制 */
    if (xReturn != pdPASS)
    {
        printf("Create FFreeRTOSCanRecvTask failed.\r\n");
        goto can_init_exit;
    }

can_init_exit:
    vTaskDelete(NULL);
}

static void FFreeRTOSCanRecvTask(void *pvParameters)
{
    FError ret = FCAN_SUCCESS;
    u8 count[FCAN_NUM] = {0};
    int i = 0;
    static FCanQueueData xReceiveStructure;
    FFreeRTOSCan *os_can_p;
    u32 instance_id = 0;
    /* As per most tasks, this task is implemented in an infinite loop. */
    for (;;)
    {
        /* wait recv interrupt give semphore */
        xQueueReceive(xQueue, &xReceiveStructure, portMAX_DELAY);
        os_can_p = xReceiveStructure.os_can_p;
        instance_id = os_can_p->can_ctrl.config.instance_id;
        memset(&recv_frame, 0, sizeof(FCanFrame));
        ret = FFreeRTOSCanRecv(os_can_p, &recv_frame[instance_id]);
        if (FCAN_SUCCESS == ret)
        {
            printf("\r\ncan 0 recv id is %#x.\r\n", recv_frame[instance_id].canid);
            printf("can 0 recv dlc is %d.\r\n", recv_frame[instance_id].candlc);
            printf("can 0 recv data is ");
            for (i = 0; i < recv_frame[instance_id].candlc; i++)
            {
                printf("%#x ", recv_frame[instance_id].data[i]);
                if (recv_frame[instance_id].data[i] != send_frame[FCAN1_ID - instance_id].data[i])
                {
                    FCAN_TEST_ERROR("\ncount=%d: can %d recv is equal to can%d send!!!\r\n", count[instance_id], instance_id, FCAN1_ID - instance_id);
                }
            }
            printf("\ncount=%d: can %d recv is equal to can%d send!!!\r\n", count[instance_id], instance_id, FCAN1_ID - instance_id);

            count[instance_id]++;
        }

    }
}

static void FFreeRTOSCanSendTask(void *pvParameters)
{
#define FCAN_SEND_ID 0x23
#define FCAN_SEND_LENGTH 8

    FError ret = FCAN_SUCCESS;
    u32 can_id = FCAN0_ID;

    u8 count[FCAN_NUM] = {0};
    int i = 0;

    /* As per most tasks, this task is implemented in an infinite loop. */
    for (;;)
    {
        printf("\r\ncan send task running.\r\n");
        for (can_id = FCAN0_ID; can_id <= FCAN1_ID; can_id++)
        {
            send_frame[can_id].canid = FCAN_SEND_ID + (can_id << 8);
            send_frame[can_id].canid &= CAN_SFF_MASK;
            send_frame[can_id].candlc = FCAN_SEND_LENGTH;
            for (i = 0; i < send_frame[can_id].candlc; i++)
            {
                send_frame[can_id].data[i] = i + (can_id << 4);
            }
            ret = FFreeRTOSCanSend(os_can_ctrl_p[can_id], &send_frame[can_id]);
            if (ret != FCAN_SUCCESS)
            {
                printf("can%d send failed.\n", can_id);
            }
            count[can_id]++;
        }
        vTaskDelay(CAN_SEND_PERIOD);
    }
}

static void prvOneShotTimerCallback(TimerHandle_t xTimer)
{
    /* Output a string to show the time at which the callback was executed. */
    printf("One-shot timer callback executing, will delete FFreeRTOSCanReadTask.\r\n");

    FFreeRTOSCanDelete();
}

/* create can test, can0 and can1 loopback */
BaseType_t FFreeRTOSCanCreate(void)
{
    BaseType_t xReturn = pdPASS;
    BaseType_t timer_started = pdPASS;

    /* The queue is created to hold a maximum of 32 structures of type xData. */
    xQueue = xQueueCreate(32, sizeof(FCanQueueData));
    if (xQueue == NULL)
    {
        printf("FFreeRTOSCanCreate FCanQueueData create failed.\r\n");
        return pdFAIL;
    }

    /* can init task */
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSCanInitTask,  /* 任务入口函数 */
                          (const char *)"FFreeRTOSCanInitTask",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          NULL,/* 任务入口函数参数 */
                          (UBaseType_t)1,  /* 任务的优先级 */
                          NULL); /* 任务控制 */

    /* Create the one shot software timer, storing the handle to the created
    software timer in xOneShotTimer. */
    xOneShotTimer = xTimerCreate("OneShot Software Timer",       /* Text name for the software timer - not used by FreeRTOS. */
                                 ONE_SHOT_TIMER_PERIOD,        /* The software timer's period in ticks. */
                                 pdFALSE,                      /* Setting uxAutoRealod to pdFALSE creates a one-shot software timer. */
                                 0,                            /* This example use the timer id. */
                                 prvOneShotTimerCallback);     /* The callback function to be used by the software timer being created. */

    /* Check the timers were created. */
    if (xOneShotTimer != NULL)
    {
        /* Start the software timers, using a block time of 0 (no block time).
        The scheduler has not been started yet so any block time specified here
        would be ignored anyway. */
        timer_started = xTimerStart(xOneShotTimer, 0);

        /* The implementation of xTimerStart() uses the timer command queue, and
        xTimerStart() will fail if the timer command queue gets full.  The timer
        service task does not get created until the scheduler is started, so all
        commands sent to the command queue will stay in the queue until after
        the scheduler has been started.  Check both calls to xTimerStart()
        passed. */
        if (timer_started != pdPASS)
        {
            vPrintf("CreateSoftwareTimerTasks xTimerStart failed. \r\n");
        }
    }
    else
    {
        vPrintf("CreateSoftwareTimerTasks xTimerCreate failed. \r\n");
    }

    return xReturn;
}

static void FFreeRTOSCanDelete(void)
{
    BaseType_t xReturn = pdPASS;

    if (send_handle)
    {
        vTaskDelete(send_handle);
        vPrintf("Delete FFreeRTOSCanSendTask success.\r\n");
    }

    if (recv_handle)
    {
        vTaskDelete(recv_handle);
        vPrintf("Delete FFreeRTOSCanRecvTask success.\r\n");
    }

    /* deinit can os instance */
    FFreeRTOSCanDeinit(os_can_ctrl_p[FCAN0_ID]);
    FFreeRTOSCanDeinit(os_can_ctrl_p[FCAN1_ID]);

    /* delete queue */
    vQueueDelete(xQueue);

    /* delete timer */
    xReturn = xTimerDelete(xOneShotTimer, 0);
    if (xReturn != pdPASS)
    {
        vPrintf("Delete OneShot Software Timer failed.\r\n");
    }
    else
    {
        vPrintf("Delete OneShot Software Timer success.\r\n");
    }
}