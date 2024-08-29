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
 * FilePath: lv_demo_creat.c
 * Date: 2023-02-05 18:27:47
 * LastEditTime: 2023-07-06 11:02:47
 * Description:  This file is for providing the port of creating lvgl demo
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 *  1.0  Wangzq     2023/03/20  Modify the format and establish the version
 *  1.1  Wangzq     2023/07/06   adapt the sdk and change the lvgl config
 */

#include <string.h>
#include <stdio.h>
#include "strto.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "fassert.h"
#include "fkernel.h"

#include "lv_demo_creat.h"
#include "lv_demo_test.h"
#include "lv_port_disp.h"
#include "fdcdp_multi_display.h"


#if LV_USE_DEMO_BENCHMARK
    #include "lv_demo_benchmark.h"
#endif

#if LV_USE_DEMO_STRESS
    #include "lv_demo_stress.h"
#endif

#if LV_USE_DEMO_WIDGETS
    #include "lv_demo_widgets.h"
#endif

/************************** Constant Definitions *****************************/
#define LVGL_HEART_TIMER_PERIOD        (pdMS_TO_TICKS(1UL))
#define LVGL_CONTINUE_TIMER             10000000
/************************** Variable Definitions *****************************/
static TimerHandle_t xLvglHeartTimer;
static TaskHandle_t demo_task;
static TaskHandle_t lvgl_init_task;
static TaskHandle_t init_task;
static TaskHandle_t hpd_task ;

static disp_parm *disp_config;
static InputParm *input_config;

extern void FFreeRTOSDispdEnableUpdate(void);
extern void FFreeRTOSDispdDisableUpdate(void);
/************************** Function Prototypes ******************************/
static void LvglHeartTimerCallback(TimerHandle_t xTimer)
{
    lv_tick_inc(1);
}

#if LV_USE_DEMO_BENCHMARK

static void on_benchmark_finished(void)
{
    FFreeRTOSDispdEnableUpdate();
    printf("task is over\r\n");
}

void benchmark(void)
{
    printf("benchmark is running\r\n");
    FFreeRTOSDispdDisableUpdate();
    lv_demo_benchmark_set_finished_cb(&on_benchmark_finished);
    lv_demo_benchmark_set_max_speed(true);
    lv_demo_benchmark();
    FFreeRTOSDispdEnableUpdate();

    while (1)
    {
        if (lv_disp_get_inactive_time(NULL) < LVGL_CONTINUE_TIMER)
        {
            lv_timer_handler(); //! run lv task
        }
        else
        {
            printf("task is over \r\n");
            break;
        }
        vTaskDelay(1);
    }
}

#endif

#if LV_USE_DEMO_STRESS
void stress(void)
{
    printf("stress is runnint\r\n");
    lv_demo_stress();
    /* loop once to allow objects to be created */
    while (1)
    {
        if (lv_disp_get_inactive_time(NULL) < LVGL_CONTINUE_TIMER)
        {
            lv_timer_handler(); //! run lv task

        }
        else
        {
            printf("task is over \n");
            break;
        }
        vTaskDelay(1);
    }
}

#endif

#if LV_USE_DEMO_WIDGETS
void widgets(void)
{
    printf("widgets is runnint\r\n");
    lv_demo_widgets();
    while (1)
    {
        if (lv_disp_get_inactive_time(NULL) < LVGL_CONTINUE_TIMER)
        {
            lv_timer_handler(); //! run lv task
        }
        else
        {
            printf("task is over \n");
            break;
        }
        vTaskDelay(1);
    }
}
#endif

/**
 * @name: FFreeRTOSLVGLDemoTask
 * @msg:  run the lvgl demo
 * @return Null
 */
static void FFreeRTOSLVGLDemoTask(void)
{
    for (;;)
    {
#if LV_USE_DEMO_BENCHMARK
        benchmark();
#endif
#if LV_USE_DEMO_STRESS
        stress();
#endif
#if LV_USE_DEMO_WIDGETS
        widgets();
#endif
        vTaskDelay(1);
    }
}

/**
 * @name: FFreeRTOSMediaInitTask
 * @msg:  init the lvgl device
 * @param  {void *} pvParameters is the parameters of demo
 * @return Null
 */
static void FFreeRTOSMediaInitTask(void *pvParameters)
{
    FASSERT(NULL != pvParameters);
    InputParm *input_config = (InputParm *)pvParameters ;
    FFreeRTOSMediaDeviceInit(input_config->channel, input_config->width, input_config->height, input_config->multi_mode, input_config->color_depth, input_config->refresh_rate);
    vTaskDelete(NULL);
}

/**
 * @name: FFreeRTOSMediaHpdTask
 * @msg:  handle the hpd event
 * @param  {void *} pvParameters is the parameters of demo
 * @return Null
 */
static void FFreeRTOSMediaHpdTask(void *pvParameters)
{
    FASSERT(NULL != pvParameters);
    InputParm *input_config = (InputParm *)pvParameters ;
    FFreeRTOSMediaHpdHandle(input_config->channel, input_config->width, input_config->height, input_config->multi_mode, input_config->color_depth, input_config->refresh_rate);
    vTaskDelete(NULL);
}

/**
 * @name: FFreeRTOSLVGLConfigTask
 * @msg:  config the lvgl
 * @param  {void *} pvParameters is the parameters of demo
 * @return Null
 */
static void FFreeRTOSLVGLConfigTask(void *pvParameters)
{
    FASSERT(NULL != pvParameters);
    InputParm *input_config = (InputParm *)pvParameters ;
    lv_init();
    disp_config = FDcDpMultiDisplayFrameBufferSet(input_config->channel, input_config->width, input_config->height, input_config->color_depth, input_config->multi_mode);
    FFreeRTOSPortInit(disp_config);
    vTaskDelete(NULL);
}

/**
 * @name: FFreeRTOSMediaInitCreate
 * @msg: creat the media init task
 * @param  {void *} args is the parameters of init function
 * @return xReturn,pdPASS:success,others:creat failed
 */
BaseType_t FFreeRTOSMediaInitCreate(void *args)
{
    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为 pdPASS */
    /* enter critical region */
    taskENTER_CRITICAL();
    /* Media init task */
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSMediaInitTask,  /* 任务入口函数 */
                          (const char *)"FFreeRTOSMediaInitTask",  /* 任务名字 */
                          (uint16_t)1024,                         /* 任务栈大小 */
                          (void *)args,                   /* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 2,                       /* 任务的优先级 */
                          (TaskHandle_t *)&init_task); /* 任务控制 */
    /* Hpd task control */
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSMediaHpdTask, /* 任务入口函数 */
                          (const char *)"FFreeRTOSMediaHpdTask", /* 任务名字 */
                          (uint16_t)1024,                        /* 任务栈大小 */
                          (void *)args,                   /* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 1,                      /* 任务的优先级 */
                          (TaskHandle_t *)&hpd_task);
    /* exit critical region */
    taskEXIT_CRITICAL();
    return xReturn;
}


/**
 * @name: FFreeRTOSlVGLConfigCreate
 * @msg:  set the lvgl init task
 * @return xReturn,pdPASS:success,others:creat failed
 */
BaseType_t FFreeRTOSlVGLConfigCreate(void *args)
{
    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为 pdPASS */
    BaseType_t timer_started = pdPASS;
    taskENTER_CRITICAL();
    /* lvgl demo task */
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSLVGLConfigTask,  /* 任务入口函数 */
                          (const char *)"FFreeRTOSLVGLConfigTask",  /* 任务名字 */
                          (uint16_t)1024,                         /* 任务栈大小 */
                          (void *)args,                                 /* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 3,                         /* 任务的优先级 */
                          (TaskHandle_t *)&lvgl_init_task); /* 任务控制 */

    /* exit critical region */
    taskEXIT_CRITICAL();

    return xReturn;
}

/**
 * @name: FFreeRTOSlVGLDemoCreate
 * @msg:  creat the media demo init task
 * @return xReturn,pdPASS:success,others:creat failed
 */
BaseType_t FFreeRTOSlVGLDemoCreate(void)
{
    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为 pdPASS */
    BaseType_t timer_started = pdPASS;
    taskENTER_CRITICAL();
    /* lvgl demo task */
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSLVGLDemoTask,  /* 任务入口函数 */
                          (const char *)"FFreeRTOSLVGLDemoTask",  /* 任务名字 */
                          (uint16_t)1024,                         /* 任务栈大小 */
                          NULL,                                   /* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 4,                         /* 任务的优先级 */
                          (TaskHandle_t *)&demo_task); /* 任务控制 */
    xLvglHeartTimer = xTimerCreate("LVGL Heart Software Timer",        /* Text name for the software timer - not used by FreeRTOS. */
                                   LVGL_HEART_TIMER_PERIOD,           /* The software timer's period in ticks. */
                                   pdTRUE,                          /* Setting uxAutoRealod to pdFALSE creates a one-shot software timer. */
                                   NULL,                               /* This example use the timer id. */
                                   LvglHeartTimerCallback);        /* The callback function to be used by the software timer being created. */
    if (xLvglHeartTimer != NULL)
    {
        timer_started = xTimerStart(xLvglHeartTimer, 0);
        if (timer_started != pdPASS)
        {
            vPrintf("CreateSoftwareTimerTasks xTimerStart failed \r\n");
        }
    }
    else
    {
        vPrintf("CreateSoftwareTimerTasks xTimerCreate failed \r\n");
    }
    /* exit critical region */
    taskEXIT_CRITICAL();

    return xReturn;
}





