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
 * FilePath: pwm_example.c
 * Date: 2022-07-11 11:32:48
 * LastEditTime: 2022-07-11 11:32:48
 * Description:  This file is for pwm test example functions.
 *
 * Modify History:
 *  Ver   Who           Date           Changes
 * ----- ------       --------      --------------------------------------
 * 1.0  wangxiaodong  2022/8/24      first release
 */
#include <string.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "fpwm.h"
#include "fpwm_os.h"
#include "fcpu_info.h"
#include "fio_mux.h"
#include "pwm_example.h"

/* The periods assigned to the one-shot timers. */
#define ONE_SHOT_TIMER_PERIOD       ( pdMS_TO_TICKS( 300000UL ) )

/* pwm pulse change period */
#define PWM_CHANGE_PERIOD           ( pdMS_TO_TICKS( 2000UL ))
/* pwm pulse amplitude of periodic variation */
#define PWM_PULSE_CHANGE    1000

/* pwm channel use, 0/1 */
#define PWM_CHANNEL_USE     FPWM_CHANNEL_0

/* pwm primary config */
#define PWM_DIV             500
#define PWM_PERIOD          10000
#define PWM_PULSE           2000

/* test task number */
#define TEST_TASK_NUM 1
static xSemaphoreHandle xCountingSemaphore;

static xTaskHandle change_handle;
static TimerHandle_t xOneShotTimer;

static FFreeRTOSPwm *os_pwm_ctrl_p;

static void FFreeRTOSPwmDelete(FFreeRTOSPwm *os_pwm_p);

static void FFreeRTOSPwmInitTask(void *pvParameters)
{
    /* The pwm_id to use is passed in via the parameter.
    Cast this to a pwm_id pointer. */
    u32 pwm_id = (u32)(uintptr)pvParameters;

    FError ret = FPWM_SUCCESS;

    /* set channel 0 and 1 iopad*/
#if defined(CONFIG_TARGET_E2000) || defined(CONFIG_TARGET_PHYTIUMPI)
    FIOPadSetPwmMux(pwm_id, 0);
    FIOPadSetPwmMux(pwm_id, 1);
#endif

    /* init pwm controller */
    os_pwm_ctrl_p = FFreeRTOSPwmInit(pwm_id);
    if (os_pwm_ctrl_p == NULL)
    {
        printf("FFreeRTOSPwmInit failed.\n");
        goto pwm_init_exit;
    }

    /* set pwm db config */
    FPwmDbVariableConfig db_cfg;
    memset(&db_cfg, 0, sizeof(db_cfg));
    db_cfg.db_rise_cycle = 500;
    db_cfg.db_fall_cycle = 500;
    db_cfg.db_polarity_sel = FPWM_DB_AHC;
    db_cfg.db_in_mode = FPWM_DB_IN_MODE_PWM0;
    db_cfg.db_out_mode = FPWM_DB_OUT_MODE_ENABLE_RISE_FALL;
    ret = FFreeRTOSPwmDbSet(os_pwm_ctrl_p, &db_cfg);
    if (FPWM_SUCCESS != ret)
    {
        printf("FFreeRTOSPwmDbSet failed.\n");
        goto pwm_init_exit;
    }
    /* start pwm config */
    FPwmVariableConfig pwm_cfg;
    memset(&pwm_cfg, 0, sizeof(pwm_cfg));
    pwm_cfg.tim_ctrl_mode = FPWM_MODULO;
    pwm_cfg.tim_ctrl_div = PWM_DIV - 1;
    pwm_cfg.pwm_period = PWM_PERIOD;
    pwm_cfg.pwm_pulse = PWM_PULSE;
    pwm_cfg.pwm_mode = FPWM_OUTPUT_COMPARE;
    pwm_cfg.pwm_polarity = FPWM_POLARITY_NORMAL;
    pwm_cfg.pwm_duty_source_mode = FPWM_DUTY_CCR;
    ret = FFreeRTOSPwmSet(os_pwm_ctrl_p, PWM_CHANNEL_USE, &pwm_cfg);
    if (FPWM_SUCCESS != ret)
    {
        printf("FFreeRTOSPwmSet failed.\n");
        goto pwm_init_exit;
    }

    memset(&db_cfg, 0, sizeof(db_cfg));
    memset(&pwm_cfg, 0, sizeof(pwm_cfg));

    FFreeRTOSPwmDbGet(os_pwm_ctrl_p, &db_cfg);
    printf("FFreeRTOSPwmDbGet:\n");
    printf("db_cfg.db_rise_cycle = %d\n", db_cfg.db_rise_cycle);
    printf("db_cfg.db_fall_cycle = %d\n", db_cfg.db_fall_cycle);
    printf("db_cfg.db_polarity_sel = %d\n", db_cfg.db_polarity_sel);
    printf("db_cfg.db_in_mode = %d\n", db_cfg.db_in_mode);
    printf("db_cfg.db_out_mode = %d\n", db_cfg.db_out_mode);

    FFreeRTOSPwmGet(os_pwm_ctrl_p, PWM_CHANNEL_USE, &pwm_cfg);
    printf("FPwmVariableGet:\n");
    printf("pwm_cfg.tim_ctrl_mode = %d\n", pwm_cfg.tim_ctrl_mode);
    printf("pwm_cfg.tim_ctrl_div = %d\n", pwm_cfg.tim_ctrl_div);
    printf("pwm_cfg.pwm_period = %d\n", pwm_cfg.pwm_period);
    printf("pwm_cfg.pwm_pulse = %d\n", pwm_cfg.pwm_pulse);
    printf("pwm_cfg.pwm_mode = %d\n", pwm_cfg.pwm_mode);
    printf("pwm_cfg.pwm_polarity = %d\n", pwm_cfg.pwm_polarity);
    printf("pwm_cfg.pwm_duty_source_mode = %d\n", pwm_cfg.pwm_duty_source_mode);

    FFreeRTOSPwmEnable(os_pwm_ctrl_p, PWM_CHANNEL_USE, TRUE);

    printf("FFreeRTOSPwmInitTask execute successfully.\r\n");

    for (int i = 0; i < TEST_TASK_NUM; i++)
    {
        xSemaphoreGive(xCountingSemaphore);
    }

pwm_init_exit:
    vTaskDelete(NULL);

}

static void FFreeRTOSPwmChangeTask(void *pvParameters)
{
    xSemaphoreTake(xCountingSemaphore, portMAX_DELAY);
    u32 pwm_pulse = PWM_PULSE;
    /* As per most tasks, this task is implemented in an infinite loop. */
    for (;;)
    {
        FFreeRTOSPwmPulseSet(os_pwm_ctrl_p, PWM_CHANNEL_USE, pwm_pulse);

        printf("FFreeRTOSPwmChangeTask run, pwm_pulse: %d\r\n", pwm_pulse);
        pwm_pulse = (pwm_pulse + PWM_PULSE_CHANGE) % PWM_PERIOD;
        vTaskDelay(PWM_CHANGE_PERIOD);
    }
}

static void prvOneShotTimerCallback(TimerHandle_t xTimer)
{
    /* Output a string to show the time at which the callback was executed. */
    printf("One-shot timer callback executing, which will delete FFreeRTOSPwmChangeTask.\r\n");

    FFreeRTOSPwmDelete(os_pwm_ctrl_p);
}

/* create pwm test, id is pwm module number */
BaseType_t FFreeRTOSPwmCreate(u32 id)
{
    FASSERT(id < FPWM_NUM);
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */
    BaseType_t xTimerStarted = pdPASS;

    xCountingSemaphore = xSemaphoreCreateCounting(TEST_TASK_NUM, 0);
    if (xCountingSemaphore == NULL)
    {
        printf("FFreeRTOSPwmCreate xCountingSemaphore create failed.\r\n");
        return pdFAIL;
    }

    taskENTER_CRITICAL(); //进入临界区
    /* pwm init task */
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSPwmInitTask,  /* 任务入口函数 */
                          (const char *)"FFreeRTOSPwmInitTask",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          (void *)(uintptr)id,/* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 1, /* 任务的优先级 */
                          NULL); /* 任务控制 */

    /* pwm占空比变化任务 */
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSPwmChangeTask,  /* 任务入口函数 */
                          (const char *)"FFreeRTOSPwmChangeTask",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          NULL,/* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 1, /* 任务的优先级 */
                          (TaskHandle_t *)&change_handle); /* 任务控制 */

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
        xTimerStarted = xTimerStart(xOneShotTimer, 0);

        /* The implementation of xTimerStart() uses the timer command queue, and
        xTimerStart() will fail if the timer command queue gets full.  The timer
        service task does not get created until the scheduler is started, so all
        commands sent to the command queue will stay in the queue until after
        the scheduler has been started.  Check both calls to xTimerStart()
        passed. */
        if (xTimerStarted != pdPASS)
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

static void FFreeRTOSPwmDelete(FFreeRTOSPwm *os_pwm_p)
{
    BaseType_t xReturn = pdPASS;
    FFreeRTOSPwmEnable(os_pwm_p, PWM_CHANNEL_USE, FALSE);
    FFreeRTOSPwmDeinit(os_pwm_p);

    if (change_handle)
    {
        vTaskDelete(change_handle);
        vPrintf("Delete FFreeRTOSPwmChangeTask successfully.\r\n");
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
        vPrintf("Delete OneShot Software Timer successfully.\r\n");
    }

}