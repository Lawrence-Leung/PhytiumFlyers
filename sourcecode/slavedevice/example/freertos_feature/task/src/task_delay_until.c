/*
This example demonstrates:
how to use delay until in tasks;
*/
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#define TASK_STACK_SIZE         1024

static xTaskHandle xtask1_handle;
static xTaskHandle xtask2_handle;

/* The task function. */
static void vTaskFunction(void *pvParameters);

/* Define the strings that will be passed in as the task parameters.  These are
defined const and off the stack to ensure they remain valid when the tasks are
executing. */
static const char *pcTextForTask1 = "Delay Until Task 1 is running\r\n";
static const char *pcTextForTask2 = "Delay Until Task 2 is running\r\n";

/*-----------------------------------------------------------*/


void CreateTasksForDelayUntilTest(void)
{
    /* Create the first task at priority 1... */
    xTaskCreate(vTaskFunction, "Delay Until Task 1", TASK_STACK_SIZE, (void *)pcTextForTask1, 1, &xtask1_handle);

    /* ... and the second task at priority 2.  The priority is the second to
    last parameter. */
    xTaskCreate(vTaskFunction, "Delay Until Task 2", TASK_STACK_SIZE, (void *)pcTextForTask2, 2, &xtask2_handle);
}

void DeleteTasksForDelayUntilTest(void)
{
    if (xtask1_handle)
    {
        vTaskDelete(xtask1_handle);
        vPrintString("Delay Until Task parameter 1 deletion \r\n");
    }

    if (xtask2_handle)
    {
        vTaskDelete(xtask2_handle);
        vPrintString("Delay Until Task parameter 2 deletion \r\n");
    }
}
/*-----------------------------------------------------------*/

static void vTaskFunction(void *pvParameters)
{
    char *pcTaskName;
    TickType_t xLastWakeTime;
    const TickType_t xDelay = pdMS_TO_TICKS(3000UL);

    /* The string to print out is passed in via the parameter.  Cast this to a
    character pointer. */
    pcTaskName = (char *) pvParameters;

    /* The xLastWakeTime variable needs to be initialized with the current tick
    count.  Note that this is the only time we access this variable.  From this
    point on xLastWakeTime is managed automatically by the vTaskDelayUntil()
    API function. */
    xLastWakeTime = xTaskGetTickCount();

    /* As per most tasks, this task is implemented in an infinite loop. */
    for (;;)
    {
        /* Print out the name of this task. */
        vPrintString(pcTaskName);

        /* We want this task to execute exactly every 1000 milliseconds.  As per
        the vTaskDelay() function, time is measured in ticks, and the
        pdMS_TO_TICKS() macro is used to convert this to milliseconds.
        xLastWakeTime is automatically updated within vTaskDelayUntil() so does not
        have to be updated by this task code. */
        vTaskDelayUntil(&xLastWakeTime, xDelay);
    }
}
