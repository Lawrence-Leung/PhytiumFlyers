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
 * FilePath: adc_example.c
 * Date: 2022-07-11 11:32:48
 * LastEditTime: 2022-07-11 11:32:48
 * Description:  This file is for ADC task implementations 
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2022/08/31  first commit
 */
#include <string.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "fadc.h"
#include "fadc_os.h"
#include "fcpu_info.h"
#include "fio_mux.h"
#include "fassert.h"

/* The periods assigned to the one-shot timers. */
#define ONE_SHOT_TIMER_PERIOD       ( pdMS_TO_TICKS( 60000UL ))

/* adc read period */
#define ADC_READ_PERIOD             ( pdMS_TO_TICKS( 2000UL ))

/* adc channel use, 0/1 */
#define ADC_CHANNEL_USE     FADC_CHANNEL_0

/* TESTC board, ADC_VREF = 1.25V */
#define REF_VOL 1.25

/* test task number */
#define TEST_TASK_NUM 1

static xSemaphoreHandle xCountingSemaphore;

static xTaskHandle read_handle;
static TimerHandle_t xOneShotTimer;

static FFreeRTOSAdc *os_adc_ctrl_p;

static void FFreeRTOSAdcDelete(FFreeRTOSAdc *os_adc_p);

static void FFreeRTOSAdcIntrSet(FFreeRTOSAdc *os_adc_p)
{
    u32 cpu_id;
    GetCpuId(&cpu_id);

    FAdcCtrl *instance_p = &os_adc_p->adc_ctrl;
    InterruptSetTargetCpus(instance_p->config.irq_num, cpu_id);
    InterruptSetPriority(instance_p->config.irq_num, instance_p->config.irq_prority);
    InterruptInstall(instance_p->config.irq_num, FAdcIntrHandler, instance_p, "adc");
    InterruptUmask(instance_p->config.irq_num);
}

static void FFreeRTOSAdcInitTask(void *pvParameters)
{
    /* The adc_id to use is passed in via the parameter.
    Cast this to a adc_id pointer. */
    u32 adc_id = (u32)(uintptr)pvParameters;

    FError ret = FADC_SUCCESS;

    /* set channel 0 and 1 iopad*/
#if defined(CONFIG_TARGET_E2000)
    FIOPadSetAdcMux(adc_id, ADC_CHANNEL_USE);
#endif

    /* init adc controller */
    os_adc_ctrl_p = FFreeRTOSAdcInit(adc_id);
    if (os_adc_ctrl_p == NULL)
    {
        printf("FFreeRTOSAdcInit failed!!!\n");
        goto adc_init_exit;
    }

    /* init adc interrupt handler */
    FFreeRTOSAdcIntrSet(os_adc_ctrl_p);

    /* adc config */
    FFreeRTOSAdcConfig adc_config;
    memset(&adc_config, 0, sizeof(adc_config));

    adc_config.channel = ADC_CHANNEL_USE;

    /* adc controller configuration*/
    adc_config.convert_config.convert_mode = FADC_SINGLE_CONVERT;
    adc_config.convert_config.channel_mode = FADC_MULTI_CHANNEL;
    adc_config.convert_config.convert_interval = 10;
    adc_config.convert_config.clk_div = 8;

    /* adc channel threshold configuration*/
    adc_config.threshold_config.high_threshold = 1000;
    adc_config.threshold_config.low_threshold = 0;

    /* adc channel interrupt configuration*/
    adc_config.event_type = FADC_INTR_EVENT_COVFIN;

    ret = FFreeRTOSAdcSet(os_adc_ctrl_p, &adc_config);
    if (FADC_SUCCESS != ret)
    {
        printf("FFreeRTOSAdcSet failed !!!\n");
        goto adc_init_exit;
    }

    printf("FFreeRTOSAdcInitTask execute success !!!\r\n");

    for (int i = 0; i < TEST_TASK_NUM; i++)
    {
        xSemaphoreGive(xCountingSemaphore);
    }

adc_init_exit:
    vTaskDelete(NULL);
}

static void FFreeRTOSAdcReadTask(void *pvParameters)
{
    xSemaphoreTake(xCountingSemaphore, portMAX_DELAY);

    FError ret = FADC_SUCCESS;
    float val = 0.0;
    u16 adc_val = 0;
    u16 count = 0;

    /* As per most tasks, this task is implemented in an infinite loop. */
    for (;;)
    {
        ret = FFreeRTOSAdcRead(os_adc_ctrl_p, ADC_CHANNEL_USE, &adc_val);
        if (ret == FADC_SUCCESS)
        {
            val = (float)adc_val;
            val = val * REF_VOL / 1024; /* 2^10 */
            printf("adc read success, count=%d, reg_value=%d, value=%f.\r\n", count, adc_val, val);
        }
        else
        {
            printf("adc read failed.\r\n");
        }
        count++;

        vTaskDelay(ADC_READ_PERIOD);
    }
}

static void prvOneShotTimerCallback(TimerHandle_t xTimer)
{
    /* Output a string to show the time at which the callback was executed. */
    printf("One-shot timer callback executing, will delete FFreeRTOSAdcReadTask.\r\n");

    FFreeRTOSAdcDelete(os_adc_ctrl_p);
}

/* create adc test, id is adc module number */
BaseType_t FFreeRTOSAdcCreate(u32 id)
{
    FASSERT(id < FADC_NUM);
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */
    BaseType_t timer_started = pdPASS;

    xCountingSemaphore = xSemaphoreCreateCounting(TEST_TASK_NUM, 0);
    if (xCountingSemaphore == NULL)
    {
        printf("FFreeRTOSAdcCreate xCountingSemaphore create failed.\r\n");
        return pdFAIL;
    }
    /* enter critical region */
    taskENTER_CRITICAL();
    /* adc init task */
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSAdcInitTask,  /* 任务入口函数 */
                          (const char *)"FFreeRTOSAdcInitTask",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          (void *)(uintptr)id,/* 任务入口函数参数 */
                          (UBaseType_t)1,  /* 任务的优先级 */
                          NULL); /* 任务控制 */

    /* 读adc任务 */
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSAdcReadTask,  /* 任务入口函数 */
                          (const char *)"FFreeRTOSAdcReadTask",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          NULL,/* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 1, /* 任务的优先级 */
                          (TaskHandle_t *)&read_handle); /* 任务控制 */

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

    /* exit critical region */
    taskEXIT_CRITICAL();

    return xReturn;
}

static void FFreeRTOSAdcDelete(FFreeRTOSAdc *os_adc_p)
{
    BaseType_t xReturn = pdPASS;

    /* deinit adc controller */
    FFreeRTOSAdcDeinit(os_adc_p);

    if (read_handle)
    {
        vTaskDelete(read_handle);
        vPrintf("Delete FFreeRTOSAdcReadTask success.\r\n");
    }

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

}