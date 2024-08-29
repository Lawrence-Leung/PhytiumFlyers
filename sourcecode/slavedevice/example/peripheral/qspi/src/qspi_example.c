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
 * FilePath: qspi_example.c
 * Date: 2022-07-11 11:32:48
 * LastEditTime: 2022-07-11 11:32:48
 * Description:  This file is for qspi test example functions.
 *
 * Modify History:
 *  Ver   Who           Date           Changes
 * ----- ------       --------      --------------------------------------
 * 1.0  wangxiaodong  2022/8/9      first release
 */
#include <string.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "fqspi.h"
#include "fqspi_flash.h"
#include "fqspi_os.h"
#include "timers.h"
#include "qspi_example.h"
#include "sdkconfig.h"
#include "fio_mux.h"

/* The periods assigned to the one-shot timers. */
#define ONE_SHOT_TIMER_PERIOD       ( pdMS_TO_TICKS( 50000UL ) )

/* write and read task delay in milliseconds */
#define TASK_DELAY_MS   10000UL


static xTaskHandle read_handle;
static xTaskHandle write_handle;
static TimerHandle_t xOneShotTimer;

/* Offset 1M from flash maximum capacity*/
#define FLASH_WR_OFFSET SZ_1M 
/* write and read start address */
static u32 flash_wr_start = 0 ; 

/* write and read cs channel */
#define QSPI_CS_CHANNEL 0

#define DAT_LENGTH  64
static u8 rd_buf[DAT_LENGTH] = {0};
static u8 wr_buf[DAT_LENGTH] = {0};

/* test task number */
#define READ_WRITE_TASK_NUM 2
static xSemaphoreHandle xCountingSemaphore;

static FFreeRTOSQspi *os_qspi_ctrl_p = NULL;

static FFreeRTOSQspiMessage message = {0};

static void FFreeRTOSQspiDelete(void);

static void QspiInitTask(void *pvParameters)
{
    /* The qspi_id to use is passed in via the parameter.
    Cast this to a qspi_id pointer. */
    u32 qspi_id = (u32)(uintptr)pvParameters;

#if defined(CONFIG_TARGET_E2000)
    FIOPadSetQspiMux(qspi_id, FQSPI_CS_0);
    FIOPadSetQspiMux(qspi_id, FQSPI_CS_1);
#endif

    /* init qspi controller */
    os_qspi_ctrl_p = FFreeRTOSQspiInit(qspi_id);
    flash_wr_start = os_qspi_ctrl_p->qspi_ctrl.flash_size - FLASH_WR_OFFSET;
    if (os_qspi_ctrl_p == NULL)
    {
        printf("FFreeRTOSWdtInit failed.\n");
        goto qspi_init_exit;
    }

    for (int i = 0; i < READ_WRITE_TASK_NUM; i++)
    {
        xSemaphoreGive(xCountingSemaphore);
    }

qspi_init_exit:
    vTaskDelete(NULL);
}

static void QspiReadTask(void *pvParameters)
{
    const char *pcTaskName = "QspiReadTask is running\r\n";
    const TickType_t xDelay = pdMS_TO_TICKS(TASK_DELAY_MS);
    FError ret = FQSPI_SUCCESS;

    xSemaphoreTake(xCountingSemaphore, portMAX_DELAY);

    /* As per most tasks, this task is implemented in an infinite loop. */
    for (;;)
    {
        /* Print out the name of this task. */
        printf(pcTaskName);

        message.read_buf = rd_buf;
        message.length = DAT_LENGTH;
        message.addr = flash_wr_start;
        message.cmd = FQSPI_FLASH_CMD_READ;
        message.cs = QSPI_CS_CHANNEL;
        ret = FFreeRTOSQspiTransfer(os_qspi_ctrl_p, &message);
        if (FQSPI_SUCCESS != ret)
        {
            printf("QspiReadTask FFreeRTOSQspiTransfer failed, return value: 0x%x\r\n", ret);
        }
        taskENTER_CRITICAL(); //进入临界区
        FtDumpHexByte(rd_buf, DAT_LENGTH);
        taskEXIT_CRITICAL(); //退出临界区

        /* Delay for a period.  This time a call to vTaskDelay() is used which
        places the task into the Blocked state until the delay period has
        expired.  The parameter takes a time specified in 'ticks', and the
        pdMS_TO_TICKS() macro is used (where the xDelay constant is
        declared) to convert TASK_DELAY_MS milliseconds into an equivalent time in
        ticks. */
        vTaskDelay(xDelay);
    }
}

static void QspiWriteTask(void *pvParameters)
{
    const char *pcTaskName = "QspiWriteTask is running\r\n";
    const TickType_t xDelay = pdMS_TO_TICKS(TASK_DELAY_MS);
    int i = 0;
    FError ret = FQSPI_SUCCESS;

    xSemaphoreTake(xCountingSemaphore, portMAX_DELAY);

    /* As per most tasks, this task is implemented in an infinite loop. */
    for (;;)
    {
        /* Print out the name of this task. */
        printf(pcTaskName);
        for (i = 0; i < DAT_LENGTH; i++)
        {
            wr_buf[i] = wr_buf[i] + 0x11;
        }

        message.addr = flash_wr_start;
        message.cmd = FQSPI_FLASH_CMD_SE;
        message.cs = QSPI_CS_CHANNEL;
        ret = FFreeRTOSQspiTransfer(os_qspi_ctrl_p, &message);
        if (FQSPI_SUCCESS != ret)
        {
            printf("QspiWriteTask FFreeRTOSQspiTransfer failed, return value: 0x%x\r\n", ret);
        }

        message.write_buf = wr_buf;
        message.length = DAT_LENGTH;
        message.addr = flash_wr_start;
        message.cmd = FQSPI_FLASH_CMD_PP;
        message.cs = QSPI_CS_CHANNEL;

        ret = FFreeRTOSQspiTransfer(os_qspi_ctrl_p, &message);
        if (FQSPI_SUCCESS != ret)
        {
            printf("QspiWriteTask FFreeRTOSQspiTransfer failed, return value: 0x%x\r\n", ret);
        }

        /* Delay for a period.  This time a call to vTaskDelay() is used which
        places the task into the Blocked state until the delay period has
        expired.  The parameter takes a time specified in 'ticks', and the
        pdMS_TO_TICKS() macro is used (where the xDelay constant is
        declared) to convert TASK_DELAY_MS milliseconds into an equivalent time in
        ticks. */
        vTaskDelay(xDelay);
    }
}

static void prvOneShotTimerCallback(TimerHandle_t xTimer)
{
    /* Output a string to show the time at which the callback was executed. */
    printf("One-shot timer callback executing, which will delete QspiReadTask and QspiWriteTask.\r\n");

    FFreeRTOSQspiDelete();
}

BaseType_t FFreeRTOSQspiCreate(u32 id)
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */
    BaseType_t xTimerStarted = pdPASS;

    memset(&message, 0, sizeof(message));

    xCountingSemaphore = xSemaphoreCreateCounting(READ_WRITE_TASK_NUM, 0);
    if (xCountingSemaphore == NULL)
    {
        printf("FFreeRTOSWdtCreate xCountingSemaphore create failed.\r\n");
        return pdFAIL;
    }

    taskENTER_CRITICAL(); /*进入临界区*/

    xReturn = xTaskCreate((TaskFunction_t)QspiInitTask,  /* 任务入口函数 */
                          (const char *)"QspiInitTask",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          (void *)(uintptr)id,/* 任务入口函数参数 */
                          (UBaseType_t)2,  /* 任务的优先级 */
                          NULL);

    xReturn = xTaskCreate((TaskFunction_t)QspiReadTask,  /* 任务入口函数 */
                          (const char *)"QspiReadTask",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          NULL,/* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 1, /* 任务的优先级 */
                          (TaskHandle_t *)&read_handle); /* 任务控制 */

    xReturn = xTaskCreate((TaskFunction_t)QspiWriteTask,  /* 任务入口函数 */
                          (const char *)"QspiWriteTask",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          NULL,/* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 1, /* 任务的优先级 */
                          (TaskHandle_t *)&write_handle); /* 任务控制 */

    /* Create the one shot software timer, storing the handle to the created
    software timer in xOneShotTimer. */
    xOneShotTimer = xTimerCreate("OneShot Software Timer",       /* Text name for the software timer - not used by FreeRTOS. */
                                 ONE_SHOT_TIMER_PERIOD,        /* The software timer's period in ticks. */
                                 pdFALSE,                      /* Setting uxAutoRealod to pdFALSE creates a one-shot software timer. */
                                 0,                            /* This example does not use the timer id. */
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
            printf("CreateSoftwareTimerTasks xTimerStart failed.\r\n");
        }
    }
    else
    {
        printf("CreateSoftwareTimerTasks xTimerCreate failed.\r\n");
    }

    taskEXIT_CRITICAL();

    return xReturn;
}

static void FFreeRTOSQspiDelete(void)
{
    BaseType_t xReturn = pdPASS;
    FFreeRTOSQspiDeinit(os_qspi_ctrl_p);
    if (read_handle)
    {
        vTaskDelete(read_handle);
        printf("Delete QspiReadTask successfully.\r\n");
    }

    if (write_handle)
    {
        vTaskDelete(write_handle);
        printf("Delete QspiWriteTask successfully.\r\n");
    }

    /* delete count sem */
    vSemaphoreDelete(xCountingSemaphore);

    /* delete timer */
    xReturn = xTimerDelete(xOneShotTimer, 0);
    if (xReturn != pdPASS)
    {
        printf("OneShot Software Timer Delete failed.\r\n");
    }
    else
    {
        printf("OneShot Software Timer Delete successfully.\r\n");
    }
}



