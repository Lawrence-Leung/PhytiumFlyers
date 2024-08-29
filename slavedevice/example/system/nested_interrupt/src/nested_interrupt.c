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
 * FilePath: nested_interrupt.c
 * Date: 2023-02-23 14:53:42
 * LastEditTime: 2023-03-01 17:57:36
 * Description:  This file is for nested interrupt test function.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  wangxiaodong 2023/2/23	  first release
 */

#include <stdio.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "croutine.h"
#include "finterrupt.h"
#include "fcpu_info.h"
#include "fexception.h"


static xTaskHandle xtask_handle;

#define TASK_STACK_SIZE         1024

/* The interrupt number to use for the software interrupt generation.  This
could be any unused number.  In this case the first chip level (non system)
interrupt is used */
#define INTERRUPT_LOW_ID         0
#define INTERRUPT_HIGH_ID        1

/* The priority of the software interrupt.  The interrupt service routine uses
an (interrupt safe) FreeRTOS API function, so the priority of the interrupt must
be equal to or lower than the priority set by
configMAX_SYSCALL_INTERRUPT_PRIORITY - remembering that on the Cortex M3 high
numeric values represent low priority values, which can be confusing as it is
counter intuitive. */
#define INTERRUPT_LOW_PRIORITY  IRQ_PRIORITY_VALUE_14
#define INTERRUPT_HIGH_PRIORITY  (INTERRUPT_LOW_PRIORITY-1)

/* Macro to force an interrupt. */
static void vTriggerNestedInterrupt(void);

static u32 cpu_id = 0;


static volatile u8 low_priority_intr_flag = 0;	/* Flag to update low priority interrupt counter */
static volatile u8 high_priority_intr_flag = 0;	/* Flag to update high priority interrupt counter */


static void vNestedPeriodTask(void *pvParameters)
{
    u8 count = 0;
    for (;;)
    {
        vTaskDelay(3000 / portTICK_RATE_MS);
        printf("Nested Interrupt Test %d.\r\n", count++);
        vTriggerNestedInterrupt();
    }
}


static void FLowPriorityHandler(s32 vector, void *param)
{
    static float val = 0.3;

    val = val * 2.1;
    
    /* update the flag to indicate the interrupt */
	low_priority_intr_flag++;

    /* Enable the nested interrupts to allow preemption */
    INTERRUPT_NESTED_ENABLE();

    InterruptCoreInterSend(INTERRUPT_HIGH_ID, (1 << cpu_id));

    printf("Nested interrupt enable.\r\n");

	/* wait till interrupts from counter configured with high priority interrupt */
	while(high_priority_intr_flag == 0)
    {
        
    };

    /* Disable the nested interrupt before exiting IRQ mode */
	INTERRUPT_NESTED_DISABLE();

    printf("low_priority_intr_flag is %d, %f \n\n\n", low_priority_intr_flag, val);

}

static void FHighPriorityHandler(s32 vector, void *param)
{
    /* update the flag to indicate the interrupt */
    high_priority_intr_flag++;
    static float val = 0.3;

    val = val * 2.0;

    printf("high_priority_intr_flag is %d, %f \r\n", high_priority_intr_flag, val);

}


static void prvSetupSoftwareNestedInterrupt()
{

    GetCpuId(&cpu_id);

    /* The interrupt service routine uses an (interrupt safe) FreeRTOS API
    function so the interrupt priority must be at or below the priority defined
    by configSYSCALL_INTERRUPT_PRIORITY. */
    InterruptSetPriority(INTERRUPT_LOW_ID, INTERRUPT_LOW_PRIORITY);

    InterruptInstall(INTERRUPT_LOW_ID, FLowPriorityHandler, NULL, NULL);

    /* Enable the interrupt. */
    InterruptUmask(INTERRUPT_LOW_ID);


    InterruptSetPriority(INTERRUPT_HIGH_ID, INTERRUPT_HIGH_PRIORITY);

    InterruptInstall(INTERRUPT_HIGH_ID, FHighPriorityHandler, NULL, NULL);

    /* Enable the interrupt. */
    InterruptUmask(INTERRUPT_HIGH_ID);

}

/* Macro to force an interrupt. */
static void vTriggerNestedInterrupt(void)
{
    low_priority_intr_flag = 0;

    high_priority_intr_flag = 0;

    InterruptCoreInterSend(INTERRUPT_LOW_ID, (1 << cpu_id));

}


void CreateNestedTasks(void)
{
    printf("Create Nest Task \r\n");
    prvSetupSoftwareNestedInterrupt();
    xTaskCreate(vNestedPeriodTask,   "NestedPeriodic", TASK_STACK_SIZE, NULL, 6, &xtask_handle);

}

void DeleteNestedTasks(void)
{
    if (xtask_handle)
    {
        vTaskDelete(xtask_handle);
        printf("Nest Periodic deletion \r\n");
    }

}