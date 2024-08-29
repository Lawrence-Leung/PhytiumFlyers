#include <stdio.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
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

xSemaphoreHandle xBinarySemaphore;

void vPeriodicTask(void *pvParameters)
{
    for (;;)
    {
        vTaskDelay(5000 / portTICK_RATE_MS);
        printf("Bin Periodic task - Generate an interrupt.\n");
        vTriggerInterrupt();
    }
}

void vSemTakeTask(void *pvParameters)
{
    xSemaphoreTake(xBinarySemaphore, 0);
    for (;;)
    {
        xSemaphoreTake(xBinarySemaphore, portMAX_DELAY);
        printf("Bin Handler task - Processing event.\n");
    }
}

static void vInterruptHandler(s32 vector, void *param)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(xBinarySemaphore, &xHigherPriorityTaskWoken);

    /* never call taskYIELD() form ISR! */
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

static void prvSetupSoftwareInterrupt()
{
    GetCpuId(&cpu_id);
    vPrintf("cpu_id is %d \r\n", cpu_id);

    /* The interrupt service routine uses an (interrupt safe) FreeRTOS API
    function so the interrupt priority must be at or below the priority defined
    by configSYSCALL_INTERRUPT_PRIORITY. */
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


void CreateBinarySemTasks(void)
{
    vSemaphoreCreateBinary(xBinarySemaphore);

    if (xBinarySemaphore != NULL)
    {
        prvSetupSoftwareInterrupt();
        xTaskCreate(vSemTakeTask,  "BinHandler",  TASK_STACK_SIZE, NULL, 3, &xtask1_handle);
        xTaskCreate(vPeriodicTask, "BinPeriodic", TASK_STACK_SIZE, NULL, 1, &xtask2_handle);
    }

}

void DeleteBinarySemTasks(void)
{
    if (xtask1_handle)
    {
        vTaskDelete(xtask1_handle);
        printf("Bin Handler deletion \r\n");
    }

    if (xtask2_handle)
    {
        vTaskDelete(xtask2_handle);
        printf("Bin Periodic deletion \r\n");
    }

}