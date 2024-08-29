/*
This example demonstrates:
how to create and delete tasks use parameters;
*/

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

/* Used as a loop counter to create a very crude delay. */
#define DELAY_LOOP_COUNT        ( 0x1fffffff )

/* task stack size set. */
#define TASK_STACK_SIZE         1024

static xTaskHandle xtask1_handle;
static xTaskHandle xtask2_handle;


/* The task function. */
static void vTaskFunction(void *pvParameters);

/* Define the strings that will be passed in as the task parameters.  These are
defined const and off the stack to ensure they remain valid when the tasks are
executing. */
const char *pcTextForTask1 = "Parameter Task 1 is running\r\n";
const char *pcTextForTask2 = "Parameter Task 2 is running\r\n";

/*-----------------------------------------------------------*/

void CreateTasksForParamterTest(void)
{
    /* Create one of the two tasks. */
    xTaskCreate(vTaskFunction,           /* Pointer to the function that implements the task. */
                "Parameter Task 1",             /* Text name for the task.  This is to facilitate debugging only. */
                TASK_STACK_SIZE,                    /* Stack depth - most small microcontrollers will use much less stack than this. */
                (void *)pcTextForTask1, /* Pass the text to be printed in as the task parameter. */
                1,                      /* This task will run at priority 1. */
                &xtask1_handle);                    /* We are not using the task handle. */

    /* Create the other task in exactly the same way.  Note this time that we
    are creating the SAME task, but passing in a different parameter.  We are
    creating two instances of a single task implementation. */
    xTaskCreate(vTaskFunction, "Parameter Task 2", TASK_STACK_SIZE, (void *)pcTextForTask2, 1, &xtask2_handle);

}

void DeleteTasksForParamterTest(void)
{
    if (xtask1_handle)
    {
        vTaskDelete(xtask1_handle);
        vPrintString("DeleteTasksForParamterTest Task parameter 1 deletion \r\n");
    }

    if (xtask2_handle)
    {
        vTaskDelete(xtask2_handle);
        vPrintString("DeleteTasksForParamterTest Task parameter 2 deletion \r\n");
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