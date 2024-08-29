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
 * FilePath: qspi_spiffs_example.c
 * Date: 2022-07-11 11:32:48
 * LastEditTime: 2022-07-11 11:32:48
 * Description: This file is for the qspi_spiffs example functions.
 *
 * Modify History:
 *  Ver      Who            Date           Changes
 * -----   ------         --------    --------------------------------------
 * 1.0   wangxiaodong    2022/8/9       first release
 * 1.1   zhangyan        2023/2/9       improve functions
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "fkernel.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "fio_mux.h"
#include "timers.h"
#include "qspi_spiffs_example.h"
#include "strto.h"
#include "fassert.h"
#include "fdebug.h"
#include "fparameters.h"
#include "spiffs_port.h"
#include "sdkconfig.h"
#ifdef CONFIG_SPIFFS_ON_FSPIM_SFUD
    #include "fspim_spiffs_port.h"
#endif

#ifdef CONFIG_SPIFFS_ON_FQSPI_SFUD
    #include "fqspi_spiffs_port.h"
#endif


enum
{
    FSPIFFS_OPS_OK = 0,
    FSPIFFS_OPS_INIT_FAILED,
    FSPIFFS_OPS_ALREADY_INITED,
    FSPIFFS_OPS_MOUNT_FAILED,
    FSPIFFS_OPS_FORMAT_FAILED,
    FSPIFFS_OPS_NOT_YET_MOUNT,
    FSPIFFS_OPS_OPEN_FILE_FAILED,
    FSPIFFS_OPS_WRITE_FILE_FAILED,
    FSPIFFS_OPS_READ_FILE_FAILED,
    FSPIFFS_OPS_REMOVE_FILE_FAILED,
    FSPIFFS_OPS_CLOSE_FILE_FAILED,
};

#define FSPIFFS_DEBUG_TAG "SPIFFS-QSPI"
#define FSPIFFS_ERROR(format, ...)   FT_DEBUG_PRINT_E(FSPIFFS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIFFS_WARN(format, ...)    FT_DEBUG_PRINT_W(FSPIFFS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIFFS_INFO(format, ...)    FT_DEBUG_PRINT_I(FSPIFFS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIFFS_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FSPIFFS_DEBUG_TAG, format, ##__VA_ARGS__)

/* spiffs start address and size */
#if defined(CONFIG_TARGET_E2000)
#define FSPIFFS_START_ADDR      (3 * SZ_1M)
#elif defined(CONFIG_TARGET_FT2004) || defined(CONFIG_TARGET_D2000)
#define FSPIFFS_START_ADDR      (7 * SZ_1M)
#endif

#define FSPIFFS_USE_SIZE        SZ_1M

#define FSPIFFS_RW_BUF_SIZE     64

/* if format flash, TRUE is need format, it tasks some time  */
#define FSPIFFS_IF_FORMAT       TRUE

/* 一个页大小两倍的一个RAM缓冲区, 用来加载和维护SPIFFS的逻辑页 */
static volatile u8 fspiffs_work_buf[FSPIFFS_LOG_PAGE_SIZE * 2] = {0};
static volatile u8 fspiffs_fds_buf[32 * 4] = {0};
static volatile u8 fspiffs_cache_buf[(FSPIFFS_LOG_PAGE_SIZE + 32) * 4] = {0};
static u8 fspiffs_rd_buf[FSPIFFS_RW_BUF_SIZE] = {0};
static u8 fspiffs_wr_buf[FSPIFFS_RW_BUF_SIZE] = {0};
static FSpiffs instance;
static spiffs_config config;
static boolean spiffs_inited = FALSE;
const char *file_name = "test.txt";

/************************** Constant Definitions *****************************/

/* The periods assigned to the one-shot timers. */
#define ONE_SHOT_TIMER_PERIOD       ( pdMS_TO_TICKS( 50000UL ) )

/* write and read task delay in milliseconds */
#define TASK_DELAY_MS   3000UL

/* write and read task number */
#define READ_WRITE_TASK_NUM 1
static xSemaphoreHandle xCountingSemaphore;

static xTaskHandle qspi_rw_handle;

static TimerHandle_t xOneShotTimer;

static const char *xString = "FFreeRTOSQspiSpiffsWriteReadTask is running\r\n";

static void FFreeRTOSQspiSpiffsDelete(void);

static int FSpiffsOpsMount(boolean do_format)
{
    int result = 0;

    if (do_format)
    {
        result = SPIFFS_mount(&instance.fs,
                              &config,
                              (u8_t *)fspiffs_work_buf,
                              (u8_t *)fspiffs_fds_buf,
                              sizeof(fspiffs_fds_buf),
                              (u8_t *)fspiffs_cache_buf,
                              sizeof(fspiffs_cache_buf),
                              NULL);

        /* try mount to get status of filesystem  */
        if ((SPIFFS_OK != result) && (SPIFFS_ERR_NOT_A_FS != result))
        {
            /* if not a valid filesystem, continue to format,
                other error cannot handle, just exit */
            FSPIFFS_ERROR("Mount spiffs failed: %d", result);
            return FSPIFFS_OPS_MOUNT_FAILED;
        }

        /* must be unmounted prior to formatting */
        SPIFFS_unmount(&instance.fs);

        FSPIFFS_DEBUG("Spiffs formatting in progress ...");
        result = SPIFFS_format(&instance.fs);
        if (SPIFFS_OK != result)
        {
            FSPIFFS_ERROR("Spiffs formatting failed: %d", result);
            return FSPIFFS_OPS_FORMAT_FAILED;
        }
    }

    /* real mount */
    result = SPIFFS_mount(&instance.fs,
                          &config,
                          (u8_t *)fspiffs_work_buf,
                          (u8_t *)fspiffs_fds_buf,
                          sizeof(fspiffs_fds_buf),
                          (u8_t *)fspiffs_cache_buf,
                          sizeof(fspiffs_cache_buf),
                          NULL);
    if (SPIFFS_OK != result)
    {
        FSPIFFS_ERROR("Remount spiffs failed: %d, you may format the medium first.", result);
        return FSPIFFS_OPS_MOUNT_FAILED;
    }
    else
    {
        vPrintf("Mount spiffs success. \r\n");
        instance.fs_ready = TRUE;
    }

    return FSPIFFS_OPS_OK;
}

static int FSpiffsOpsListAll(void)
{
    int ret = FSPIFFS_OPS_OK;
    int result = SPIFFS_OK;

    if (FALSE == instance.fs_ready)
    {
        FSPIFFS_ERROR("Please mount file system first.");
        return FSPIFFS_OPS_NOT_YET_MOUNT;
    }

    static spiffs_DIR dir;
    static struct spiffs_dirent entry;

    memset(&dir, 0, sizeof(dir));
    memset(&entry, 0, sizeof(entry));

    struct spiffs_dirent *cur_entry = &entry;
    (void)SPIFFS_opendir(&instance.fs, "/", &dir);

    while (NULL != (cur_entry = SPIFFS_readdir(&dir, cur_entry)))
    {
        printf("-- %s file-id: [0x%04x] page-id: [%d] file-size: %d\r\n",
               cur_entry->name,
               cur_entry->pix,
               cur_entry->obj_id,
               cur_entry->size);
    }

    (void)SPIFFS_closedir(&dir);
    return ret;
}

int FSpiffsOpsCreateFile(const char *file_name)
{
    FASSERT((file_name) && (strlen(file_name) > 0));
    if (FALSE == instance.fs_ready)
    {
        FSPIFFS_ERROR("Please mount file system first.");
        return FSPIFFS_OPS_NOT_YET_MOUNT;
    }

    int ret = FSPIFFS_OPS_OK;

    /* create file */
    s32_t result = SPIFFS_creat(&instance.fs, file_name, 0);
    if (result < 0)
    {
        FSPIFFS_ERROR("Failed to create file %s", file_name);
        return FSPIFFS_OPS_OPEN_FILE_FAILED;
    }

    /* open file */
    spiffs_file fd = SPIFFS_open(&instance.fs, file_name, SPIFFS_RDONLY, 0);
    if (0 > fd)
    {
        FSPIFFS_ERROR("Failed to open file %s errno %d", file_name, SPIFFS_errno(&instance.fs));
        return FSPIFFS_OPS_OPEN_FILE_FAILED;
    }

    /* check file status */
    static spiffs_stat status;
    memset(&status, 0, sizeof(status));
    result = SPIFFS_fstat(&instance.fs, fd, &status);
    if (result < 0)
    {
        FSPIFFS_ERROR("Failed to get status of file %s, errno %d", file_name, SPIFFS_errno(&instance.fs));
        ret = FSPIFFS_OPS_OPEN_FILE_FAILED;
        goto err_exit;
    }

    if (0 != strcmp(status.name, file_name))
    {
        FSPIFFS_ERROR("Created file name %s != %s", status.name, file_name);
        ret = FSPIFFS_OPS_OPEN_FILE_FAILED;
        goto err_exit;
    }

    if (0 != status.size)
    {
        FSPIFFS_ERROR("Invalid file size %d", status.size);
        ret = FSPIFFS_OPS_OPEN_FILE_FAILED;
        goto err_exit;
    }

    vPrintf("File %s created successfully.\r\n", file_name);

err_exit:
    (void)SPIFFS_close(&instance.fs, fd);
    return ret;
}

int FSpiffsOpsWriteFile(const char *file_name, const char *str)
{
    FASSERT((file_name) && (strlen(file_name) > 0));
    FASSERT(str);
    int ret = FSPIFFS_OPS_OK;
    const u32 wr_len = strlen(str) + 1;

    spiffs_file fd = SPIFFS_open(&instance.fs, file_name, SPIFFS_RDWR | SPIFFS_TRUNC, 0);
    if (0 > fd)
    {
        FSPIFFS_ERROR("Failed to open file %s, errno %d", file_name, SPIFFS_errno(&instance.fs));
        return FSPIFFS_OPS_OPEN_FILE_FAILED;
    }

    int result = SPIFFS_write(&instance.fs, fd, (void *)str, wr_len);
    if (result < 0)
    {
        FSPIFFS_ERROR("Failed to write file %s, errno %d", file_name, SPIFFS_errno(&instance.fs));
        ret = FSPIFFS_OPS_WRITE_FILE_FAILED;
        goto err_exit;
    }

    /* check file status */
    static spiffs_stat status;
    memset(&status, 0, sizeof(status));
    result = SPIFFS_fstat(&instance.fs, fd, &status);
    if (result < 0)
    {
        FSPIFFS_ERROR("Failed to get status of file %s, errno %d", file_name, SPIFFS_errno(&instance.fs));
        ret = FSPIFFS_OPS_WRITE_FILE_FAILED;
        goto err_exit;
    }

    if (status.size != wr_len)
    {
        FSPIFFS_ERROR("File write size %ld != %ld", status.size, wr_len);
        ret = FSPIFFS_OPS_WRITE_FILE_FAILED;
        goto err_exit;
    }

    /* flush all pending writes from cache to flash */
    (void)SPIFFS_fflush(&instance.fs, fd);
    vPrintf("Write file %s with %d bytes successfully.\r\n", file_name, wr_len);
err_exit:
    (void)SPIFFS_close(&instance.fs, fd);
    return ret;
}

int FSpiffsOpsReadFile(const char *file_name)
{
    FASSERT((file_name) && (strlen(file_name) > 0));
    int ret = FSPIFFS_OPS_OK;
    int result = SPIFFS_OK;

    if (FALSE == instance.fs_ready)
    {
        FSPIFFS_ERROR("Please mount file system first.");
        return FSPIFFS_OPS_NOT_YET_MOUNT;
    }

    /* check file status */
    static spiffs_stat status;

    spiffs_flags open_flags = 0;

    /* open the file in read-only mode */
    open_flags = SPIFFS_RDWR;
    spiffs_file fd = SPIFFS_open(&instance.fs, file_name, open_flags, 0);
    if (0 > fd)
    {
        FSPIFFS_ERROR("Failed to open file %s, errno %d", file_name, SPIFFS_errno(&instance.fs));
        return FSPIFFS_OPS_OPEN_FILE_FAILED;
    }

    /* check file status */
    memset(&status, 0, sizeof(status));
    result = SPIFFS_fstat(&instance.fs, fd, &status);
    if (result < 0)
    {
        FSPIFFS_ERROR("Failed to get status of file %s, errno %d", file_name, SPIFFS_errno(&instance.fs));
        ret = FSPIFFS_OPS_OPEN_FILE_FAILED;
        goto err_exit;
    }

    s32_t offset = SPIFFS_lseek(&instance.fs, fd, 0, SPIFFS_SEEK_END);
    if ((s32_t)status.size != offset)
    {
        FSPIFFS_ERROR("File %s spiffs:%ld != fs:%ld", file_name, status.size, offset);
        ret = FSPIFFS_OPS_OPEN_FILE_FAILED;
        goto err_exit;
    }

    memset(fspiffs_rd_buf, 0, FSPIFFS_RW_BUF_SIZE);

    /* seek to offset and start read */
    if (0 > SPIFFS_lseek(&instance.fs, fd, 0, SPIFFS_SEEK_SET))
    {
        FSPIFFS_ERROR("Seek file failed.");
        ret = FSPIFFS_OPS_READ_FILE_FAILED;
        goto err_exit;
    }


    s32_t read_len = min((s32_t)FSPIFFS_RW_BUF_SIZE, (s32_t)status.size);
    s32_t read_bytes = SPIFFS_read(&instance.fs, fd, (void *)fspiffs_rd_buf, read_len);
    if (read_bytes < 0)
    {
        FSPIFFS_ERROR("Failed to read file %s, errno %d", file_name, SPIFFS_errno(&instance.fs));
        ret = FSPIFFS_OPS_READ_FILE_FAILED;
        goto err_exit;
    }

    vPrintf("Read %s success, str = %s\n", file_name, fspiffs_rd_buf);

err_exit :
    /* close file */
    (void)SPIFFS_close(&instance.fs, fd);

    return ret;
}


static void FFreeRTOSQspiSpiffsInitTask(void *pvParameters)
{
    int result = 0;

    if (TRUE == spiffs_inited)
    {
        FSPIFFS_WARN("Spiffs is already initialized.");
        return;
    }

    /* The qspi_id to use is passed in via the parameter.
    Cast this to a qspi_id pointer. */
    u32 qspi_id = (u32)(uintptr)pvParameters;
    printf("qspi_id: %d\n", qspi_id);

#if defined(CONFIG_TARGET_E2000)
    FIOPadSetQspiMux(qspi_id, FQSPI_CS_0);
    FIOPadSetQspiMux(qspi_id, FQSPI_CS_1);
#endif

    memset(&config, 0, sizeof(config));
    config = *FSpiffsGetDefaultConfig();
    config.phys_addr = FSPIFFS_START_ADDR; /* may use part of flash */
    config.phys_size = FSPIFFS_USE_SIZE;

    memset(&instance, 0, sizeof(instance));
    instance.fs_addr = FSPIFFS_START_ADDR;
    instance.fs_size = FSPIFFS_USE_SIZE;

    result = FSpiffsInitialize(&instance, FSPIFFS_PORT_TO_FQSPI);
    if (FSPIFFS_PORT_OK != result)
    {
        FSPIFFS_ERROR("Initialize spiffs failed.");
        return;
    }

    FSpiffsOpsMount(FSPIFFS_IF_FORMAT);

    FSpiffsOpsCreateFile(file_name);

    spiffs_inited = TRUE;

    FSpiffsOpsListAll();

    FSPIFFS_INFO("Spiffs init successfully.");

    for (int i = 0; i < READ_WRITE_TASK_NUM; i++)
    {
        xSemaphoreGive(xCountingSemaphore);
    }

    vTaskDelete(NULL);

}

static void FFreeRTOSQspiSpiffsWriteReadTask(void *pvParameters)
{
    const char *pcTaskName = (char *) pvParameters;
    const TickType_t xDelay = pdMS_TO_TICKS(TASK_DELAY_MS);
    FError ret = FT_SUCCESS;
    char *string = "spiffs qspi write times";
    static int i = 0;
    memset(fspiffs_wr_buf, 0, FSPIFFS_RW_BUF_SIZE);
    
    xSemaphoreTake(xCountingSemaphore, portMAX_DELAY);
    /* As per most tasks, this task is implemented in an infinite loop. */
    for (;;)
    {
        /* Print out the name of this task. */
        vPrintf(pcTaskName);
        i++;
        sprintf(fspiffs_wr_buf, "%s-%d", string, i);
        vPrintf("Write to %s, str = %s\n", file_name, fspiffs_wr_buf);
        
        FSpiffsOpsWriteFile(file_name, fspiffs_wr_buf);
        FSpiffsOpsReadFile(file_name);

        if (0 != strcmp(fspiffs_wr_buf, fspiffs_rd_buf))
        {
            FSPIFFS_ERROR("Read and write data are not equal!!!!\nwrite data:%s ,read data:%s\n", fspiffs_wr_buf, fspiffs_rd_buf);
            vTaskDelete(NULL);
        }
        else
        {
            printf("Successfully, read and write data are equal.\n\n");
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
    vPrintf("One-shot timer callback executing, delete QspiSpiffs ReadTask and WriteTask.\r\n");

    FFreeRTOSQspiSpiffsDelete();
}


BaseType_t FFreeRTOSQspiSpiffsCreate(u32 qspi_id)
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */
    BaseType_t xTimerStarted = pdPASS;

    xCountingSemaphore = xSemaphoreCreateCounting(READ_WRITE_TASK_NUM, 0);
    if (xCountingSemaphore == NULL)
    {
        printf("FFreeRTOSQspiSpiffsCreate xCountingSemaphore create failed.\r\n");
        return pdFAIL;
    }

    taskENTER_CRITICAL(); /*进入临界区*/

    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSQspiSpiffsInitTask,  /* 任务入口函数 */
                          (const char *)"FFreeRTOSQspiSpiffsInitTask",/* 任务名字 */
                          (uint16_t)4096,  /* 任务栈大小 */
                          (void *)(uintptr)qspi_id,/* 任务入口函数参数 */
                          (UBaseType_t)1,  /* 任务的优先级 */
                          NULL); /* 任务控制 */

    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSQspiSpiffsWriteReadTask,  /* 任务入口函数 */
                          (const char *)"FFreeRTOSQspiSpiffsWriteReadTask",/* 任务名字 */
                          (uint16_t)4096,  /* 任务栈大小 */
                          (void *)xString,/* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 1, /* 任务的优先级 */
                          (TaskHandle_t *)&qspi_rw_handle); /* 任务控制 */

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
            vPrintf("CreateSoftwareTimerTasks xTimerStart failed. \r\n");
        }
    }
    else
    {
        vPrintf("CreateSoftwareTimerTasks xTimerCreate failed. \r\n");
    }

    taskEXIT_CRITICAL();

    return xReturn;
}

static void FFreeRTOSQspiSpiffsDelete(void)
{
    BaseType_t xReturn = pdPASS;

    FSpiffsDeInitialize(&instance);

    if (qspi_rw_handle)
    {
        vTaskDelete(qspi_rw_handle);
        vPrintf("Delete FFreeRTOSQspiSpiffsWriteReadTask successfully.\r\n");
    }

    /* delete count sem */
    vSemaphoreDelete(xCountingSemaphore);

    /* delete timer */
    xReturn = xTimerDelete(xOneShotTimer, 0);
    if (xReturn != pdPASS)
    {
        vPrintf("OneShot Software Timer Delete failed.\r\n");
    }
    else
    {
        vPrintf("OneShot Software Timer Delete successfully.\r\n");
    }

}



