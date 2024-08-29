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
 * FilePath: sd_read_write.c
 * Date: 2022-07-25 15:58:24
 * LastEditTime: 2022-07-25 15:58:25
 * Description:   This file is for providing functions used in cmd_sd.c file.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  zhugengyu  2022/8/26    first commit
 */
/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#include "fassert.h"
#include "fdebug.h"
#include "fsleep.h"
#include "fkernel.h"
#include "fcache.h"
#include "fio.h"

#include "sdmmc_host_os.h"
/************************** Constant Definitions *****************************/
#define SD_WR_BUF_LEN       4096
#define SD_EVT_INIT_DONE    (0x1 << 0)
#define SD_EVT_WRITE_DONE   (0x1 << 1)
#define SD_EVT_READ_DONE    (0x1 << 2)
/**************************** Type Definitions *******************************/
typedef struct
{
    sdmmc_host_instance_t *cur_host;
    sdmmc_host_config_t *cur_host_config;
    fsize_t start_blk;
    fsize_t block_num;
} SdioTestInfo;
/************************** Variable Definitions *****************************/
static u8 sd_write_buffer[SD_WR_BUF_LEN] = {0};
static u8 sd_read_buffer[SD_WR_BUF_LEN] = {0};

static sdmmc_host_instance_t tf_host;
static sdmmc_host_config_t tf_host_config;
static sdmmc_host_instance_t emmc_host;
static sdmmc_host_config_t emmc_host_config;
static SdioTestInfo test_info =
{
    .cur_host = &tf_host,
    .cur_host_config = &tf_host_config,
    .start_blk = 0U,
    .block_num = 4U
};

static u32 sd_slot = 0U;
static EventGroupHandle_t sync = NULL;
static TaskHandle_t write_task = NULL;
static TimerHandle_t exit_timer = NULL;
static u32 run_times = 2U;
static boolean is_running = FALSE;
/***************** Macros (Inline Functions) Definitions *********************/
#define FSDIO_DEBUG_TAG "FSDIO-SD"
#define FSDIO_ERROR(format, ...)   FT_DEBUG_PRINT_E(FSDIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDIO_WARN(format, ...)    FT_DEBUG_PRINT_W(FSDIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDIO_INFO(format, ...)    FT_DEBUG_PRINT_I(FSDIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDIO_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FSDIO_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/

/*****************************************************************************/
static void SDExitCallback(TimerHandle_t timer)
{
    FError err = FT_SUCCESS;
    printf("Exiting.....\r\n");

    if (write_task)
    {
        vTaskDelete(write_task);
        write_task = NULL;
    }

    if (sync)
    {
        vEventGroupDelete(sync);
        sync = NULL;
    }

    if (pdPASS != xTimerDelete(timer, 0)) /* delete timer ifself */
    {
        FSDIO_ERROR("Delete exit timer failed.");
        exit_timer = NULL;
    }

    is_running = FALSE;
}

static void SDSendEvent(u32 evt_bits)
{
    FASSERT(sync);
    BaseType_t x_result = pdFALSE;

    FSDIO_DEBUG("Ack evt 0x%x", evt_bits);
    x_result = xEventGroupSetBits(sync, evt_bits);
}

static boolean SDWaitEvent(u32 evt_bits, TickType_t wait_delay)
{
    FASSERT(sync);
    EventBits_t ev;
    ev = xEventGroupWaitBits(sync, evt_bits,
                             pdTRUE, pdFALSE, wait_delay); /* wait for cmd/data done */
    if (ev & evt_bits)
    {
        return TRUE;
    }

    return FALSE;
}

static void SDInitTask(void *args)
{
    if (SDMMC_OK != sdmmc_host_init(test_info.cur_host, test_info.cur_host_config))
    {
        FSDIO_ERROR("Init sdio failed.");
        goto task_exit;
    }

    SDSendEvent(SD_EVT_INIT_DONE);

task_exit:
    vTaskDelete(NULL); /* delete task itself */
}

static void SDWriteReadTask(void *args)
{
    u32 times = 0U;
    const TickType_t wait_delay = pdMS_TO_TICKS(2000UL); /* wait for 2 seconds */
    FError err;
    const uintptr trans_len = test_info.block_num * 512U;
    char ch = 'A';

    if (trans_len > SD_WR_BUF_LEN)
    {
        FSDIO_ERROR("Trans length exceeds the buffer limits.");
        goto task_exit;
    }

    SDWaitEvent(SD_EVT_INIT_DONE, portMAX_DELAY);

    for (;;)
    {
        printf("Start reading ...\r\n");
        memset(sd_read_buffer, 0U, trans_len);
        if (SDMMC_OK != sdmmc_os_read_sectors(&(test_info.cur_host->card),
                                              sd_read_buffer,
                                              test_info.start_blk,
                                              test_info.block_num))
        {
            FSDIO_ERROR("Sdio read failed.");
            goto task_exit;
        }

        FCacheDCacheFlushRange((uintptr)(void *)sd_read_buffer, trans_len);
        printf("==>Read from Block [%d:%d]\r\n",
               test_info.start_blk,
               test_info.start_blk + test_info.block_num);

        FtDumpHexByte(sd_read_buffer, min(trans_len, (fsize_t)(2 * 512U)));

        /*************************************************************/

        printf("Start writing ...\r\n");
        memset(sd_write_buffer, (ch + times), trans_len);
        printf("==>Write %c to Block [%d:%d]\r\n",
               ch,
               test_info.start_blk,
               test_info.start_blk + test_info.block_num);

        if (SDMMC_OK != sdmmc_os_write_sectors(&(test_info.cur_host->card),
                                               sd_write_buffer,
                                               test_info.start_blk,
                                               test_info.block_num))
        {
            FSDIO_ERROR("Sdio write failed.");
            goto task_exit;
        }

        vTaskDelay(wait_delay);

        if (++times > run_times)
        {
            break;
        }
    }

task_exit:
    printf("Exit from write task.\r\n");
    vTaskSuspend(NULL); /* suspend task */
}

BaseType_t FFreeRTOSSdWriteRead(u32 slot_id, boolean is_emmc, u32 start_blk, u32 blk_num)
{
    BaseType_t ret = pdPASS;
    const TickType_t total_run_time = pdMS_TO_TICKS(30000UL); /* run for 10 secs deadline */

    if (is_running)
    {
        FSDIO_ERROR("Task is running.");
        return pdPASS;
    }

    is_running = TRUE;

    printf("This is sd write read task.\r\n");

    FASSERT_MSG(NULL == sync, "Event group exists.");
    FASSERT_MSG((sync = xEventGroupCreate()) != NULL, "Create event group failed.");

    test_info.cur_host = is_emmc ? &emmc_host : &tf_host;
    test_info.cur_host_config = is_emmc ? &emmc_host_config : &tf_host_config;

    test_info.cur_host_config->slot = slot_id;
    test_info.cur_host_config->type = SDMMC_HOST_TYPE_FSDIO;
    test_info.cur_host_config->flags = SDMMC_HOST_WORK_MODE_DMA | SDMMC_HOST_WORK_MODE_IRQ;
    test_info.cur_host_config->flags |= is_emmc ? 0U : SDMMC_HOST_REMOVABLE_CARD;

    test_info.start_blk = start_blk;
    test_info.block_num = blk_num;
    taskENTER_CRITICAL(); /* no schedule when create task */

    FtOut32((uintptr)0x32b31178, 0x1f); /* set delay of SDIO-1 on E2000 Demo otherwise detect may fail */

    ret = xTaskCreate((TaskFunction_t)SDInitTask,
                      (const char *)"SDInitTask",
                      (uint16_t)2048,
                      NULL,
                      (UBaseType_t)configMAX_PRIORITIES - 1,
                      NULL);
    FASSERT_MSG(pdPASS == ret, "Create task failed.");

    ret = xTaskCreate((TaskFunction_t)SDWriteReadTask,
                      (const char *)"SDWriteReadTask",
                      (uint16_t)2048,
                      NULL,
                      (UBaseType_t)configMAX_PRIORITIES - 2,
                      &write_task);

    FASSERT_MSG(pdPASS == ret, "Create task failed.");

    exit_timer = xTimerCreate("Exit-Timer",                 /* Text name for the software timer - not used by FreeRTOS. */
                              total_run_time,                 /* The software timer's period in ticks. */
                              pdFALSE,                        /* Setting uxAutoRealod to pdFALSE creates a one-shot software timer. */
                              NULL,                           /* use timer id to pass task data for reference. */
                              SDExitCallback);                /* The callback function to be used by the software timer being created. */

    FASSERT_MSG(NULL != exit_timer, "Create exit timer failed.");

    taskEXIT_CRITICAL(); /* allow schedule since task created */

    ret = xTimerStart(exit_timer, 0); /* start */

    FASSERT_MSG(pdPASS == ret, "Start exit timer failed.");

    return ret;
}