/*
This example demonstrates:
how to change task priority;
*/
#include "FreeRTOS.h"
#include "task.h"

#define TASK_STACK_SIZE         1024

/* The two task functions. */
static void vTask1(void *pvParameters);
static void vTask2(void *pvParameters);

/* Used to hold the handle. */
TaskHandle_t xtask1_handle;
TaskHandle_t xtask2_handle;

/*-----------------------------------------------------------*/

void CreateTasksForChangePriorityTest(void)
{
    /* Create the first task at priority 2.  This time the task parameter is
    not used and is set to NULL.  The task handle is also not used so likewise
    is also set to NULL. */
    xTaskCreate(vTask1, "ChangePriority Task 1", TASK_STACK_SIZE, NULL, 2, &xtask1_handle);
    /* The task is created at priority 2 ^. */

    /* Create the second task at priority 1 - which is lower than the priority
    given to Task1.  Again the task parameter is not used so is set to NULL -
    BUT this time we want to obtain a handle to the task so pass in the address
    of the xtask2_handle variable. */
    xTaskCreate(vTask2, "ChangePriority Task 2", TASK_STACK_SIZE, NULL, 1, &xtask2_handle);
    /* The task handle is the last parameter ^^^^^^^^^^^^^ */

}

void DeleteTasksForChangePriorityTest(void)
{
    if (xtask1_handle)
    {
        vTaskDelete(xtask1_handle);
        vPrintString("ChangePriority Task1 deletion \r\n");
    }

    if (xtask2_handle)
    {
        vTaskDelete(xtask2_handle);
        vPrintString("ChangePriority Task2 deletion \r\n");
    }
}
/*-----------------------------------------------------------*/

void vTask1(void *pvParameters)
{
    UBaseType_t uxPriority;

    /* This task will always run before Task2 as it has the higher priority.
    Neither Task1 nor Task2 ever block so both will always be in either the
    Running or the Ready state.

    Query the priority at which this task is running - passing in NULL means
    "return our own priority". */
    uxPriority = uxTaskPriorityGet(NULL);

    for (;;)
    {
        /* Print out the name of this task. */
        vPrintf("ChangePriority Task1 is running, Task1 Priority=%d\r\n", uxPriority);

        /* Setting the Task2 priority above the Task1 priority will cause
        Task2 to immediately start running (as then Task2 will have the higher
        priority of the    two created tasks). */
        vPrintString("ChangePriority About to raise the Task2 priority\r\n");
        vTaskPrioritySet(xtask2_handle, (uxPriority + 1));

        /* Task1 will only run when it has a priority higher than Task2.
        Therefore, for this task to reach this point Task2 must already have
        executed and set its priority back down to 0. */
        vTaskDelay(3000);
    }
}

/*-----------------------------------------------------------*/

void vTask2(void *pvParameters)
{
    UBaseType_t uxPriority;

    /* Task1 will always run before this task as Task1 has the higher priority.
    Neither Task1 nor Task2 ever block so will always be in either the
    Running or the Ready state.

    Query the priority at which this task is running - passing in NULL means
    "return our own priority". */
    uxPriority = uxTaskPriorityGet(NULL);

    for (;;)
    {
        /* For this task to reach this point Task1 must have already run and
        set the priority of this task higher than its own.

        Print out the name of this task. */
        vPrintf("ChangePriority Task2 is running, Task2 Priority=%d\r\n", uxPriority);

        /* Set our priority back down to its original value.  Passing in NULL
        as the task handle means "change our own priority".  Setting the
        priority below that of Task1 will cause Task1 to immediately start
        running again. */
        vPrintString("ChangePriority About to lower the Task2 priority\r\n");
        vTaskPrioritySet(NULL, (uxPriority - 2));
        vTaskDelay(3000);
    }
}
/*-----------------------------------------------------------*/




