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
 * FilePath: fatfs_examples.c
 * Date: 2022-07-11 11:32:48
 * LastEditTime: 2022-07-11 11:32:48
 * Description:  This file is for the fatfs test example functions.
 *
 * Modify History:
 *  Ver   Who         Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/12/7    init commit
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "event_groups.h"

#include "fkernel.h"
#include "strto.h"
#include "fassert.h"
#include "fdebug.h"
#include "fparameters.h"
#include "sdkconfig.h"

#include "ff_utils.h"
#include "fatfs_examples.h"
/************************** Constant Definitions *****************************/
#define FATFS_EVT_INIT_DONE        (0x1 << 0)
#define FATFS_EVT_CYC_TEST_DONE    (0x1 << 1)

/************************** Variable Definitions *****************************/
static const char *mount_points[FFREERTOS_DISK_TYPE_NUM] =
{
    [FFREERTOS_FATFS_RAM_DISK] = FF_RAM_DISK_MOUNT_POINT,
    [FFREERTOS_FATFS_TF_CARD] = FF_FSDIO_TF_DISK_MOUNT_POINT,
    [FFREERTOS_FATFS_EMMC_CARD] = FF_FSDIO_EMMC_DISK_MOUNT_POINT,
    [FFREERTOS_FATFS_USB_DISK] = FF_USB_DISK_MOUNT_POINT,
    [FFREERTOS_FATFS_SATA_DISK] = FF_SATA_DISK_MOUNT_POINT,
    [FFREERTOS_FATFS_SATA_PCIE_DISK] = FF_SATA_PCIE_DISK_MOUNT_POINT
};
static const MKFS_PARM fs_option =
{
    .fmt = FM_EXFAT, /* format file system as exFAT to support > 4GB storage */
    .n_fat = 0, /* use default setting for other options */
    .align = 0,
    .n_root = 0,
    .au_size = 0
};
static boolean is_running = FALSE;
static EventGroupHandle_t sync = NULL;
static TimerHandle_t exit_timer = NULL;
static ff_fatfs file_sys[FFREERTOS_DISK_TYPE_NUM];

/***************** Macros (Inline Functions) Definitions *********************/
#define FF_DEBUG_TAG "FATFS"
#define FF_ERROR(format, ...)   FT_DEBUG_PRINT_E(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_WARN(format, ...)    FT_DEBUG_PRINT_W(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_INFO(format, ...)    FT_DEBUG_PRINT_I(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FF_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/

/*****************************************************************************/
static void FatfsSendEvent(u32 evt_bits)
{
    FASSERT(sync);
    BaseType_t x_result = pdFALSE;

    FF_DEBUG("Ack evt_bits is 0x%x.", evt_bits);
    x_result = xEventGroupSetBits(sync, evt_bits);
}

static boolean FatfsWaitEvent(u32 evt_bits, TickType_t wait_delay)
{
    FASSERT(sync);
    EventBits_t ev;
    ev = xEventGroupWaitBits(sync, evt_bits,
                             pdTRUE, pdFALSE, wait_delay);
    if (ev & evt_bits)
    {
        return TRUE;
    }

    return FALSE;
}

static void FatfsInitTask(void *args)
{
    FRESULT fr = FR_OK;

#ifdef CONFIG_FATFS_RAM_DISK
    fr = ff_setup(&file_sys[FFREERTOS_FATFS_RAM_DISK],
                  mount_points[FFREERTOS_FATFS_RAM_DISK],
                  &fs_option, pdFALSE);

    if (FR_OK != fr)
    {
        FF_ERROR("RAM disk init failed, err = %d.", fr);
        goto task_exit;
    }
#endif

#ifdef CONFIG_FATFS_SDMMC_FSDIO_TF
    fr = ff_setup(&file_sys[FFREERTOS_FATFS_TF_CARD],
                  mount_points[FFREERTOS_FATFS_TF_CARD],
                  &fs_option, pdFALSE);

    if (FR_OK != fr)
    {
        FF_ERROR("TF card init failed, err = %d.", fr);
        goto task_exit;
    }
#endif

#ifdef CONFIG_FATFS_SDMMC_FSDIO_EMMC
    fr = ff_setup(&file_sys[FFREERTOS_FATFS_EMMC_CARD],
                  mount_points[FFREERTOS_FATFS_EMMC_CARD],
                  &fs_option, pdFALSE);

    if (FR_OK != fr)
    {
        FF_ERROR("SDIO card init failed, err = %d.", fr);
        goto task_exit;
    }
#endif

#ifdef CONFIG_FATFS_USB
    fr = ff_setup(&file_sys[FFREERTOS_FATFS_USB_DISK],
                  mount_points[FFREERTOS_FATFS_USB_DISK],
                  &fs_option, pdFALSE);

    if (FR_OK != fr)
    {
        FF_ERROR("USB init failed, err = %d.", fr);
        goto task_exit;
    }
#endif

#ifdef CONFIG_FATFS_FSATA
    fr = ff_setup(&file_sys[FFREERTOS_FATFS_SATA_DISK],
                  mount_points[FFREERTOS_FATFS_SATA_DISK],
                  &fs_option, pdFALSE);

    if (FR_OK != fr)
    {
        FF_ERROR("SATA init failed, err = %d.", fr);
        goto task_exit;
    }
#endif
#ifdef CONFIG_FATFS_FSATA_PCIE
    fr = ff_setup(&file_sys[FFREERTOS_FATFS_SATA_PCIE_DISK],
                  mount_points[FFREERTOS_FATFS_SATA_PCIE_DISK],
                  &fs_option, pdFALSE);

    if (FR_OK != fr)
    {
        FF_ERROR("SATA PCIE init failed, err = %d.", fr);
        goto task_exit;
    }
#endif
    FatfsSendEvent(FATFS_EVT_INIT_DONE);
task_exit:
    vTaskDelete(NULL); /* delete task itself */
}

static void FatfsTestTask(void *args)
{
    const char *root = NULL;
    FRESULT fr = FR_OK;

    FatfsWaitEvent(FATFS_EVT_INIT_DONE, portMAX_DELAY);

#ifdef CONFIG_FATFS_BASIC_TEST
    {
#ifdef CONFIG_FATFS_RAM_DISK
        printf("\r\n========Basic test for RAM Disk=================\r\n");
        fr = ff_basic_test(mount_points[FFREERTOS_FATFS_RAM_DISK], "logfile.txt");
        if (FR_OK != fr)
        {
            FF_ERROR("RAM disk basic test failed, err = %d.", fr);
            goto task_exit;
        }
#endif

#ifdef CONFIG_FATFS_SDMMC_FSDIO_TF
        printf("\r\n========Basic test for TF Card=================\r\n");
        fr = ff_basic_test(mount_points[FFREERTOS_FATFS_TF_CARD], "logfile.txt");
        if (FR_OK != fr)
        {
            FF_ERROR("TF card basic test failed, err = %d.", fr);
            goto task_exit;
        }
#endif

#ifdef CONFIG_FATFS_SDMMC_FSDIO_EMMC
        printf("\r\n========Basic test for eMMC=================\r\n");
        fr = ff_basic_test(mount_points[FFREERTOS_FATFS_EMMC_CARD], "logfile.txt");
        if (FR_OK != fr)
        {
            FF_ERROR("SDIO basic test failed, err = %d.", fr);
            goto task_exit;
        }
#endif

#ifdef CONFIG_FATFS_USB
        printf("\r\n========Basic test for USB Disk=================\r\n");
        fr = ff_basic_test(mount_points[FFREERTOS_FATFS_USB_DISK], "logfile.txt");
        if (FR_OK != fr)
        {
            FF_ERROR("USB basic test failed, err = %d.", fr);
            goto task_exit;
        }
#endif

#ifdef CONFIG_FATFS_FSATA
        printf("\r\n========Basic test for SATA Disk=================\r\n");
        fr = ff_basic_test(mount_points[FFREERTOS_FATFS_SATA_DISK], "logfile.txt");
        if (FR_OK != fr)
        {
            FF_ERROR("SATA basic test failed, err = %d.", fr);
            goto task_exit;
        }
#endif

#ifdef CONFIG_FATFS_FSATA_PCIE
        printf("\r\n========Basic test for SATA PCIE Disk=================\r\n");
        fr = ff_basic_test(mount_points[FFREERTOS_FATFS_SATA_PCIE_DISK], "logfile.txt");
        if (FR_OK != fr)
        {
            FF_ERROR("SATA PCIE basic test failed, err = %d.", fr);
            goto task_exit;
        }
#endif
    }
#endif

    /* speed test will test diskio and destory file system */
#ifdef CONFIG_FATFS_SPEED_TEST
    {
#ifdef CONFIG_FATFS_RAM_DISK
        printf("\r\n========Speed test for RAM Disk=================\r\n");
        fr = ff_speed_bench(mount_points[FFREERTOS_FATFS_RAM_DISK], 300000U);
        if (FR_OK != fr)
        {
            FF_ERROR("RAM disk speed test failed, err = %d.", fr);
            goto task_exit;
        }
#endif

#ifdef CONFIG_FATFS_SDMMC_FSDIO_TF
        printf("\r\n========Speed test for TF Card=================\r\n");
        fr = ff_speed_bench(mount_points[FFREERTOS_FATFS_TF_CARD], 300000U);
        if (FR_OK != fr)
        {
            FF_ERROR("TF speed test failed, err = %d.", fr);
            goto task_exit;
        }
#endif

#ifdef CONFIG_FATFS_SDMMC_FSDIO_EMMC
        printf("\r\n========Speed test for eMMC=================\r\n");
        fr = ff_speed_bench(mount_points[FFREERTOS_FATFS_EMMC_CARD], 300000U);
        if (FR_OK != fr)
        {
            FF_ERROR("SDIO speed test failed, err = %d.", fr);
            goto task_exit;
        }
#endif

#ifdef CONFIG_FATFS_USB
        printf("\r\n========Speed test for USB Disk=================\r\n");
        fr = ff_speed_bench(mount_points[FFREERTOS_FATFS_USB_DISK], 300000U);
        if (FR_OK != fr)
        {
            FF_ERROR("USB speed test failed, err = %d.", fr);
            goto task_exit;
        }
#endif

#ifdef CONFIG_FATFS_FSATA
        printf("\r\n========Speed test for SATA Disk=================\r\n");
        fr = ff_speed_bench(mount_points[FFREERTOS_FATFS_SATA_DISK], 30000U);
        if (FR_OK != fr)
        {
            FF_ERROR("SATA speed test failed, err = %d.", fr);
            goto task_exit;
        }
#endif

#ifdef CONFIG_FATFS_FSATA_PCIE
        printf("\r\n========Speed test for SATA PCIE Disk=================\r\n");
        fr = ff_speed_bench(mount_points[FFREERTOS_FATFS_SATA_PCIE_DISK], 30000U);
        if (FR_OK != fr)
        {
            FF_ERROR("SATA PCIE speed test failed, err = %d.", fr);
            goto task_exit;
        }
#endif
    }
#endif

    /* cycle test will test diskio and destory file system */
#ifdef CONFIG_FATFS_CYCLE_TEST
    {
#ifdef CONFIG_FATFS_RAM_DISK
        printf("\r\n========Cycle test for RAM Disk=================\r\n");
        if (ff_cycle_test(mount_points[FFREERTOS_FATFS_RAM_DISK], 3))
        {
            goto task_exit;
        }
#endif

#ifdef CONFIG_FATFS_SDMMC_FSDIO_TF
        printf("\r\n========Cycle test for TF Disk=================\r\n");
        if (ff_cycle_test(mount_points[FFREERTOS_FATFS_TF_CARD], 3))
        {
            goto task_exit;
        }
#endif

#ifdef CONFIG_FATFS_SDMMC_FSDIO_EMMC
        printf("\r\n========Cycle test for SDIO Disk=================\r\n");
        if (ff_cycle_test(mount_points[FFREERTOS_FATFS_EMMC_CARD], 3))
        {
            goto task_exit;
        }
#endif

#ifdef CONFIG_FATFS_USB
        printf("\r\n========Cycle test for USB Disk=================\r\n");
        if (ff_cycle_test(mount_points[FFREERTOS_FATFS_USB_DISK], 3))
        {
            goto task_exit;
        }
#endif

#ifdef CONFIG_FATFS_FSATA
        printf("\r\n========Cycle test for SATA Disk=================\r\n");
        if (ff_cycle_test(mount_points[FFREERTOS_FATFS_SATA_DISK], 3))
        {
            goto task_exit;
        }
#endif

#ifdef CONFIG_FATFS_FSATA_PCIE
        printf("\r\n========Cycle test for SATA PCIE Disk=================\r\n");
        if (ff_cycle_test(mount_points[FFREERTOS_FATFS_SATA_PCIE_DISK], 3))
        {
            goto task_exit;
        }
#endif
    }
#endif

task_exit:
    FatfsSendEvent(FATFS_EVT_CYC_TEST_DONE);
    printf("Exit from test task.\r\n");
    vTaskDelete(NULL); /* delete task itself */
}

static void FatfsExitCallback(TimerHandle_t timer)
{
    if (sync)
    {
        vEventGroupDelete(sync);
        sync = NULL;
    }

    is_running = FALSE;
}

BaseType_t FFreeRTOSFatfsTest(void)
{
    BaseType_t ret = pdPASS;
    const TickType_t total_run_time = pdMS_TO_TICKS(30000UL); /* run for 30 secs deadline */

    if (is_running)
    {
        FF_ERROR("The task is running.");
        return pdPASS;
    }

    FASSERT_MSG(NULL == sync, "Event group exists.");
    FASSERT_MSG((sync = xEventGroupCreate()) != NULL, "Create event group failed.");

    taskENTER_CRITICAL(); /* no schedule when create task */

    ret = xTaskCreate((TaskFunction_t)FatfsInitTask,
                      (const char *)"FatfsInitTask",
                      (uint16_t)2048,
                      NULL,
                      (UBaseType_t)configMAX_PRIORITIES - 1,
                      NULL);
    FASSERT_MSG(pdPASS == ret, "Create task failed.");

    ret = xTaskCreate((TaskFunction_t)FatfsTestTask,
                      (const char *)"FatfsTestTask",
                      (uint16_t)2048,
                      NULL,
                      (UBaseType_t)configMAX_PRIORITIES - 1,
                      NULL);
    FASSERT_MSG(pdPASS == ret, "Create task failed.");

    exit_timer = xTimerCreate("FatfsExitTimer",             /* Text name for the software timer - not used by FreeRTOS. */
                              total_run_time,                 /* The software timer's period in ticks. */
                              pdFALSE,                        /* Setting uxAutoRealod to pdFALSE creates a one-shot software timer. */
                              NULL,                           /* use timer id to pass task data for reference. */
                              FatfsExitCallback);             /* The callback function to be used by the software timer being created. */

    FASSERT_MSG(NULL != exit_timer, "Create exit timer failed.");

    taskEXIT_CRITICAL(); /* allow schedule since task created */
    return ret;
}