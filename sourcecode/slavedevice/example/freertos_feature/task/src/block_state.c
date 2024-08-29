/*
This example demonstrates:
how to make task block state;
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
static const char *pcTextForTask1 = "Block Task 1 is running\r\n";
static const char *pcTextForTask2 = "Block Task 2 is running\r\n";

/*-----------------------------------------------------------*/

void CreateTasksForBlockTest(void)
{
    /* Create the first task at priority 1... */
    xTaskCreate(vTaskFunction, "Block Task 1", TASK_STACK_SIZE, (void *)pcTextForTask1, 1, &xtask1_handle);

    /* ... and the second task at priority 2.  The priority is the second to
    last parameter. */
    xTaskCreate(vTaskFunction, "Block Task 2", TASK_STACK_SIZE, (void *)pcTextForTask2, 2, &xtask2_handle);


}


void DeleteTasksForBlockTest(void)
{
    if (xtask1_handle)
    {
        vTaskDelete(xtask1_handle);
        vPrintString("DeleteTasksForBlockTest Task parameter 1 deletion \r\n");
    }

    if (xtask2_handle)
    {
        vTaskDelete(xtask2_handle);
        vPrintString("DeleteTasksForBlockTest Task parameter 2 deletion \r\n");
    }
}

/*-----------------------------------------------------------*/

static void vTaskFunction(void *pvParameters)
{
    char *pcTaskName;
    const TickType_t xDelay = pdMS_TO_TICKS(3000UL);

    /* The string to print out is passed in via the parameter.  Cast this to a
    character pointer. */
    pcTaskName = (char *) pvParameters;

    /* As per most tasks, this task is implemented in an infinite loop. */
    for (;;)
    {
        /* Print out the name of this task. */
        vPrintString(pcTaskName);

        /* Delay for a period.  This time a call to vTaskDelay() is used which
        places the task into the Blocked state until the delay period has
        expired.  The parameter takes a time specified in 'ticks', and the
        pdMS_TO_TICKS() macro is used (where the xDelay constant is
        declared) to convert 3000 milliseconds into an equivalent time in
        ticks. */
        vTaskDelay(xDelay);
    }
}
