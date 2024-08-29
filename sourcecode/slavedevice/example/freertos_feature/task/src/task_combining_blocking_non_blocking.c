#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#define TASK_STACK_SIZE         1024

/* The task functions. */
static void vContinuousProcessingTask(void *pvParameters);
static void vPeriodicTask(void *pvParameters);

static xTaskHandle xtask1_handle;
static xTaskHandle xtask2_handle;
static xTaskHandle xtask3_handle;

/* Define the strings that will be passed in as the task parameters.  These are
defined const and off the stack to ensure they remain valid when the tasks are
executing. */
static const char *pcTextForTask1 = "BlockingOrNone Continuous task 1 running\r\n";
static const char *pcTextForTask2 = "BlockingOrNone Continuous task 2 running\r\n";
static const char *pcTextForPeriodicTask = "BlockingOrNone Periodic task is running\r\n";

/*-----------------------------------------------------------*/
void CreateTasksForBlockingOrNone(void)
{
    /* Create two instances of the continuous processing task, both at priority 1. */
    xTaskCreate(vContinuousProcessingTask, "BlockingOrNone Task 1",
                TASK_STACK_SIZE, (void *)pcTextForTask1, 1, &xtask1_handle);

    xTaskCreate(vContinuousProcessingTask, "BlockingOrNone Task 2",
                TASK_STACK_SIZE, (void *)pcTextForTask2, 1, &xtask2_handle);

    /* Create one instance of the periodic task at priority 2. */
    xTaskCreate(vPeriodicTask, "BlockingOrNone Task 3",
                TASK_STACK_SIZE, (void *)pcTextForPeriodicTask, 2, &xtask3_handle);
}

void DeleteTasksForBlockingOrNone(void)
{
    if (xtask1_handle)
    {
        vTaskDelete(xtask1_handle);
        vPrintString("BlockingOrNone Task 1 deletion \r\n");
    }

    if (xtask2_handle)
    {
        vTaskDelete(xtask2_handle);
        vPrintString("BlockingOrNone Task 2 deletion \r\n");
    }

    if (xtask3_handle)
    {
        vTaskDelete(xtask3_handle);
        vPrintString("BlockingOrNone Task 3 deletion \r\n");
    }
}
/*-----------------------------------------------------------*/

void vContinuousProcessingTask(void *pvParameters)
{
    char *pcTaskName;
    /* The string to print out is passed in via the parameter.  Cast this to a
    character pointer. */
    pcTaskName = (char *) pvParameters;

    /* As per most tasks, this task is implemented in an infinite loop. */
    for (;;)
    {
        /* Print out the name of this task.  This task just does this repeatedly
        without ever blocking or delaying. */
        vPrintString(pcTaskName);
        vTaskDelay(5000);

    }
}
/*-----------------------------------------------------------*/

void vPeriodicTask(void *pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xDelay = pdMS_TO_TICKS(5000UL);

    /* The xLastWakeTime variable needs to be initialized with the current tick
    count.  Note that this is the only time we access this variable.  From this
    point on xLastWakeTime is managed automatically by the vTaskDelayUntil()
    API function. */
    xLastWakeTime = xTaskGetTickCount();

    /* As per most tasks, this task is implemented in an infinite loop. */
    for (;;)
    {
        /* Print out the name of this task. */
        vPrintString("Periodic task is running\r\n");

        /* We want this task to execute exactly every 100 milliseconds. */
        vTaskDelayUntil(&xLastWakeTime, xDelay);
    }
}