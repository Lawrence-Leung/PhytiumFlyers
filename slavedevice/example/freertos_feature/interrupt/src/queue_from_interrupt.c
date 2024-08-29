#include <stdio.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "croutine.h"
#include "finterrupt.h"
#include "fcpu_info.h"

static xTaskHandle xtask1_handle;
static xTaskHandle xtask2_handle;

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

static u32 cpu_id = 0;

static xQueueHandle xIntegerQueue;
static xQueueHandle xStringQueue;

static void vIntegerGenerator(void *pvParameters)
{
    portTickType xLastExecutionTime;
    unsigned long ulValueToSend = 0;
    int i;

    xLastExecutionTime = xTaskGetTickCount();

    for (;;)
    {
        vTaskDelayUntil(&xLastExecutionTime, 5000 / portTICK_RATE_MS);

        for (i = 0; i < 5; ++i)
        {
            xQueueSendToBack(xIntegerQueue, &ulValueToSend, 0);
            ++ulValueToSend;
        }

        vPrintf("Queue Periodic task - About to generate an interrupt.\n");
        vTriggerInterrupt();
        vPrintf("Queue Periodic task - Interrupt generated.\n\n");
    }
}

static void vInterruptHandler(s32 vector, void *param)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    static unsigned long ulReceivedNumber;

    static const char *pcStrings[] =
    {
        "String 0\n",
        "String 1\n",
        "String 2\n",
        "String 3\n"
    };

    while (xQueueReceiveFromISR(xIntegerQueue, &ulReceivedNumber,
                                &xHigherPriorityTaskWoken) != errQUEUE_EMPTY)
    {

        // last 2 bits: values 0-3
        ulReceivedNumber &= 0x03;
        xQueueSendToBackFromISR(xStringQueue, &pcStrings[ulReceivedNumber],
                                &xHigherPriorityTaskWoken);
    }

    // never call taskYIELD() form ISR!
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

static void vStringPrinter(void *pvParameters)
{
    char *pcString;

    for (;;)
    {
        xQueueReceive(xStringQueue, &pcString, portMAX_DELAY);
        vPrintf("pcString = %s\n", pcString);
    }
}

static void prvSetupSoftwareInterrupt(void)
{
    GetCpuId(&cpu_id);
    vPrintf("cpu_id is %d \r\n", cpu_id);

    InterruptSetPriority(INTERRUPT_ID, INTERRUPT_PRIORITY);

    InterruptInstall(INTERRUPT_ID, vInterruptHandler, NULL, NULL);

    /* Enable the interrupt. */
    InterruptUmask(INTERRUPT_ID);
}

/* Macro to force an interrupt. */
static void vTriggerInterrupt(void)
{
    InterruptCoreInterSend(INTERRUPT_ID, (1 << cpu_id));
}

void CreateQueueTasks(void)
{
    xIntegerQueue = xQueueCreate(10, sizeof(unsigned long));
    xStringQueue = xQueueCreate(10, sizeof(char *));

    prvSetupSoftwareInterrupt();

    xTaskCreate(vIntegerGenerator, "QueueIntGen", TASK_STACK_SIZE, NULL, 1, &xtask1_handle);
    xTaskCreate(vStringPrinter,    "QueueString", TASK_STACK_SIZE, NULL, 2, &xtask2_handle);

}

void DeleteQueueTasks(void)
{
    if (xtask1_handle)
    {
        vTaskDelete(xtask1_handle);
        printf("Queue IntGen deletion \r\n");
    }

    if (xtask2_handle)
    {
        vTaskDelete(xtask2_handle);
        printf("Queue String deletion \r\n");
    }
}