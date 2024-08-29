/*
This example demonstrates:
uses task notification to unblock a task from within an interrupt
service routine—effectively synchronizing the task with the interrupt.
the ulTaskNotifyTake() xClearOnExit parameter was set to pdTRUE.
*/

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "finterrupt.h"
#include "fcpu_info.h"

static xTaskHandle xtask1_handle = NULL;
static xTaskHandle xtask2_handle = NULL;

#define TASK_STACK_SIZE         1024

/* The interrupt number to use for the software interrupt generation.  This
could be any unused number.  In this case the first chip level (non system)
interrupt is used */
#define INTERRUPT_ID        0

/* The priority of the software interrupt.  The interrupt service routine uses
an (interrupt safe) FreeRTOS API function, so the priority of the interrupt must
be equal to or lower than the priority set by
configMAX_SYSCALL_INTERRUPT_PRIORITY - remembering that on the Cortex M3 high
numeric values represent low priority values, which can be confusing as it is
counter intuitive. */
#define INTERRUPT_PRIORITY  IRQ_PRIORITY_VALUE_12

/* Macro to force an interrupt. */
static void vTriggerInterrupt(void);

/* The tasks to be created. */
static void vHandlerTask(void *pvParameters);
static void vPeriodicTask(void *pvParameters);

/* The service routine for the (simulated) interrupt.  This is the interrupt
that sets an event bit in the event group. */
static void vSetupSoftwareInterrupt(void);

/* The rate at which the periodic task generates software interrupts. */
static const TickType_t xInterruptFrequency = pdMS_TO_TICKS(5000UL);

static u32 cpu_id = 0;

/*-----------------------------------------------------------*/

void CreateNotifyTakeTrueTasks(void)
{
    /* Install the handler for the software interrupt.  The syntax necessary
    to do this is dependent on the FreeRTOS port being used.  The syntax
    shown here can only be used with the FreeRTOS Windows port, where such
    interrupts are only simulated. */
    vSetupSoftwareInterrupt();

    /* Create the 'handler' task, which is the task to which interrupt
    processing is deferred, and so is the task that will be synchronized
    with the interrupt.  The handler task is created with a high priority to
    ensure it runs immediately after the interrupt exits.  In this case a
    priority of 3 is chosen.  The handle of the task is saved for use by the
    ISR. */
    xTaskCreate(vHandlerTask, "Notify True Handler", TASK_STACK_SIZE, NULL, 3, &xtask1_handle);

    /* Create the task that will periodically generate a software interrupt.
    This is created with a priority below the handler task to ensure it will
    get preempted each time the handler task exits the Blocked state. */
    xTaskCreate(vPeriodicTask, "Notify True Periodic", TASK_STACK_SIZE, NULL, 1, &xtask2_handle);

}

void DeleteNotifyTakeTrueTasks(void)
{
    if (xtask1_handle)
    {
        vTaskDelete(xtask1_handle);
        vPrintString("Task notify True Handler deletion \r\n");
    }

    if (xtask2_handle)
    {
        vTaskDelete(xtask2_handle);
        vPrintString("Task notify True Periodic deletion \r\n");
    }
}
/*-----------------------------------------------------------*/

static void vHandlerTask(void *pvParameters)
{
    /* xMaxExpectedBlockTime is set to be a little longer than the maximum expected
    time between events. */
    const TickType_t xMaxExpectedBlockTime = xInterruptFrequency + pdMS_TO_TICKS(10);
    uint32_t ulEventsToProcess;

    /* As per most tasks, this task is implemented within an infinite loop. */
    for (;;)
    {
        /* Wait to receive a notification sent directly to this task from the
        interrupt handler. */
        ulEventsToProcess = ulTaskNotifyTake(pdTRUE, xMaxExpectedBlockTime);
        if (ulEventsToProcess != 0)
        {
            /* To get here at least one event must have occurred.  Loop here
            until all the pending events have been processed (in this case, just
            print out a message for each event). */
            while (ulEventsToProcess > 0)
            {
                vPrintString("Task notify True Handler task - Processing event.");
                ulEventsToProcess--;
            }
        }
        else
        {
            /* If this part of the function is reached then an interrupt did not
            arrive within the expected time, and (in a real application) it may
            be necessary to perform some error recovery operations. */
        }
    }
}
/*-----------------------------------------------------------*/

static void ulExampleInterruptHandler(s32 vector, void *param)
{
    BaseType_t xHigherPriorityTaskWoken;

    /* The xHigherPriorityTaskWoken parameter must be initialized to pdFALSE as
    it will get set to pdTRUE inside the interrupt safe API function if a
    context switch is required. */
    xHigherPriorityTaskWoken = pdFALSE;

    /* Send a notification directly to the handler task. */
    vTaskNotifyGiveFromISR(/* The handle of the task to which the notification
                            is being sent.  The handle was saved when the task
                            was created. */
        xtask1_handle,

        /* xHigherPriorityTaskWoken is used in the usual
        way. */
        &xHigherPriorityTaskWoken);

    /* Pass the xHigherPriorityTaskWoken value into portYIELD_FROM_ISR().  If
    xHigherPriorityTaskWoken was set to pdTRUE inside vTaskNotifyGiveFromISR()
    then calling portYIELD_FROM_ISR() will request a context switch.  If
    xHigherPriorityTaskWoken is still pdFALSE then calling
    portYIELD_FROM_ISR() will have no effect.  The implementation of
    portYIELD_FROM_ISR() used by the Windows port includes a return statement,
    which is why this function does not explicitly return a value. */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
/*-----------------------------------------------------------*/

static void vPeriodicTask(void *pvParameters)
{
    /* As per most tasks, this task is implemented within an infinite loop. */
    for (;;)
    {
        /* This task is just used to 'simulate' an interrupt.  This is done by
        periodically generating a simulated software interrupt.  Block until it
        is time to generate the software interrupt again. */
        vTaskDelay(xInterruptFrequency);

        /* Generate the interrupt, printing a message both before and after
        the interrupt has been generated so the sequence of execution is evident
        from the output.

        The syntax used to generate a software interrupt is dependent on the
        FreeRTOS port being used.  The syntax used below can only be used with
        the FreeRTOS Windows port, in which such interrupts are only
        simulated. */
        vPrintString("Task Notify True Periodic task - About to generate an interrupt.");
        vTriggerInterrupt();
        vPrintString("Task Notify True Periodic task - Interrupt generated.");
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

    InterruptInstall(INTERRUPT_ID, ulExampleInterruptHandler, NULL, NULL);

    /* Enable the interrupt. */
    InterruptUmask(INTERRUPT_ID);
}

/* Macro to force an interrupt. */
static void vTriggerInterrupt(void)
{
    InterruptCoreInterSend(INTERRUPT_ID, (1 << cpu_id));
}






