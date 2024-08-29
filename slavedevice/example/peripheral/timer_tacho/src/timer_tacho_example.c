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
 * FilePath: timer_tacho_example.c
 * Date: 2022-08-24 13:57:55
 * LastEditTime: 2022-08-24 13:57:56
 * Description:  This file is for timer tacho example functions.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 liushengming 2022/11/25   init
 */

#include <string.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "fassert.h"
#include "timers.h"
#include "ftimer_tacho_os.h"
#include "timer_tacho_example.h"
#include "fparameters.h"
#include "fio_mux.h"
#include "fcpu_info.h"
#include "sdkconfig.h"
#include "fdebug.h"
#include "fio_mux.h"

/* The periods assigned to the one-shot timers. */
#define ONE_SHOT_TIMER_PERIOD       ( pdMS_TO_TICKS( 50000UL ) )

#define TIMER_IRQ_PRIORITY 0xb
#define TACHO_IRQ_PRIORITY 0xc
#define TIMER_INSTANCE_NUM 0U

#ifdef CONFIG_TARGET_PHYTIUMPI
#define TACHO_INSTANCE_NUM 3U
#else
#define TACHO_INSTANCE_NUM 12U
#endif

/* write and read task delay in milliseconds */
#define TASK_DELAY_MS   2000UL

static xTaskHandle timer_handle;
static xTaskHandle tacho_handle;
static xTaskHandle cap_handle;
static xTaskHandle init_handle;

static FFreeRTOSTimerTacho *os_timer_ctrl;
static FFreeRTOSTimerTacho *os_tacho_ctrl;

volatile int timerflag = 0;
volatile int tachoflag = 0;

/***** timer intr and handler******/
/**
 * @name: CycCmpIntrHandler
 * @msg: 循环定时回调函数
 * @return {*}
 * @param {void} *param
 */
static void CycCmpIntrHandler(void *param)
{
    FTimerTachoCtrl *instance_p = (FTimerTachoCtrl *)param;
    timerflag++;
    printf("Cyc intr,id: %d,times_in: %d.\r\n", instance_p->config.id, timerflag);
}

/**
 * @name: OnceCmpIntrHandler
 * @msg: 单次定时回调服务函数
 * @return {*}
 * @param {void} *param
 */
static void OnceCmpIntrHandler(void *param)
{
    FTimerTachoCtrl *instance_p = (FTimerTachoCtrl *)param;
    printf("Once cmp intr, timer id: %d.\r\n", instance_p->config.id);
    FTimerSetInterruptMask(instance_p, FTIMER_EVENT_ONCE_CMP, FALSE);
}

/**
 * @name: RolloverIntrHandler
 * @msg: 此中断已经在驱动层进行了屏蔽，由于我们设置的cmp值已经是翻转值，所以等同于中断计数中断，此处可作为用法的拓展
 * @return {*}
 * @param {void} *param
 */
static void RolloverIntrHandler(void *param)
{
    FTimerTachoCtrl *instance_p = (FTimerTachoCtrl *)param;
    /* Anything else that you can do.*/
    printf("Roll over cmp intr, timer id: %d", instance_p->config.id);
}

/**
 * @name: TimerDisableIntr
 * @msg: 失能中断
 * @return {void}
 * @param {FTimerTachoCtrl} *instance_p 驱动控制数据结构
 */
void TimerDisableIntr(FTimerTachoCtrl *instance_p)
{
    u32 irq_num = FTIMER_TACHO_IRQ_NUM(instance_p->config.id);

    InterruptMask(irq_num);
}

/**
 * @name: TimerEnableIntr
 * @msg: 设置并且使能中断
 * @return {void}
 * @param  {FTimerTachoCtrl} *instance_p 驱动控制数据结构
 */
static void TimerEnableIntr(FTimerTachoCtrl *instance_p)
{
    FASSERT(instance_p);

    u32 irq_num = FTIMER_TACHO_IRQ_NUM(instance_p->config.id);

    u32 cpu_id;
    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(irq_num, cpu_id);
    /* disable timer irq */
    InterruptMask(irq_num);

    /* umask timer irq */
    InterruptSetPriority(irq_num, TIMER_IRQ_PRIORITY);
    InterruptInstall(irq_num, FTimerTachoIntrHandler, instance_p, instance_p->config.name);

    FTimerTachoSetIntr(instance_p);
    /* enable irq */
    InterruptUmask(irq_num);

    return ;
}

/***** tacho intr and handler******/

static void TachoDisableIntr(FTimerTachoCtrl *instance_p)
{
    FASSERT(instance_p);
    u32 irq_num = FTIMER_TACHO_IRQ_NUM(instance_p->config.id);
    InterruptMask(irq_num);
}

static void TachOverIntrHandler(void *param)
{
    FTimerTachoCtrl *instance_p = (FTimerTachoCtrl *)param;
    u32 irq_num = FTIMER_TACHO_IRQ_NUM(instance_p->config.id);
    InterruptMask(irq_num);
    u32 rpm;
    FTachoGetFanRPM(instance_p, &rpm);
    printf("TachOver intr,tacho id: %d,rpm:%d.\r\n", instance_p->config.id, rpm);
    InterruptUmask(irq_num);
    tachoflag++;
    if (tachoflag > 20)
    {
        tachoflag = 0;
        TachoDisableIntr(instance_p);
        printf("Please deinit tacho,then init.");
    }
}

static void CapIntrHandler(void *param)
{
    FTimerTachoCtrl *instance_p = (FTimerTachoCtrl *)param;
    printf("TachCapt intr,tacho id: %d", instance_p->config.id);
}

static void TachUnderIntrHandler(void *param)
{
    FTimerTachoCtrl *instance_p = (FTimerTachoCtrl *)param;
    u32 irq_num = FTIMER_TACHO_IRQ_NUM(instance_p->config.id);
    InterruptMask(irq_num);
    u32 rpm;
    FTachoGetFanRPM(instance_p, &rpm);
    printf("TachUnder intr,tacho id: %d,rpm:%d.\r\n", instance_p->config.id, rpm);
    InterruptUmask(irq_num);
    tachoflag++;
    if (tachoflag > 20)
    {
        tachoflag = 0;
        TachoDisableIntr(instance_p);
        printf("Please deinit tacho,then init again.");
    }
}

void TachoEnableIntr(FTimerTachoCtrl *instance_p)
{
    FASSERT(instance_p);
    u32 irq_num = FTIMER_TACHO_IRQ_NUM(instance_p->config.id);

    u32 cpu_id;
    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(irq_num, cpu_id);

    /* disable timer irq */
    InterruptMask(irq_num);

    /* umask timer irq */
    InterruptSetPriority(irq_num, TACHO_IRQ_PRIORITY);
    InterruptInstall(irq_num, FTimerTachoIntrHandler, instance_p, instance_p->config.name);

    FTimerTachoSetIntr(instance_p);
    /* enable irq */
    InterruptUmask(irq_num);

    return;
}

static void TimerTask(void *pvParameters)
{
    TickType_t xDelay = pdMS_TO_TICKS(TASK_DELAY_MS);
    FError ret = FREERTOS_TIMER_TACHO_SUCCESS;
    FFreeRTOSTimerTacho *timer_p = (FFreeRTOSTimerTacho *)pvParameters;
    vTaskDelay(xDelay);
    printf("\r\n*****TimerTask is running...\r\n");

    TimerEnableIntr(&os_timer_ctrl->ctrl);
    ret = FFreeRTOSTimerStart(timer_p);
    if (ret != FREERTOS_TIMER_TACHO_SUCCESS)
    {
        printf("TimerTask Start failed.\r\n");
        return;
    }

    xDelay = pdMS_TO_TICKS(10000);/*delay 10s*/
    vTaskDelay(xDelay);

    ret = FFreeRTOSTimerStop(timer_p);
    if (ret != FREERTOS_TIMER_TACHO_SUCCESS)
    {
        printf("TimerTask Stop failed.\r\n");
        return;
    }

    /* disable timer irq */
    TimerDisableIntr(&os_timer_ctrl->ctrl);
    FFreeRTOSTimerDeinit(timer_p);
    printf("***TimerTask is over.\r\n");
    vTaskDelete(NULL);
}

static void TachoTask(void *pvParameters)
{
    const TickType_t xDelay = pdMS_TO_TICKS(TASK_DELAY_MS);
    FFreeRTOSTimerTacho *tacho_p = (FFreeRTOSTimerTacho *)pvParameters;
    FError ret = FREERTOS_TIMER_TACHO_SUCCESS;
    vTaskDelay(xDelay);
    printf("\r\n*****TachoTask is running...\r\n");

    TachoEnableIntr(&tacho_p->ctrl);
    ret = FFreeRTOSTimerStart(tacho_p);
    u32 rpm;
    vTaskDelay(100);/*等待采样周期完成*/
    if (ret != FREERTOS_TIMER_TACHO_SUCCESS)
    {
        printf("Tacho start failed.\r\n");
        return;
    }
    for (size_t i = 0; i < 5; i++)
    {
        ret = FFreeRTOSTachoGetRPM(tacho_p, &rpm);
        if (ret != FREERTOS_TIMER_TACHO_SUCCESS)
        {
            printf("TachoTask Stop failed.\r\n");
            return;
        }
        printf("***GET_RPM:%d.\r\n", rpm);
        vTaskDelay(xDelay);/*Collect every 2 seconds*/
    }
    TachoDisableIntr(&tacho_p->ctrl);
    FFreeRTOSTimerStop(tacho_p);
    FFreeRTOSTachoDeinit(tacho_p);

tacho_task_exit:
    printf("***TachoTask over.\r\n");
    vTaskDelete(NULL);
}

static void captask(void *pvParameters)
{
    const TickType_t xDelay = pdMS_TO_TICKS(TASK_DELAY_MS);
    FFreeRTOSTimerTacho *cap_p = (FFreeRTOSTimerTacho *)pvParameters;
    FError ret = FREERTOS_TIMER_TACHO_SUCCESS;
    vTaskDelay(pdMS_TO_TICKS(1));

    printf("\r\n*****TimerCapTask is running...\r\n");

    TachoEnableIntr(&cap_p->ctrl);
    ret = FFreeRTOSTimerStart(cap_p);
    if (ret != FREERTOS_TIMER_TACHO_SUCCESS)
    {
        printf("Tacho start failed.\r\n");
        goto tacho_task_exit;
    }
    for (size_t i = 0; i < 5; i++)
    {
        printf("Get id %d CaptureCnt is :%d.\r\n", cap_p->ctrl.config.id, FFreeRTOSTachoGetCNT(cap_p));
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    /* disable tacho irq */
    TachoDisableIntr(&cap_p->ctrl);
    FFreeRTOSTimerStop(cap_p);
    FFreeRTOSTachoDeinit(cap_p);

tacho_task_exit:
    printf("TimerCapTask is over.\r\n");
    vTaskDelete(NULL);
}

static void InitTask(void *pvParameters)
{
    BaseType_t xReturn = pdPASS;
    /* init timers controller */

    os_timer_ctrl = FFreeRTOSTimerInit(TIMER_INSTANCE_NUM, FTIMER_CYC_CMP, 2000000);/* 2000000 us = 2 s */
    if (os_timer_ctrl == NULL)
    {
        printf("*Timer init error.\r\n");
        goto timer_init_exit;
    }
    FTimerRegisterEvtCallback(&os_timer_ctrl->ctrl, FTIMER_EVENT_CYC_CMP, CycCmpIntrHandler);
    FTimerRegisterEvtCallback(&os_timer_ctrl->ctrl, FTIMER_EVENT_ONCE_CMP, OnceCmpIntrHandler);
    FTimerRegisterEvtCallback(&os_timer_ctrl->ctrl, FTIMER_EVENT_ROLL_OVER, RolloverIntrHandler);

    /*init mode: FTIMER_WORK_MODE_CAPTURE or FTIMER_WORK_MODE_TACHO */
    os_tacho_ctrl = FFreeRTOSTachoInit(TACHO_INSTANCE_NUM, FTIMER_WORK_MODE_TACHO);
    if (os_timer_ctrl == NULL)
    {
        printf("*Tacho init error.\r\n");
        goto timer_init_exit;
    }
    /* set iopad mux */
    FIOPadSetTachoMux(TACHO_INSTANCE_NUM);

    FTimerRegisterEvtCallback(&os_tacho_ctrl->ctrl, FTACHO_EVENT_OVER, TachOverIntrHandler);
    FTimerRegisterEvtCallback(&os_tacho_ctrl->ctrl, FTACHO_EVENT_UNDER, TachUnderIntrHandler);
    FTimerRegisterEvtCallback(&os_tacho_ctrl->ctrl, FTACHO_EVENT_CAPTURE, CapIntrHandler);

    taskENTER_CRITICAL(); //进入临界区
    xReturn = xTaskCreate((TaskFunction_t)TimerTask,             /* 任务入口函数 */
                          (const char *)"TimerTask",             /* 任务名字 */
                          (uint16_t)1024,                        /* 任务栈大小 */
                          (void *)os_timer_ctrl,                 /* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 1, /* 任务的优先级 */
                          (TaskHandle_t *)&timer_handle);        /* 任务控制 */
    FASSERT_MSG(xReturn == pdPASS, "TimerTask create is failed.");

    xReturn = xTaskCreate((TaskFunction_t)TachoTask, /* 任务入口函数 */
                          (const char *)"TachoTask",/* 任务名字 */
                          (uint16_t)1024, /* 任务栈大小 */
                          (void *)os_tacho_ctrl,/* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 2, /* 任务的优先级 */
                          (TaskHandle_t *)&tacho_handle); /* 任务控制 */
    FASSERT_MSG(xReturn == pdPASS, "TachoTask create is failed.");

    taskEXIT_CRITICAL(); //退出临界区
timer_init_exit:
    vTaskDelete(NULL);
}

BaseType_t FFreeRTOSTimerTachoCreate(void)
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */

    taskENTER_CRITICAL(); //进入临界区

    /* init timers controller */

    xReturn = xTaskCreate((TaskFunction_t)InitTask,  /* 任务入口函数 */
                          (const char *)"InitTask",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          (void *)NULL,/* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 1, /* 任务的优先级 */
                          (TaskHandle_t *)&init_handle); /* 任务控制 */
    FASSERT_MSG(xReturn == pdPASS, "TachoTask create is failed.");

    taskEXIT_CRITICAL(); //退出临界区

    return xReturn;
}

