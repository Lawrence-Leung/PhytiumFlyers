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
 * FilePath: freertos_configs.c
 * Date: 2022-02-24 13:42:19
 * LastEditTime: 2022-03-21 17:03:31
 * Description:  This file is for the freertos config functions
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  wangxiaodong  2021/9/26  first release
 * 1.1  wangxiaodong  2021/12/24 adapt new standalone
 * 1.2  wangxiaodong  2022/6/20  v0.1.0
 * 2.0  wangxiaodong  2022/8/9   adapt E2000D
 * 2.1  liuzhihong    2023/1/12  improve lwip functions
 */
#include <stdio.h>
#include <stdarg.h>
#include "FreeRTOS.h"
#include "task.h"
#include "ftypes.h"
#include "fparameters.h"
#include "fgeneric_timer.h"
#include "finterrupt.h"
#include "fcpu_info.h"
#include "fassert.h"
#include "fexception.h"
#include "fprintf.h"

static volatile u32 is_in_irq = 0 ;

void vMainAssertCalled(const char *pcFileName, uint32_t ulLineNumber)
{
    printf("Assert Error is %s : %d .\r\n", pcFileName, ulLineNumber);
    for (;;)
        ;
}

void vApplicationMallocFailedHook(void)
{
    u32 cpu_id;
    GetCpuId(&cpu_id);
    printf("CPU %d Malloc Failed.\r\n", cpu_id);
    while (1)
        ;
}



/*
void vApplicationIdleHook(void)
{

}
*/

u32 PlatformGetGicDistBase(void)
{
    return GICV3_BASE_ADDR;
}

static u32 cntfrq; /* System frequency */

void vConfigureTickInterrupt(void)
{
    /* Disable the timer */
    GenericTimerStop(GENERIC_TIMER_ID0);
    /* Get system frequency */
    cntfrq = GenericTimerFrequecy();

    /* Set tick rate */
    GenericTimerSetTimerValue(GENERIC_TIMER_ID0, cntfrq / configTICK_RATE_HZ);
    GenericTimerInterruptEnable(GENERIC_TIMER_ID0);

    /* Set as the lowest priority */
    InterruptSetPriority(GENERIC_TIMER_NS_IRQ_NUM, configKERNEL_INTERRUPT_PRIORITY);
    InterruptUmask(GENERIC_TIMER_NS_IRQ_NUM);

    GenericTimerStart(GENERIC_TIMER_ID0);
}

void vClearTickInterrupt(void)
{
    GenericTimerSetTimerValue(GENERIC_TIMER_ID0, cntfrq / configTICK_RATE_HZ);
}

volatile unsigned int gCpuRuntime;

void vApplicationInterruptHandler(uint32_t ulICCIAR)
{
    int ulInterruptID;
    is_in_irq ++;
    /* Interrupts cannot be re-enabled until the source of the interrupt is
    cleared. The ID of the interrupt is obtained by bitwise ANDing the ICCIAR
    value with 0x3FF. */
    ulInterruptID = ulICCIAR & 0x3FFUL;

    /* call handler function */
    if (ulInterruptID == GENERIC_TIMER_NS_IRQ_NUM)
    {
        /* Generic Timer */
        gCpuRuntime++;
        FreeRTOS_Tick_Handler();
    }
    else
    {
        FExceptionInterruptHandler((void *)(uintptr)ulInterruptID);
    }
    is_in_irq --;
}


/**
 * @name: vApplicationInIrq
 * @msg:  Used to indicate whether you are currently in an interrupt
 * @return {int} 1:is in an irq ,0 is not in
 * @note:
 */

int vApplicationInIrq(void)
{
    return is_in_irq;
}

static InterruptDrvType finterrupt;

void vApplicationInitIrq(void)
{
    InterruptInit(&finterrupt, INTERRUPT_DRV_INTS_ID, INTERRUPT_ROLE_MASTER);
}


void vApplicationStackOverflowHook(xTaskHandle pxTask, signed char *pcTaskName)
{
    (void) pxTask;
    (void) pcTaskName;

    taskDISABLE_INTERRUPTS();
    FASSERT(FALSE);

}

/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must provide an
 * implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
 * used by the Idle task. */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize)
{
    /* If the buffers to be provided to the Idle task are declared inside this
     * function then they must be declared static - otherwise they will be allocated on
     * the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
      state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
      Note that, as the array is necessarily of type StackType_t,
      configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/* configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
 * application must provide an implementation of `vApplicationGetTimerTaskMemory()
 * to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize)
{
    /* If the buffers to be provided to the Timer task are declared inside this
     * function then they must be declared static - otherwise they will be allocated on
     * the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
      task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
      Note that, as the array is necessarily of type StackType_t,
      configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

void vPrintString(const char *pcString)
{
    /* Print the string, using a critical section as a crude method of mutual
    exclusion. */
    taskENTER_CRITICAL();
    {
        printf("%s\r\n", pcString);
    }
    taskEXIT_CRITICAL();
}

void vPrintStringAndNumber(const char *pcString, uint32_t ulValue)
{
    /* Print the string, using a critical section as a crude method of mutual
    exclusion. */
    taskENTER_CRITICAL();
    {
        printf("%s %lu\r\n", pcString, ulValue);
    }
    taskEXIT_CRITICAL();
}

void vPrintf(const char *format, ...)
{
    /* Print the string, using a critical section as a crude method of mutual exclusion. */
    taskENTER_CRITICAL();
    {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
    taskEXIT_CRITICAL();
}

