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
 * FilePath: atomic_example.c
 * Date: 2023-02-23 14:53:42
 * LastEditTime: 2023-03-01 17:57:36
 * Description:  This file is for atomic test function.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  wangxiaodong 2023/06/23	  first release
 */

#include <stdio.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "fatomic.h"
#include "fassert.h"
#include "fkernel.h"

static xTaskHandle xtask_handle;

#define TASK_STACK_SIZE         1024


void FAtomicExample(void *pvParameters)
{
    int ret;
    int i = 0;
    u32 count = 0;
    u32 times = 0;
    for (;;)
    {
        i = 0;
        count = 0;
        while (i++ < 10)
        {
            ret = FATOMIC_ADD(count, 1);
        }
        FASSERT_MSG(count == 10, "FATOMIC_ADD error\r\n");

        i = 0;
        while (i++ < 10) 
        {
            ret = FATOMIC_INC(count);
        }
        FASSERT_MSG(count == 20, "FATOMIC_INC error\r\n");

        i = 0;
        while (i++ < 10) 
        {
            ret = FATOMIC_SUB(count, 1);
        }
        FASSERT_MSG(count == 10, "FATOMIC_SUB error\r\n");

        i = 0;
        while (i++ < 10) 
        {
            ret = FATOMIC_DEC(count);
        }
        FASSERT_MSG(count == 0, "FATOMIC_DEC error\r\n");

        i = 0;
        count = 0;
        while (i++ < 16)
        {
            ret = FATOMIC_OR(count, BIT(i-1));
        }
        FASSERT_MSG(count == 0xFFFF, "FATOMIC_OR error\r\n");

        i = 0;
        count = 0xFFFF;
        while (i++ < 16)
        {
            ret = FATOMIC_AND(count, ~BIT(i-1));
        }
        FASSERT_MSG(count == 0, "FATOMIC_AND error\r\n");

        FATOMIC_CAS_BOOL(count, 0, 1);
        FASSERT_MSG(count == 1, "FATOMIC_CAS_BOOL error\r\n");

        FATOMIC_CAS_VAL(count, 0xFF, 0);
        FASSERT_MSG(count == 1, "FATOMIC_CAS_VAL error\r\n");

        FATOMIC_LOCK(count, 1);
        FASSERT_MSG(count == 1, "FATOMIC_LOCK error\r\n");

        FATOMIC_UNLOCK(count);
        FASSERT_MSG(count == 0, "FATOMIC_UNLOCK error\r\n");

        printf("Atomic example test success, times = %d!!! \r\n", times++);

        vTaskDelay(1000);
    }

}


void CreateAtomicTasks(void)
{
    printf("Create Atomic Task \r\n");
    xTaskCreate(FAtomicExample,   "AtomicPeriodic", TASK_STACK_SIZE, NULL, 6, &xtask_handle);

}

void DeleteAtomicTasks(void)
{
    if (xtask_handle)
    {
        vTaskDelete(xtask_handle);
        printf("AtomicPeriodic deletion \r\n");
    }
}