/*
 * FreeRTOS V202011.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "sdkconfig.h"
#if !defined(__ASSEMBLER__)
    #include "fparameters.h"
    #include "finterrupt.h"
#endif

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html
 *----------------------------------------------------------*/

/*
 * The FreeRTOS Cortex-A port implements a full interrupt nesting model.
 *
 * Interrupts that are assigned a priority at or below
 * configMAX_API_CALL_INTERRUPT_PRIORITY (which counter-intuitively in the ARM
 * generic interrupt controller [GIC] means a priority that has a numerical
 * value above configMAX_API_CALL_INTERRUPT_PRIORITY) can call FreeRTOS safe API
 * functions and will nest.
 *
 * Interrupts that are assigned a priority above
 * configMAX_API_CALL_INTERRUPT_PRIORITY (which in the GIC means a numerical
 * value below configMAX_API_CALL_INTERRUPT_PRIORITY) cannot call any FreeRTOS
 * API functions, will nest, and will not be masked by FreeRTOS critical
 * sections (although it is necessary for interrupts to be globally disabled
 * extremely briefly as the interrupt mask is updated in the GIC).
 *
 * FreeRTOS functions that can be called from an interrupt are those that end in
 * "FromISR".  FreeRTOS maintains a separate interrupt safe API to enable
 * interrupt entry to be shorter, faster, simpler and smaller.
 *
 */

/* the interrupt priority used by the tick interrupt */
#define configKERNEL_INTERRUPT_PRIORITY         ( CONFIG_FREERTOS_KERNEL_INTERRUPT_PRIORITIES )
/* the highest interrupt priority from which interrupt-safe FreeRTOS API functions can be called */
#define configMAX_API_CALL_INTERRUPT_PRIORITY   ( CONFIG_FREERTOS_MAX_API_CALL_INTERRUPT_PRIORITIES )

#ifdef CONFIG_FREERTOS_OPTIMIZED_SCHEDULER
    #define configUSE_PORT_OPTIMISED_TASK_SELECTION         1
#endif

#ifdef CONFIG_FREERTOS_USE_TICKLESS_IDLE
    #define configUSE_TICKLESS_IDLE 1
    #define configEXPECTED_IDLE_TIME_BEFORE_SLEEP           CONFIG_FREERTOS_IDLE_TIME_BEFORE_SLEEP
#endif

#define configTICK_RATE_HZ          ( CONFIG_FREERTOS_HZ )
#define configUSE_PREEMPTION 1
#define configUSE_IDLE_HOOK 1
#define configUSE_TICK_HOOK 1
#define configMAX_PRIORITIES        ( CONFIG_FREERTOS_MAX_PRIORITIES )
#define configMINIMAL_STACK_SIZE    ( CONFIG_FREERTOS_MINIMAL_TASK_STACKSIZE )
#define configTOTAL_HEAP_SIZE       ( CONFIG_FREERTOS_TOTAL_HEAP_SIZE * 1024)
#define configMAX_TASK_NAME_LEN     ( CONFIG_FREERTOS_MAX_TASK_NAME_LEN )
#ifdef CONFIG_FREERTOS_USE_TRACE_FACILITY
    #define configUSE_TRACE_FACILITY                        1       /* Used by uxTaskGetSystemState(), and other trace facility functions */
#endif

#define configUSE_TASK_FPU_SUPPORT (CONFIG_FREERTOS_TASK_FPU_SUPPORT)


/* 与宏 configUSE_TRACE_FACILITY 同时为 1 时会编译下面 3 个函数
* prvWriteNameToBuffer()
* vTaskList(),
* vTaskGetRunTimeStats()
*/
#define configUSE_16_BIT_TICKS 0 //use 32 bit ticktype
#define configIDLE_SHOULD_YIELD 1 //idle task would not yield for task with same priority
#define configUSE_MUTEXES 1
#define configQUEUE_REGISTRY_SIZE ( CONFIG_FREERTOS_QUEUE_REGISTRY_SIZE )
#define configCHECK_FOR_STACK_OVERFLOW 0
#define configUSE_RECURSIVE_MUTEXES 1
#define configUSE_MALLOC_FAILED_HOOK 1
#define configUSE_APPLICATION_TASK_TAG 0
#define configUSE_COUNTING_SEMAPHORES 1
#define configUSE_QUEUE_SETS 1
#define configSUPPORT_STATIC_ALLOCATION 1 //use dynamic memory allocation
#define configSUPPORT_DYNAMIC_ALLOCATION 1

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 0 //disable co-routines
#define configMAX_CO_ROUTINE_PRIORITIES (2)

/* Software timer definitions. */
#define configUSE_TIMERS 1
#define configTIMER_TASK_PRIORITY       ( CONFIG_FREERTOS_TIMER_TASK_PRIORITY )
#define configTIMER_QUEUE_LENGTH        ( CONFIG_FREERTOS_TIMER_QUEUE_LENGTH )
#define configTIMER_TASK_STACK_DEPTH    ( CONFIG_FREERTOS_TIMER_TASK_STACK_DEPTH )

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#define INCLUDE_vTaskPrioritySet 1
#define INCLUDE_uxTaskPriorityGet 1
#define INCLUDE_vTaskDelete 1
#define INCLUDE_vTaskCleanUpResources 1
#define INCLUDE_vTaskSuspend 1
#define INCLUDE_vTaskDelayUntil 1
#define INCLUDE_vTaskDelay 1
#define INCLUDE_xTimerPendFunctionCall 1
#define INCLUDE_eTaskGetState 1
#define INCLUDE_xTaskAbortDelay 1
#define INCLUDE_xTaskGetTaskHandle 1
#define INCLUDE_xTaskGetHandle 1

#define INCLUDE_xSemaphoreGetMutexHolder 1
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS  ( CONFIG_FREERTOS_THREAD_LOCAL_STORAGE_POINTERS )
#define INCLUDE_xTaskGetIdleTaskHandle 1

/* This demo makes use of one or more example stats formatting functions.  These
format the raw data provided by the uxTaskGetSystemState() function in to human
readable ASCII form.  See the notes in the implementation of vTaskList() within
FreeRTOS/Source/tasks.c for limitations. */
#ifdef CONFIG_FREERTOS_USE_STATS_FORMATTING_FUNCTIONS
    #define configUSE_STATS_FORMATTING_FUNCTIONS            1   /* Used by vTaskList() */
#endif


/* Run time stats are not generated.  portCONFIGURE_TIMER_FOR_RUN_TIME_STATS and
portGET_RUN_TIME_COUNTER_VALUE must be defined if configGENERATE_RUN_TIME_STATS
is set to 1. */
#ifdef CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS
    #define configGENERATE_RUN_TIME_STATS                   1
#endif

#ifndef __ASSEMBLER__ // skip when preprocess asm
    extern volatile unsigned int gCpuRuntime;
    #define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() (gCpuRuntime = 0ul)
    #define portGET_RUN_TIME_COUNTER_VALUE() gCpuRuntime
#endif

/* The size of the global output buffer that is available for use when there
are multiple command interpreters running at once (for example, one on a UART
and one on TCP/IP).  This is done to prevent an output buffer being defined by
each implementation - which would waste RAM.  In this case, there is only one
command interpreter running. */
#define configCOMMAND_INT_MAX_OUTPUT_SIZE 2096

/* Normal assert() semantics without relying on the provision of an assert.h
header file. */

#define configASSERT(x)                            \
    do  \
    {   \
        void vMainAssertCalled(const char *pcFileName, uint32_t ulLineNumber);\
        if ((x) == 0)   \
        {               \
            vMainAssertCalled(__FILE__, __LINE__);\
        }               \
    } while (0);


/****** Hardware specific settings. *******************************************/

/*
 * The application must provide a function that configures a peripheral to
 * create the FreeRTOS tick interrupt, then define configSETUP_TICK_INTERRUPT()
 * in FreeRTOSConfig.h to call the function.    FreeRTOS_Tick_Handler() must
 * be installed as the peripheral's interrupt handler.
 */

#define configSETUP_TICK_INTERRUPT() \
    do                               \
    {                                \
        void vConfigureTickInterrupt(void);\
        vConfigureTickInterrupt(); \
    } while (0)


#define configCLEAR_TICK_INTERRUPT() \
    do \
    {   \
        void vClearTickInterrupt(void);\
        vClearTickInterrupt(); \
    }while (0)

/* The following constant describe the hardware, and are correct for the
QEMU-Virt. */
#define configINTERRUPT_CONTROLLER_BASE_ADDRESS (GICV3_DISTRIBUTOR_BASE_ADDR)
#define configINTERRUPT_CONTROLLER_CPU_INTERFACE_OFFSET (0x2000UL)
#define configUNIQUE_INTERRUPT_PRIORITIES 16

#if !defined(__ASSEMBLER__)
    void vPrintString(const char *pcString);
    void vPrintStringAndNumber(const char *pcString, uint32_t ulValue);
    void vPrintf(const char *format, ...);
#endif

#endif /* FREERTOS_CONFIG_H */
