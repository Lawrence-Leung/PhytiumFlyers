/*
This example demonstrates how to:
uses xEventGroupSync() to synchronize three instances of a single task implementation.
The task parameter is used to pass into each instance the event bit the task will
set when it calls xEventGroupSync().
*/
#include <time.h>
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

static xTaskHandle xtask1_handle;
static xTaskHandle xtask2_handle;
static xTaskHandle xtask3_handle;

#define TASK_STACK_SIZE         1024

/* Definitions for the event bits in the event group. */
#define FIRST_TASK_BIT  ( 1UL << 0UL ) /* Event bit 0, which is set by the first task. */
#define SECOND_TASK_BIT ( 1UL << 1UL ) /* Event bit 1, which is set by the second task. */
#define THIRD_TASK_BIT  ( 1UL << 2UL ) /* Event bit 2, which is set by the third task. */

/* Pseudo random number generation functions - implemented in this file as the
MSVC rand() function has unexpected consequences. */
static uint32_t prvRand(void);
static void prvSRand(uint32_t ulSeed);

/* Three instances of this task are created. */
static void vSyncingTask(void *pvParameters);

/*-----------------------------------------------------------*/

/* Use by the pseudo random number generator. */
static uint32_t ulNextRand;

/* Declare the event group used to synchronize the three tasks. */
static EventGroupHandle_t xEventGroup;

void CreateSyncTasks(void)
{
    /* The tasks created in this example block for a random time.  The block
    time is generated using rand() - seed the random number generator. */
    prvSRand((uint32_t) time(NULL));

    /* Before an event group can be used it must first be created. */
    xEventGroup = xEventGroupCreate();

    /* Create three instances of the task.  Each task is given a different name,
    which is later printed out to give a visual indication of which task is
    executing.  The event bit to use when the task reaches its synchronization
    point is passed into the task using the task parameter. */
    xTaskCreate(vSyncingTask, "Sync Task 1", TASK_STACK_SIZE, (void *) FIRST_TASK_BIT, 1, &xtask1_handle);
    xTaskCreate(vSyncingTask, "Sync Task 2", TASK_STACK_SIZE, (void *) SECOND_TASK_BIT, 1, &xtask2_handle);
    xTaskCreate(vSyncingTask, "Sync Task 3", TASK_STACK_SIZE, (void *) THIRD_TASK_BIT, 1, &xtask3_handle);
}

void DeleteSyncTasks(void)
{
    if (xtask1_handle)
    {
        vTaskDelete(xtask1_handle);
        vPrintString("Eventgroup Sync Task 1 deletion \r\n");
    }

    if (xtask2_handle)
    {
        vTaskDelete(xtask2_handle);
        vPrintString("Eventgroup Sync Task 2 deletion \r\n");
    }

    if (xtask3_handle)
    {
        vTaskDelete(xtask3_handle);
        vPrintString("Eventgroup Sync Task 3 deletion \r\n");
    }

}
/*-----------------------------------------------------------*/

static void vSyncingTask(void *pvParameters)
{
    const EventBits_t uxAllSyncBits = (FIRST_TASK_BIT | SECOND_TASK_BIT | THIRD_TASK_BIT);
    const TickType_t xMaxDelay = pdMS_TO_TICKS(500UL);
    const TickType_t xMinDelay = pdMS_TO_TICKS(4000UL);
    TickType_t xDelayTime;
    EventBits_t uxThisTasksSyncBit;

    /* Three instances of this task are created - each task uses a different
    event bit in the synchronization.  The event bit to use by this task
    instance is passed into the task using the task's parameter.  Store it in
    the uxThisTasksSyncBit variable. */
    uxThisTasksSyncBit = (EventBits_t) pvParameters;

    for (;;)
    {
        /* Simulate this task taking some time to perform an action by delaying
        for a pseudo random time.  This prevents all three instances of this
        task from reaching the synchronization point at the same time, and
        allows the example's behavior to be observed more easily. */
        xDelayTime = (prvRand() % xMaxDelay) + xMinDelay;
        vTaskDelay(xDelayTime);

        /* Print out a message to show this task has reached its synchronization
        point.  pcTaskGetTaskName() is an API function that returns the name
        assigned to the task when the task was created. */
        vPrintString(pcTaskGetTaskName(NULL));
        vPrintString(" reached sync point\n");

        /* Wait for all the tasks to have reached their respective
        synchronization points. */
        xEventGroupSync( /* The event group used to synchronize. */
            xEventGroup,

            /* The bit set by this task to indicate it has reached
            the synchronization point. */
            uxThisTasksSyncBit,

            /* The bits to wait for, one bit for each task taking
            part in the synchronization. */
            uxAllSyncBits,

            /* Wait indefinitely for all three tasks to reach the
            synchronization point. */
            portMAX_DELAY);

        /* Print out a message to show this task has passed its synchronization
        point.  As an indefinite delay was used the following line will only be
        reached after all the tasks reached their respective synchronization
        points. */
        vPrintString(pcTaskGetTaskName(NULL));
        vPrintString(" exited sync point\n");

    }
}
/*-----------------------------------------------------------*/

static uint32_t prvRand(void)
{
    const uint32_t ulMultiplier = 0x015a4e35UL, ulIncrement = 1UL;
    uint32_t ulReturn;

    /* Utility function to generate a pseudo random number as the MSVC rand()
    function has unexpected consequences. */
    taskENTER_CRITICAL();
    ulNextRand = (ulMultiplier * ulNextRand) + ulIncrement;
    ulReturn = (ulNextRand >> 16UL) & 0x7fffUL;
    taskEXIT_CRITICAL();
    return ulReturn;
}
/*-----------------------------------------------------------*/

static void prvSRand(uint32_t ulSeed)
{
    /* Utility function to seed the pseudo random number generator. */
    ulNextRand = ulSeed;
}
/*-----------------------------------------------------------*/


