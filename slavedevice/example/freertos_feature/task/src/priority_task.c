/*
This example demonstrates:
how to create and delete tasks use priority;
*/
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

/* Used as a loop counter to create a very crude delay. */
#define DELAY_LOOP_COUNT        ( 0x1fffffff )

#define TASK_STACK_SIZE         1024

static xTaskHandle xtask1_handle;
static xTaskHandle xtask2_handle;

/* The task function. */
static void vTaskFunction(void *pvParameters);

/* Define the strings that will be passed in as the task parameters.  These are
defined const and off the stack to ensure they remain valid when the tasks are
executing. */
static const char *pcTextForTask1 = "Priority Task 1 is running\r\n";
static const char *pcTextForTask2 = "Priority Task 2 is running\r\n";

/*-----------------------------------------------------------*/

void CreateTasksForPriorityTest(void)
{
    /* Create the first task at priority 1... */
    xTaskCreate(vTaskFunction, "Priority Task 1", TASK_STACK_SIZE, (void *)pcTextForTask1, 1, &xtask1_handle);

    /* ... and the second task at priority 2.  The priority is the second to
    last parameter. */
    xTaskCreate(vTaskFunction, "Priority Task 2", TASK_STACK_SIZE, (void *)pcTextForTask2, 2, &xtask2_handle);
}

void DeleteTasksForPriorityTest(void)
{
    if (xtask1_handle)
    {
        vTaskDelete(xtask1_handle);
        vPrintString("DeleteTasksForPriorityTest Task priority 1 deletion \r\n");
    }

    if (xtask2_handle)
    {
        vTaskDelete(xtask2_handle);
        vPrintString("DeleteTasksForPriorityTest Task priority 2 deletion \r\n");
    }

}
/*-----------------------------------------------------------*/

static void vTaskFunction(void *pvParameters)
{
    char *pcTaskName;
    volatile uint32_t ul;

    /* The string to print out is passed in via the parameter.  Cast this to a
    character pointer. */
    pcTaskName = (char *) pvParameters;

    /* As per most tasks, this task is implemented in an infinite loop. */
    for (;;)
    {
        /* Print out the name of this task. */
        vPrintString(pcTaskName);

        /* Delay for a period. */
        for (ul = 0; ul < DELAY_LOOP_COUNT; ul++)
        {
            /* This loop is just a very crude delay implementation.  There is
            nothing to do in here.  Later exercises will replace this crude
            loop with a proper delay/sleep function. */
        }
    }
}
