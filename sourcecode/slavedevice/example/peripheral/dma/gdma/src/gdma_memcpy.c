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
 * FilePath: gdma_memcpy.c
 * Date: 2022-07-20 11:07:42
 * LastEditTime: 2022-07-20 11:16:57
 * Description:  This files is for GDMA task implementations 
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 zhugengyu    2022/08/26   first commit
 */
/***************************** Include Files *********************************/
#include <string.h>

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "fkernel.h"
#include "fcache.h"
#include "fassert.h"
#include "fdebug.h"
#include "fio_mux.h"

#include "fgdma_os.h"
/************************** Constant Definitions *****************************/
#define GDMA_CHAN_TRANS_END(chan)           (0x1U << (chan))
#define GDMA_BUF_A_LEN                      200U
#define GDMA_BUF_B_LEN                      1024U
#define GDMA_WORK_TASK_NUM                  2U
/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static FFreeRTOSGdma *gdma = NULL;
static TaskHandle_t task_a = NULL;
static TaskHandle_t task_b = NULL;
static TimerHandle_t exit_timer = NULL;
static EventGroupHandle_t chan_evt = NULL;
static xSemaphoreHandle init_locker = NULL;
static u8 src_a[GDMA_BUF_A_LEN] __attribute__((aligned(4))) = {0U};
static u8 dst_a[GDMA_BUF_A_LEN] __attribute__((aligned(4))) = {0U};
static u8 src_b[GDMA_BUF_B_LEN] __attribute__((aligned(4))) = {0U};
static u8 dst_b[GDMA_BUF_B_LEN] __attribute__((aligned(4))) = {0U};
static u32 chan_evt_bits = 0U; /* bits by GDMA_CHAN_TRANS_END */
static u32 memcpy_times = 3U;
static boolean is_running = FALSE;

/***************** Macros (Inline Functions) Definitions *********************/
#define FGDMA_DEBUG_TAG "GDMA-MEM"
#define FGDMA_ERROR(format, ...)   FT_DEBUG_PRINT_E(FGDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGDMA_WARN(format, ...)    FT_DEBUG_PRINT_W(FGDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGDMA_INFO(format, ...)    FT_DEBUG_PRINT_I(FGDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGDMA_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FGDMA_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/

/*****************************************************************************/

static void GdmaMemcpyExitCallback(TimerHandle_t timer)
{
    printf("exiting...\r\n");

    if (task_a) /* stop and delete send task */
    {
        vTaskDelete(task_a);
        task_a = NULL;
    }

    if (task_b) /* stop and delete recv task */
    {
        vTaskDelete(task_b);
        task_a = NULL;
    }

    if (chan_evt)
    {
        vEventGroupDelete(chan_evt);
        chan_evt = NULL;
        chan_evt_bits = 0U;
    }

    if (init_locker)
    {
        vSemaphoreDelete(init_locker);
        init_locker = NULL;
    }

    if (gdma)
    {
        if (FT_SUCCESS != FFreeRTOSGdmaDeInit(gdma))
        {
            FGDMA_ERROR("delete gdma failed !!!");
        }
        gdma = NULL;
    }

    if (exit_timer)
    {
        if (pdPASS != xTimerDelete(exit_timer, 0))
        {
            FGDMA_ERROR("delete exit timer failed !!!");
        }
        exit_timer = NULL;
    }

    is_running = FALSE;
}

static FFreeRTOSGdmaRequest *GdmaPrepareRequest(u8 *src, u8 *dst, fsize_t buf_len, fsize_t trans_num)
{
    FASSERT_MSG(buf_len % trans_num == 0, "invalid transaction num");
    FFreeRTOSGdmaRequest *req = pvPortMalloc(sizeof(FFreeRTOSGdmaRequest));
    fsize_t loop;
    fsize_t pre_buf_len = buf_len / trans_num;

    req->trans = pvPortMalloc(sizeof(FFreeRTOSGdmaTranscation) * trans_num);
    req->valid_trans_num = 0U;
    req->total_trans_num = trans_num;
    for (loop = 0; loop < trans_num; loop++)
    {
        req->trans[loop].src_buf = src + pre_buf_len * loop;
        req->trans[loop].dst_buf = dst + pre_buf_len * loop;
        req->trans[loop].data_len = pre_buf_len;
        FGDMA_INFO("src: %p, dst: %p, len: %d.", req->trans[loop].src_buf, req->trans[loop].dst_buf,
                   req->trans[loop].data_len);
        req->valid_trans_num++;
    }

    return req;
}

static void GdmaMemcpyAckChanXEnd(FGdmaChan *const chan, void *args)
{
    FASSERT(chan);
    FGdmaChanIndex chan_id = chan->config.chan_id;

    BaseType_t xhigher_priority_task_woken = pdFALSE;
    BaseType_t x_result = pdFALSE;

    FGDMA_INFO("ack gdma chan %d.", chan_id);
    x_result = xEventGroupSetBitsFromISR(chan_evt, GDMA_CHAN_TRANS_END(chan_id),
                                         &xhigher_priority_task_woken);

    return;
}

static boolean GdmaMemcpyWaitChanXEnd(u32 chan_id)
{
    const TickType_t wait_delay = pdMS_TO_TICKS(5000UL); /* wait for 5 seconds */
    EventBits_t ev;
    boolean ok = FALSE;

    /* block task to wait memcpy finish signal */
    ev = xEventGroupWaitBits(chan_evt, GDMA_CHAN_TRANS_END(chan_id),
                             pdTRUE, pdTRUE, wait_delay); /* wait for all bits */
    if ((ev & GDMA_CHAN_TRANS_END(chan_id))) /* wait until channel finished memcpy */
    {
        FGDMA_INFO("memcpy finished !! chan bits: 0x%x.", chan_evt_bits);
        ok = TRUE;
    }
    else
    {
        FGDMA_ERROR("wait memcpy timeout !!! 0x%x != 0x%x.", ev, chan_evt_bits);
        ok = FALSE;
    }

    return ok;
}

static void GdmaInitTask(void *args)
{
    u32 gdma_id = FGDMA0_ID;

    gdma = FFreeRTOSGdmaInit(gdma_id);
    FASSERT_MSG(gdma, "init gdma failed");

    FASSERT_MSG(init_locker, "init locker NULL");
    for (u32 loop = 0U; loop < GDMA_WORK_TASK_NUM; loop++)
    {
        xSemaphoreGive(init_locker);
    }

    vTaskDelete(NULL);
}

static void GdmaMemcpyTaskA(void *args)
{
    FASSERT(init_locker);
    xSemaphoreTake(init_locker, portMAX_DELAY);

    char ch = 'A';
    u32 chan_id = 0U;
    u8 times = 0U;
    FError err = FT_SUCCESS;
    const TickType_t wait_delay = pdMS_TO_TICKS(2000UL); /* wait for 2 seconds */
    FFreeRTOSGdmaRequest *req_a = GdmaPrepareRequest(src_a, dst_a, GDMA_BUF_A_LEN, 5);

    req_a->req_done_handler = GdmaMemcpyAckChanXEnd;
    req_a->req_done_args = NULL;

    err = FFreeRTOSGdmaSetupChannel(gdma, chan_id, req_a);
    if (FT_SUCCESS != err)
    {
        FGDMA_ERROR("setup chan-%d failed.", chan_id);
        goto task_err;
    }

    for (;;)
    {
        FGDMA_INFO("[A]start memcpy data ...");

        /* recv task has high priority, send task will not run before recv task blocked */
        ch = (char)('A' + (times) % 10); /* send different content each time */

        memset(src_a, ch, GDMA_BUF_A_LEN);
        memset(dst_a, 0, GDMA_BUF_A_LEN);

        FCacheDCacheInvalidateRange((uintptr)src_a, GDMA_BUF_A_LEN);
        FCacheDCacheInvalidateRange((uintptr)dst_a, GDMA_BUF_A_LEN);

        if (FT_SUCCESS != FFreeRTOSGdmaStart(gdma, chan_id))
        {
            FGDMA_ERROR("[A]start failed !!!");
            goto task_err;
        }

        if (!GdmaMemcpyWaitChanXEnd(chan_id))
        {
            goto task_err;
        }

        FCacheDCacheInvalidateRange((uintptr)src_a, GDMA_BUF_A_LEN);
        FCacheDCacheInvalidateRange((uintptr)dst_a, GDMA_BUF_A_LEN);

        /* compare if memcpy success */
        if (0 == memcmp(src_a, dst_a, GDMA_BUF_A_LEN))
        {
            taskENTER_CRITICAL();
            printf("[A]memcpy success !!!\r\n");
            printf("[A]src buf...\r\n");
            FtDumpHexByte((const u8 *)src_a, min((fsize_t)GDMA_BUF_A_LEN, (fsize_t)64U));
            printf("[A]dst buf...\r\n");
            FtDumpHexByte((const u8 *)dst_a, min((fsize_t)GDMA_BUF_A_LEN, (fsize_t)64U));
            taskEXIT_CRITICAL();
        }
        else
        {
            FGDMA_ERROR("[A]src != dst, memcpy failed !!!");
            goto task_err;
        }

        if (times++ > memcpy_times)
        {
            break;
        }

        vTaskDelay(wait_delay);
    }

task_err:
    (void)FFreeRTOSGdmaRevokeChannel(gdma, chan_id);
    printf("[A]exit from memcpy task !!!\r\n");
    vTaskSuspend(NULL); /* failed, not able to run, suspend task itself */
}

static void GdmaMemcpyTaskB(void *args)
{
    FASSERT(init_locker);
    xSemaphoreTake(init_locker, portMAX_DELAY);

    char ch = '0';
    u8 times = 0U;
    u32 chan_id = 1U;
    FError err = FT_SUCCESS;
    const TickType_t wait_delay = pdMS_TO_TICKS(2000UL); /* wait for 2 seconds */
    FFreeRTOSGdmaRequest *req_b = GdmaPrepareRequest(src_b, dst_b, GDMA_BUF_B_LEN, chan_id);

    req_b->req_done_handler = GdmaMemcpyAckChanXEnd;
    req_b->req_done_args = NULL;

    err = FFreeRTOSGdmaSetupChannel(gdma, chan_id, req_b);
    if (FT_SUCCESS != err)
    {
        FGDMA_ERROR("setup chan-%d failed.", chan_id);
        goto task_err;
    }

    for (;;)
    {
        FGDMA_INFO("[B]start memcpy ...");

        /* recv task has high priority, send task will not run before recv task blocked */
        ch = (char)('0' + (times) % 10); /* send different content each time */

        memset(src_b, ch, GDMA_BUF_B_LEN);
        memset(dst_b, 0, GDMA_BUF_B_LEN);

        FCacheDCacheInvalidateRange((uintptr)src_b, GDMA_BUF_B_LEN);
        FCacheDCacheInvalidateRange((uintptr)dst_b, GDMA_BUF_B_LEN);

        if (FT_SUCCESS != FFreeRTOSGdmaStart(gdma, chan_id))
        {
            FGDMA_ERROR("[B]start failed !!!");
            goto task_err;
        }

        if (!GdmaMemcpyWaitChanXEnd(chan_id))
        {
            goto task_err;
        }

        /* compare if memcpy success */
        if (0 == memcmp(src_b, dst_b, GDMA_BUF_B_LEN))
        {
            taskENTER_CRITICAL();
            printf("[B]memcpy success !!!\r\n");
            printf("[B]src buf...\r\n");
            FtDumpHexByte((const u8 *)src_b, min((fsize_t)GDMA_BUF_B_LEN, (fsize_t)64U));
            printf("[B]dst buf...\r\n");
            FtDumpHexByte((const u8 *)dst_b, min((fsize_t)GDMA_BUF_B_LEN, (fsize_t)64U));
            taskEXIT_CRITICAL();
        }
        else
        {
            FGDMA_ERROR("[B]src != dst, memcpy failed !!!");
            goto task_err;
        }

        if (times++ > memcpy_times)
        {
            break;
        }

        vTaskDelay(wait_delay);
    }

task_err:
    (void)FFreeRTOSGdmaRevokeChannel(gdma, chan_id);
    printf("[B]exit from memcpy task !!!\r\n");
    vTaskSuspend(NULL); /* failed, not able to run, suspend task itself */
}

BaseType_t FFreeRTOSRunGdmaMemcpy(void)
{
    BaseType_t ret = pdPASS;
    const TickType_t total_run_time = pdMS_TO_TICKS(30000UL); /* loopback run for 30 secs deadline */

    if (is_running)
    {
        FGDMA_ERROR("task is running !!!!");
        return pdPASS;
    }

    is_running = TRUE;

    FASSERT_MSG(NULL == chan_evt, "event group exists !!!");
    FASSERT_MSG((chan_evt = xEventGroupCreate()) != NULL, "create event group failed !!!");

    FASSERT_MSG(NULL == init_locker, "init locker exists !!!");
    FASSERT_MSG((init_locker = xSemaphoreCreateCounting(GDMA_WORK_TASK_NUM, 0U)) != NULL, "create event group failed !!!");

    taskENTER_CRITICAL(); /* no schedule when create task */

    ret = xTaskCreate((TaskFunction_t)GdmaInitTask,  /* task entry */
                      (const char *)"GdmaInitTask",/* task name */
                      (uint16_t)1024,  /* task stack size in words */
                      NULL, /* task params */
                      (UBaseType_t)configMAX_PRIORITIES - 1,  /* task priority */
                      NULL); /* task handler */

    FASSERT_MSG(pdPASS == ret, "create task failed");

    ret = xTaskCreate((TaskFunction_t)GdmaMemcpyTaskA,  /* task entry */
                      (const char *)"GdmaMemcpyTaskA",/* task name */
                      (uint16_t)4096,  /* task stack size in words */
                      NULL, /* task params */
                      (UBaseType_t)configMAX_PRIORITIES - 1,  /* task priority */
                      (TaskHandle_t *)&task_a); /* task handler */

    FASSERT_MSG(pdPASS == ret, "create task failed");

    ret = xTaskCreate((TaskFunction_t)GdmaMemcpyTaskB,  /* task entry */
                      (const char *)"GdmaMemcpyTaskB",/* task name */
                      (uint16_t)4096,  /* task stack size in words */
                      NULL, /* task params */
                      (UBaseType_t)configMAX_PRIORITIES - 2,  /* task priority */
                      (TaskHandle_t *)&task_b); /* task handler */

    FASSERT_MSG(pdPASS == ret, "create task failed");

    exit_timer = xTimerCreate("Exit-Timer",                 /* Text name for the software timer - not used by FreeRTOS. */
                              total_run_time,                 /* The software timer's period in ticks. */
                              pdFALSE,                        /* Setting uxAutoRealod to pdFALSE creates a one-shot software timer. */
                              NULL,                           /* use timer id to pass task data for reference. */
                              GdmaMemcpyExitCallback);        /* The callback function to be used by the software timer being created. */

    FASSERT_MSG(NULL != exit_timer, "create exit timer failed");

    taskEXIT_CRITICAL(); /* allow schedule since task created */

    ret = xTimerStart(exit_timer, 0); /* start */

    FASSERT_MSG(pdPASS == ret, "start exit timer failed");

    return ret;
}