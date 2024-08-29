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

xSemaphoreHandle xCountingSemaphore;

static void vPeriodicTask(void *pvParameters)
{
    for (;;)
    {
        vTaskDelay(5000 / portTICK_RATE_MS);
        printf("Count Periodic task - About to generate an interrupt.\n");
        vTriggerInterrupt();
        printf("Count Periodic task - Interrupt generated.\n\n");
    }
}

static void vSemTakeTask(void *pvParameters)
{
    xSemaphoreTake(xCountingSemaphore, 0);

    for (;;)
    {
        xSemaphoreTake(xCountingSemaphore, portMAX_DELAY);
        printf("Count Handler task - Processing event, sem_count: %d\n", uxSemaphoreGetCount(xCountingSemaphore));
    }
}

static void vInterruptHandler(s32 vector, void *param)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    /* simulating multiple interrupts */
    xSemaphoreGiveFromISR(xCountingSemaphore, &xHigherPriorityTaskWoken);
    xSemaphoreGiveFromISR(xCountingSemaphore, &xHigherPriorityTaskWoken);
    xSemaphoreGiveFromISR(xCountingSemaphore, &xHigherPriorityTaskWoken);

    /* never call taskYIELD() form ISR! */
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);

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


void CreateCountSemTasks(void)
{
    xCountingSemaphore = xSemaphoreCreateCounting(10, 0);

    if (xCountingSemaphore != NULL)
    {
        prvSetupSoftwareInterrupt();

        xTaskCreate(vSemTakeTask,  "CountHandler",  TASK_STACK_SIZE, NULL, 3, &xtask1_handle);
        xTaskCreate(vPeriodicTask, "CountPeriodic", TASK_STACK_SIZE, NULL, 1, &xtask2_handle);
    }
}

void DeleteCountSemTasks(void)
{
    if (xtask1_handle)
    {
        vTaskDelete(xtask1_handle);
        printf("Count Handler deletion \r\n");
    }

    if (xtask2_handle)
    {
        vTaskDelete(xtask2_handle);
        printf("Count Periodic deletion \r\n");
    }

}
