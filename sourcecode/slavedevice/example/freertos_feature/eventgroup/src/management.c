/*
This example demonstrates how to:
    Create an event group.
    Set bits in an event group from an interrupt service routine.
    Set bits in an event group from a task.
    Block on an event group.
*/
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "timers.h" /* For the xTimerPendFunctionCallFromISR() function. */
#include "finterrupt.h"
#include "fcpu_info.h"

static xTaskHandle xtask1_handle;
static xTaskHandle xtask2_handle;
static xTaskHandle xtask3_handle;

#define TASK_STACK_SIZE         1024

/* The interrupt number to use for the software interrupt generation.  This
could be any unused number.  In this case the first chip level (non system)
interrupt is used */
#define INTERRUPT_ID        0

/* The priority of the software interrupt. The interrupt service routine uses
an (interrupt safe) FreeRTOS API function, so the priority of the interrupt must
be equal to or lower than the priority set by configMAX_SYSCALL_INTERRUPT_PRIORITY. */
#define INTERRUPT_PRIORITY  IRQ_PRIORITY_VALUE_12

/* Definitions for the event bits in the event group. */
#define FIRST_TASK_BIT  ( 1UL << 0UL ) /* Event bit 0, which is set by a task. */
#define SECOND_TASK_BIT ( 1UL << 1UL ) /* Event bit 1, which is set by a task. */
#define ISR_BIT         ( 1UL << 2UL ) /* Event bit 2, which is set by an ISR. */

/* The tasks to be created. */
static void vIntegerGenerator(void *pvParameters);
static void vEventBitSettingTask(void *pvParameters);
static void vEventBitReadingTask(void *pvParameters);

/* A function that can be deferred to run in the RTOS daemon task.  The function
prints out the string passed to it using the pvParameter1 parameter. */
void vPrintStringFromDaemonTask(void *pvParameter1, uint32_t ulParameter2);

/* The service routine for the (simulated) interrupt.  This is the interrupt
that sets an event bit in the event group. */
static void vSetupSoftwareInterrupt(void);

/* Macro to force an interrupt. */
static void vTriggerInterrupt(void);

static u32 cpu_id = 0;

/*-----------------------------------------------------------*/

/* Declare the event group in which bits are set from both a task and an ISR. */
static EventGroupHandle_t xEventGroup;

void CreateManagementTasks(void)
{
    /* Before an event group can be used it must first be created. */
    xEventGroup = xEventGroupCreate();
    vSetupSoftwareInterrupt();

    /* Create the task that sets event bits in the event group. */
    xTaskCreate(vEventBitSettingTask, "Manage BitSetter", TASK_STACK_SIZE, NULL, 1, &xtask1_handle);

    /* Create the task that waits for event bits to get set in the event
    group. */
    xTaskCreate(vEventBitReadingTask, "Manage BitReader", TASK_STACK_SIZE, NULL, 2, &xtask2_handle);

    /* Create the task that is used to periodically generate a software
    interrupt. */
    xTaskCreate(vIntegerGenerator, "Manage IntGen", TASK_STACK_SIZE, NULL, 3, &xtask3_handle);

    /* Install the handler for the software interrupt.  The syntax necessary
    to do this is dependent on the FreeRTOS port being used.  */
}

void DeleteManagementTasks(void)
{
    if (xtask1_handle)
    {
        vTaskDelete(xtask1_handle);
        vPrintString("Eventgroup management BitSetter deletion");
    }

    if (xtask2_handle)
    {
        vTaskDelete(xtask2_handle);
        vPrintString("Eventgroup management BitReader deletion");
    }

    if (xtask3_handle)
    {
        vTaskDelete(xtask3_handle);
        vPrintString("Eventgroup management IntGen deletion");
    }

}
/*-----------------------------------------------------------*/

static void vEventBitSettingTask(void *pvParameters)
{
    const TickType_t xDelay = pdMS_TO_TICKS(5000UL), xDontBlock = 0;

    for (;;)
    {
        /* Delay for a short while before starting the next loop. */
        vTaskDelay(xDelay);

        /* Print out a message to say event bit 0 is about to be set by the
        task, then set event bit 0. */
        vPrintString("Manage Bit setting task -\t about to set bit 0.");
        xEventGroupSetBits(xEventGroup, FIRST_TASK_BIT);

        /* Delay for a short while before setting the other bit set within this
        task. */
        vTaskDelay(xDelay);

        /* Print out a message to say event bit 1 is about to be set by the
        task, then set event bit 1. */
        vPrintString("Manage Bit setting task -\t about to set bit 1.");
        xEventGroupSetBits(xEventGroup, SECOND_TASK_BIT);
    }
}
/*-----------------------------------------------------------*/

static void ulEventBitSettingISR(s32 vector, void *param)
{
    BaseType_t xHigherPriorityTaskWoken;
    /* The string is not printed within the interrupt service, but is instead
    sent to the RTOS daemon task for printing.  It is therefore declared static to
    ensure the compiler does not allocate the string on the stack of the ISR (as the
    ISR's stack frame will not exist when the string is printed from the daemon
    task. */
    static const char *pcString = "Manage Bit setting ISR -\t about to set bit 2.";

    /* As always, xHigherPriorityTaskWoken is initialized to pdFALSE. */
    xHigherPriorityTaskWoken = pdFALSE;

    /* Print out a message to say bit 2 is about to be set.  Messages cannot be
    printed from an ISR, so defer the actual output to the RTOS daemon task by
    pending a function call to run in the context of the RTOS daemon task. */
    xTimerPendFunctionCallFromISR(vPrintStringFromDaemonTask, (void *) pcString, 0, &xHigherPriorityTaskWoken);

    /* Set bit 2 in the event group. */
    xEventGroupSetBitsFromISR(xEventGroup, ISR_BIT, &xHigherPriorityTaskWoken);

    /* xEventGroupSetBitsFromISR() writes to the timer command queue.  If
    writing to the timer command queue results in the RTOS daemon task leaving
    the Blocked state, and if the priority of the RTOS daemon task is higher
    than the priority of the currently executing task (the task this interrupt
    interrupted) then xHigherPriorityTaskWoken will have been set to pdTRUE
    inside xEventGroupSetBitsFromISR().

    xHigherPriorityTaskWoken is used as the parameter to portYIELD_FROM_ISR().
    If xHigherPriorityTaskWoken equals pdTRUE then calling portYIELD_FROM_ISR()
    will request a context switch.  If xHigherPriorityTaskWoken is still pdFALSE
    then calling portYIELD_FROM_ISR() will have no effect.

    The implementation of portYIELD_FROM_ISR() used by the Windows port includes
    a return statement, which is why this function does not explicitly return a
    value. */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
/*-----------------------------------------------------------*/

static void vEventBitReadingTask(void *pvParameters)
{
    const EventBits_t xBitsToWaitFor = (FIRST_TASK_BIT | SECOND_TASK_BIT | ISR_BIT);
    EventBits_t xEventGroupValue;

    for (;;)
    {
        /* Block to wait for event bits to become set within the event group. */
        xEventGroupValue = xEventGroupWaitBits( /* The event group to read. */
                               xEventGroup,

                               /* Bits to test. */
                               xBitsToWaitFor,

                               /* Clear bits on exit if the
                               unblock condition is met. */
                               pdTRUE,

                               /* Don't wait for all bits. */
                               pdFALSE,

                               /* Don't time out. */
                               portMAX_DELAY);

        /* Print a message for each bit that was set. */
        if ((xEventGroupValue & FIRST_TASK_BIT) != 0)
        {
            vPrintString("Manage Bit reading task -\t event bit 0 was set");
        }

        if ((xEventGroupValue & SECOND_TASK_BIT) != 0)
        {
            vPrintString("Manage Bit reading task -\t event bit 1 was set");
        }

        if ((xEventGroupValue & ISR_BIT) != 0)
        {
            vPrintString("Manage Bit reading task -\t event bit 2 was set");
        }
    }
}
/*-----------------------------------------------------------*/

void vPrintStringFromDaemonTask(void *pvParameter1, uint32_t ulParameter2)
{
    /* The string to print is passed into this function using the pvParameter1
    parameter. */
    vPrintString((const char *) pvParameter1);
}
/*-----------------------------------------------------------*/

static void vIntegerGenerator(void *pvParameters)
{
    TickType_t xLastExecutionTime;
    const TickType_t xDelay = pdMS_TO_TICKS(5000UL);

    /* Initialize the variable used by the call to vTaskDelayUntil(). */
    xLastExecutionTime = xTaskGetTickCount();

    for (;;)
    {
        /* This is a periodic task.  Block until it is time to run again.
        The task will execute every 500ms. */
        vTaskDelayUntil(&xLastExecutionTime, xDelay);

        /* Generate the interrupt that will set a bit in the event group. */
        vTriggerInterrupt();
    }
}

static void vSetupSoftwareInterrupt(void)
{

    GetCpuId(&cpu_id);
    vPrintf("cpu_id is %d \r\n", cpu_id);

    /* The interrupt service routine uses an (interrupt safe) FreeRTOS API
    function so the interrupt priority must be at or below the priority defined
    by configSYSCALL_INTERRUPT_PRIORITY. */
    InterruptSetPriority(INTERRUPT_ID, INTERRUPT_PRIORITY);

    InterruptInstall(INTERRUPT_ID, ulEventBitSettingISR, NULL, NULL);

    /* Enable the interrupt. */
    InterruptUmask(INTERRUPT_ID);
}

/* Macro to force an interrupt. */
static void vTriggerInterrupt(void)
{
    InterruptCoreInterSend(INTERRUPT_ID, (1 << cpu_id));
}
