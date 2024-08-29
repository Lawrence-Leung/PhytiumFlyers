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
 * FilePath: sfud_read_write.c
 * Date: 2022-07-12 09:53:00
 * LastEditTime: 2022-07-12 09:53:02
 * Description:  This file is for providing functions used in cmd_sf.c file.
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

#include "fdebug.h"
#include "fsleep.h"
#include "fkernel.h"

#include "sfud.h"

/************************** Constant Definitions *****************************/
#define SFUD_WR_BUF_LEN   64
#if defined(CONFIG_TARGET_E2000D)||defined(CONFIG_TARGET_E2000Q)
#define SFUD_FLASH_INDEX  SFUD_FSPIM2_INDEX
#elif defined(CONFIG_TARGET_PHYTIUMPI)
#define SFUD_FLASH_INDEX  SFUD_FSPIM0_INDEX
#endif

/************************** Variable Definitions *****************************/
static u32 flash_addr = 0x0;
static u8 flash_buffer[SFUD_WR_BUF_LEN];
/***************** Macros (Inline Functions) Definitions *********************/
#define FSPIM_DEBUG_TAG "SFUD-DEMO"
#define FSPIM_ERROR(format, ...)   FT_DEBUG_PRINT_E(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_WARN(format, ...)    FT_DEBUG_PRINT_W(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_INFO(format, ...)    FT_DEBUG_PRINT_I(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/

/*****************************************************************************/
static void SfudInitTask(void *args)
{
    sfud_err sfud_ret = sfud_init();
    if (SFUD_SUCCESS != sfud_ret)
    {
        goto task_exit;
    }

    const sfud_flash *flash = sfud_get_device(SFUD_FLASH_INDEX);
    if (NULL == flash)
    {
        FSPIM_ERROR("Flash not found.");
        goto task_exit;
    }

    /* print flash info */
    printf("Flash %s is found.\r\n", flash->name);
    printf("    manufacturer id: 0x%x \r\n", flash->chip.mf_id);
    printf("    memory-type id: 0x%x \r\n", flash->chip.type_id);
    printf("    capacity id: 0x%x \r\n", flash->chip.capacity_id);

    if (flash->chip.capacity < SZ_1M)
    {
        printf("    cacpity: %d KB \r\n", flash->chip.capacity / SZ_1K);
    }
    else
    {
        printf("    cacpity: %d MB\r\n", flash->chip.capacity / SZ_1M);
    }

    printf("    Erase granularity: %d Bytes\r\n", flash->chip.erase_gran);

task_exit:
    vTaskDelete(NULL); /* delete task itself */
}

static void SfudWriteTask(void *args)
{
    sfud_err sfud_ret;
    u32 in_chip_addr = flash_addr;
    const sfud_flash *flash = NULL;
    u8 status = 0;
    u8 *write_buf = flash_buffer;

    flash = sfud_get_device(SFUD_FLASH_INDEX);
    if (NULL == flash)
    {
        FSPIM_ERROR("Flash not found.");
        goto task_exit;
    }

    /* remove flash write protect */
    sfud_ret = sfud_write_status(flash, TRUE, status);
    if (SFUD_SUCCESS != sfud_ret)
    {
        FSPIM_ERROR("Write flash status failed.");
        goto task_exit;
    }

    /* get flash status */
    sfud_ret = sfud_read_status(flash, &status);
    if (SFUD_SUCCESS != sfud_ret)
    {
        FSPIM_ERROR("Read flash status failed.");
        goto task_exit;
    }
    else
    {
        printf("Flash status: 0x%x\r\n", status);
    }

    /* write to flash */
    taskENTER_CRITICAL(); /* no schedule when printf bulk */
    printf("Data to write @0x%x...\r\n", in_chip_addr);
    FtDumpHexByte(write_buf, SFUD_WR_BUF_LEN);
    taskEXIT_CRITICAL();

    /* erase before write */
    sfud_ret = sfud_erase(flash, in_chip_addr, SFUD_WR_BUF_LEN);
    if (SFUD_SUCCESS != sfud_ret)
    {
        FSPIM_ERROR("Erase flash failed.");
        goto task_exit;
    }

    sfud_ret = sfud_write(flash, in_chip_addr, SFUD_WR_BUF_LEN, write_buf);
    if (SFUD_SUCCESS != sfud_ret)
    {
        FSPIM_ERROR("Write flash failed.");
        goto task_exit;
    }

task_exit:
    vTaskDelete(NULL); /* delete task itself */
}

static void SfudReadTask(void *args)
{
    sfud_err sfud_ret;
    u32 in_chip_addr = flash_addr;
    const sfud_flash *flash = NULL;
    u8 status = 0;
    u8 *read_buf = flash_buffer;

    flash = sfud_get_device(SFUD_FLASH_INDEX);
    if (NULL == flash)
    {
        FSPIM_ERROR("Flash not found.");
        goto task_exit;
    }

    /* get flash status */
    sfud_ret = sfud_read_status(flash, &status);
    if (SFUD_SUCCESS != sfud_ret)
    {
        FSPIM_ERROR("Read flash status failed.");
        goto task_exit;
    }
    else
    {
        printf("Flash status: 0x%x\r\n", status);
    }

    /* read from flash */
    memset(read_buf, 0, SFUD_WR_BUF_LEN);
    sfud_ret = sfud_read(flash, in_chip_addr, SFUD_WR_BUF_LEN, read_buf);
    if (SFUD_SUCCESS != sfud_ret)
    {
        FSPIM_ERROR("Read flash failed.");
        goto task_exit;
    }

    taskENTER_CRITICAL(); /* no schedule when printf bulk */
    printf("Data read from flash @0x%x...\r\n", in_chip_addr);
    FtDumpHexByte(read_buf, SFUD_WR_BUF_LEN);
    taskEXIT_CRITICAL();

task_exit:
    vTaskDelete(NULL); /* delete task itself */
}

BaseType_t FFreeRTOSSfudRead(u32 in_chip_addr)
{
    BaseType_t xReturn = pdPASS;

    printf("This is sfud read task.\r\n");

    memset(flash_buffer, 0, sizeof(flash_buffer));
    flash_addr = in_chip_addr;

    taskENTER_CRITICAL(); /* no schedule when create task */

    xReturn = xTaskCreate((TaskFunction_t)SfudReadTask,
                          (const char *)"SfudReadTask",
                          (uint16_t)2048,
                          NULL,
                          (UBaseType_t)configMAX_PRIORITIES - 1,
                          NULL);

    taskEXIT_CRITICAL(); /* allow schedule since task created */

    return xReturn;
}

BaseType_t FFreeRTOSSfudWrite(u32 in_chip_addr, const char *content)
{
    BaseType_t xReturn = pdPASS;

    printf("This is sfud write task.\r\n");

    flash_addr = in_chip_addr;
    if (strlen(content) + 1 > SFUD_WR_BUF_LEN)
    {
        return pdFAIL;
    }

    memset(flash_buffer, 0, sizeof(flash_buffer));
    memcpy(flash_buffer, content, strlen(content) + 1);

    taskENTER_CRITICAL(); /* no schedule when create task */

    xReturn = xTaskCreate((TaskFunction_t)SfudWriteTask,
                          (const char *)"SfudWriteTask",
                          (uint16_t)2048,
                          NULL,
                          (UBaseType_t)configMAX_PRIORITIES - 1,
                          NULL);

    taskEXIT_CRITICAL(); /* allow schedule since task created */

    return xReturn;
}

BaseType_t FFreeRTOSSfudInit(void)
{
    BaseType_t xReturn = pdPASS;

    printf("This is sfud init task.\r\n");

    taskENTER_CRITICAL(); /* no schedule when create task */

    xReturn = xTaskCreate((TaskFunction_t)SfudInitTask,
                          (const char *)"SfudInitTask",
                          (uint16_t)2048,
                          NULL,
                          (UBaseType_t)configMAX_PRIORITIES - 1,
                          NULL);

    taskEXIT_CRITICAL(); /* allow schedule since task created */

    return pdPASS;
}