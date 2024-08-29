/*
This example demonstrates:
how to create and delete tasks;
*/
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#define DELAY_LOOP_COUNT        ( 0x1fffffff )
#define TASK_STACK_SIZE         1024

static xTaskHandle xtask1_handle;
static xTaskHandle xtask2_handle;

/* The task functions. */
static void vTask1(void *pvParameters);
static void vTask2(void *pvParameters);

/*-----------------------------------------------------------*/

void CreateTasks(void)
{
    /* Create one of the two tasks. */
    xTaskCreate(vTask1,      /* Pointer to the function that implements the task. */
                "Create Task 1",    /* Text name for the task.  This is to facilitate debugging only. */
                TASK_STACK_SIZE,        /* Stack depth - most small microcontrollers will use much less stack than this. */
                NULL,       /* We are not using the task parameter. */
                1,          /* This task will run at priority 1. */
                &xtask1_handle);        /* We are using the task handle to delete this task. */

    /* Create the other task in exactly the same way. */
    xTaskCreate(vTask2, "Create Task 2", TASK_STACK_SIZE, NULL, 1, &xtask2_handle);

}

void DeleteTasks(void)
{
    if (xtask1_handle)
    {
        vTaskDelete(xtask1_handle);
        vPrintString("DeleteTasks Task1 deletion \r\n");
    }

    if (xtask2_handle)
    {
        vTaskDelete(xtask2_handle);
        vPrintString("DeleteTasks Task2 deletion \r\n");
    }
}

/*-----------------------------------------------------------*/

static void vTask1(void *pvParameters)
{
    const char *pcTaskName = "Create Task 1 is running\r\n";
    volatile uint32_t ul;

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
/*-----------------------------------------------------------*/

static void vTask2(void *pvParameters)
{
    const char *pcTaskName = "Create Task 2 is running\r\n";
    volatile uint32_t ul;

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