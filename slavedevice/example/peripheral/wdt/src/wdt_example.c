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
 * FilePath: wdt_example.c
 * Date: 2022-07-11 11:32:48
 * LastEditTime: 2022-07-11 11:32:48
 * Description:  This file is for wdt test example functions.
 *
 * Modify History:
 *  Ver      Who            Date           Changes
 * -----   ------         --------     --------------------------------------
 *  1.0   wangxiaodong    2022/8/9      first release
 */
#include <string.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "fparameters.h"
#include "fgeneric_timer.h"
#include "fwdt.h"
#include "fwdt_os.h"
#include "fcpu_info.h"
#include "wdt_example.h"

/* The periods assigned to the one-shot timers. */
#define ONE_SHOT_TIMER_PERIOD       ( pdMS_TO_TICKS( 15000UL ) )
#define AUTO_RELOAD_TIMER_PERIOD    ( pdMS_TO_TICKS( 1000UL ) )

/* watchdog timeout value in seconds */
#define WDT_TIMEOUT 4

/* watchdog feed period */
#define WDT_FEED_PERIOD             ( pdMS_TO_TICKS( 3000UL ))

/* test task number */
#define READ_WRITE_TASK_NUM 2
static xSemaphoreHandle xCountingSemaphore;

static xTaskHandle queue_receive_handle;
static xTaskHandle feed_handle;
static TimerHandle_t xOneShotTimer;
static TimerHandle_t xAutoReloadTimer;

/* Declare a variable of type QueueHandle_t.  This is used to store the queue
that is accessed by all three tasks. */
static QueueHandle_t xQueue;

static FFreeRTOSWdt *os_wdt_ctrl_p;

typedef struct
{
    u32 count;
    FWdtCtrl *ctrl;
} FWdtQueueData;

static void FFreeRTOSWdtDelete(FFreeRTOSWdt *os_wdt_p);

static void FFreeRTOSWdtInterruptHandler(s32 vector, void *param)
{
    FASSERT(param != NULL);
    static FWdtQueueData xSendStructure;

    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    xSendStructure.ctrl = (FWdtCtrl *)param;
    xSendStructure.count++;

    FWdtRefresh(xSendStructure.ctrl);

    printf("FFreeRTOSWdtInterruptHandler has been run %d times \r\n", xSendStructure.count);

    xQueueSendToBackFromISR(xQueue, &xSendStructure, &xHigherPriorityTaskWoken);

    /* never call taskYIELD() form ISR! */
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);

}

static void FFreeRTOSWdtInitTask(void *pvParameters)
{
    /* The wdt_id to use is passed in via the parameter.
    Cast this to a wdt_id pointer. */
    u32 wdt_id = (u32)(uintptr)pvParameters;
    u32 timeout = WDT_TIMEOUT;
    u32 cpu_id = 0;

    /* init wdt controller */
    os_wdt_ctrl_p = FFreeRTOSWdtInit(wdt_id);
    if (os_wdt_ctrl_p == NULL)
    {
        printf("FFreeRTOSWdtInit failed.\n");
        goto wdt_init_exit;
    }
    /* set wdt timeout value */
    FFreeRTOSWdtControl(os_wdt_ctrl_p, FREERTOS_WDT_CTRL_SET_TIMEOUT, &timeout);

    /* start wdt controller */
    FFreeRTOSWdtControl(os_wdt_ctrl_p, FREERTOS_WDT_CTRL_START, NULL);

    /* set wdt timeout interrupt handler */
    FWdtCtrl *pctrl = &os_wdt_ctrl_p->wdt_ctrl;
    FWdtConfig *pconfig = &os_wdt_ctrl_p->wdt_ctrl.config;
    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(pconfig->irq_num, cpu_id);
    /* interrupt init */
    InterruptSetPriority(pconfig->irq_num, pconfig->irq_prority);
    InterruptInstall(pconfig->irq_num, FFreeRTOSWdtInterruptHandler, pctrl, pconfig->instance_name);
    InterruptUmask(pconfig->irq_num);

    printf("FFreeRTOSWdtInitTask execute successfully.\r\n");

    for (int i = 0; i < READ_WRITE_TASK_NUM; i++)
    {
        xSemaphoreGive(xCountingSemaphore);
    }

wdt_init_exit:
    vTaskDelete(NULL);
}

static void FFreeRTOSWdtQueueReceiveTask(void)
{
    xSemaphoreTake(xCountingSemaphore, portMAX_DELAY);

    static FWdtQueueData xReceiveStructure;

    /* As per most tasks, this task is implemented in an infinite loop. */
    for (;;)
    {
        xQueueReceive(xQueue, &xReceiveStructure, portMAX_DELAY);
        u32 seconds = GenericTimerRead(GENERIC_TIMER_ID0) / GenericTimerFrequecy();
        vPrintf("FFreeRTOSWdtQueueReceiveTask run, count = %d, time seconds: %d\r\n", xReceiveStructure.count, seconds);
    }
}

static void FFreeRTOSWdtFeedTask(void *pvParameters)
{
    xSemaphoreTake(xCountingSemaphore, portMAX_DELAY);

    /* As per most tasks, this task is implemented in an infinite loop. */
    for (;;)
    {
        FFreeRTOSWdtControl(os_wdt_ctrl_p, FREERTOS_WDT_CTRL_KEEPALIVE, NULL);
        u32 seconds = GenericTimerRead(GENERIC_TIMER_ID0) / GenericTimerFrequecy();
        vPrintf("FFreeRTOSWdtFeedTask run, time seconds: %d\r\n", seconds);
        vTaskDelay(WDT_FEED_PERIOD);
    }
}

static void prvOneShotTimerCallback(TimerHandle_t xTimer)
{
    /* Output a string to show the time at which the callback was executed. */
    vPrintf("One-shot timer callback executing, will delete FFreeRTOSWdtFeedTask.\r\n");

    /* The count of the number of times this software timer has expired is
    stored in the timer's ID.  Obtain the ID, increment it, then save it as the
    new ID value.  The ID is a void pointer, so is cast to a uint32_t. */

    if (feed_handle)
    {
        vTaskDelete(feed_handle);
        vPrintf("Delete FFreeRTOSWdtFeedTask success.\r\n");
    }

}

static void prvAutoReloadTimerCallback(TimerHandle_t xTimer)
{
    /* Output a string to show the time at which the callback was executed. */
    static u32 count = 0;
    u32 time_left = 0;

    count++;
    /* The count of the number of times this software timer has expired is
    stored in the timer's ID.  Obtain the ID, increment it, then save it as the
    new ID value.  The ID is a void pointer, so is cast to a uint32_t. */

    if (count >= 30)
    {
        vPrintf("Auto-reload callback executing, Delete FFreeRTOSWdtQueueReceiveTask and software timer.\r\n");
        FFreeRTOSWdtDelete(os_wdt_ctrl_p);
    }
    else
    {
        FFreeRTOSWdtControl(os_wdt_ctrl_p, FREERTOS_WDT_CTRL_GET_TIMELEFT, &time_left);
        vPrintf("Auto-reload callback executing, wdt timeleft=%d\n", time_left);
    }

}

BaseType_t FFreeRTOSWdtCreate(u32 id)
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */
    BaseType_t xTimer1Started = pdPASS;
    BaseType_t xTimer2Started = pdPASS;
    u32 wdt_id = id;
    /* The queue is created to hold a maximum of 3 structures of type xData. */
    xQueue = xQueueCreate(3, sizeof(FWdtQueueData));
    if (xQueue == NULL)
    {
        printf("FFreeRTOSWdtCreate FWdtQueueData create failed.\r\n");
        return pdFAIL;
    }

    xCountingSemaphore = xSemaphoreCreateCounting(READ_WRITE_TASK_NUM, 0);
    if (xCountingSemaphore == NULL)
    {
        printf("FFreeRTOSWdtCreate xCountingSemaphore create failed.\r\n");
        return pdFAIL;
    }

    taskENTER_CRITICAL(); //进入临界区
    /* wdt init task */
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSWdtInitTask,  /* 任务入口函数 */
                          (const char *)"FFreeRTOSWdtInitTask",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          (void *)(uintptr)id,/* 任务入口函数参数 */
                          (UBaseType_t)1,  /* 任务的优先级 */
                          NULL); /* 任务控制 */

    /* 看门狗后半段的处理任务，等待处理超时中断触发后的发送的消息队列 */
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSWdtQueueReceiveTask,  /* 任务入口函数 */
                          (const char *)"FFreeRTOSWdtQueueReceiveTask",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          NULL,/* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 1, /* 任务的优先级 */
                          (TaskHandle_t *)&queue_receive_handle); /* 任务控制 */

    /* 主动喂狗任务，周期比看门狗的超时时间短 */
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSWdtFeedTask,  /* 任务入口函数 */
                          (const char *)"FFreeRTOSWdtFeedTask",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          NULL,/* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 1, /* 任务的优先级 */
                          (TaskHandle_t *)&feed_handle); /* 任务控制 */

    /* Create the one shot software timer, storing the handle to the created
    software timer in xOneShotTimer. */
    xOneShotTimer = xTimerCreate("OneShot Software Timer",       /* Text name for the software timer - not used by FreeRTOS. */
                                 ONE_SHOT_TIMER_PERIOD,        /* The software timer's period in ticks. */
                                 pdFALSE,                      /* Setting uxAutoRealod to pdFALSE creates a one-shot software timer. */
                                 0,                            /* This example use the timer id. */
                                 prvOneShotTimerCallback);     /* The callback function to be used by the software timer being created. */

    /* Create the auto-reload, storing the handle to the created timer in
    xAutoReloadTimer. */
    xAutoReloadTimer = xTimerCreate("Auto-reload Timer",         /* Text name for the software timer - not used by FreeRTOS. */
                                    AUTO_RELOAD_TIMER_PERIOD,     /* The software timer's period in ticks. */
                                    pdTRUE,                       /* Set uxAutoRealod to pdTRUE to create an auto-reload timer. */
                                    0,                            /* This example use the timer id. */
                                    prvAutoReloadTimerCallback);  /* The callback function to be used by the software timer being created. */

    /* Check the timers were created. */
    if ((xOneShotTimer != NULL) && (xAutoReloadTimer != NULL))
    {
        /* Start the software timers, using a block time of 0 (no block time).
        The scheduler has not been started yet so any block time specified here
        would be ignored anyway. */
        xTimer1Started = xTimerStart(xOneShotTimer, 0);
        xTimer2Started = xTimerStart(xAutoReloadTimer, 0);

        /* The implementation of xTimerStart() uses the timer command queue, and
        xTimerStart() will fail if the timer command queue gets full.  The timer
        service task does not get created until the scheduler is started, so all
        commands sent to the command queue will stay in the queue until after
        the scheduler has been started.  Check both calls to xTimerStart()
        passed. */
        if ((xTimer1Started != pdPASS) || (xTimer2Started != pdPASS))
        {
            vPrintf("CreateSoftwareTimerTasks xTimerStart failed.\r\n");
        }
    }
    else
    {
        vPrintf("CreateSoftwareTimerTasks xTimerCreate failed.\r\n");
    }

    taskEXIT_CRITICAL(); //退出临界区

    return xReturn;
}

static void FFreeRTOSWdtDelete(FFreeRTOSWdt *os_wdt_p)
{
    BaseType_t xReturn = pdPASS;
    FFreeRTOSWdtControl(os_wdt_p, FREERTOS_WDT_CTRL_STOP, NULL);
    FFreeRTOSWdtDeinit(os_wdt_p);
    if (queue_receive_handle)
    {
        vTaskDelete(queue_receive_handle);
        vPrintf("Delete FFreeRTOSWdtQueueReceiveTask success.\r\n");
    }

    /* delete queue */
    vQueueDelete(xQueue);

    /* delete count sem */
    vSemaphoreDelete(xCountingSemaphore);

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

    xReturn = xTimerDelete(xAutoReloadTimer, 0);
    if (xReturn != pdPASS)
    {
        vPrintf("Delete AutoReload Software Timer failed.\r\n");
    }
    else
    {
        vPrintf("Delete AutoReload Software Timer success.\r\n");
    }
}