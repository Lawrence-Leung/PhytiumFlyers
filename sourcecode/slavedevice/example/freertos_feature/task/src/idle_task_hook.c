#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#define TASK_STACK_SIZE         1024

static xTaskHandle xtask1_handle;
static xTaskHandle xtask2_handle;

/* The task function. */
static void vTaskFunction(void *pvParameters);

/* A variable that is incremented by the idle task hook function. */
static uint32_t ulIdleCycleCount = 0UL;

/* Define the strings that will be passed in as the task parameters.  These are
defined const and off the stack to ensure they remain valid when the tasks are
executing. */
static const char *pcTextForTask1 = "Idle Task 1 is running";
static const char *pcTextForTask2 = "Idle Task 2 is running";

/*-----------------------------------------------------------*/

void CreateTasksForIdleTask(void)
{
    BaseType_t ret;

    /* Create the first task at priority 1... */
    ret = xTaskCreate(vTaskFunction, "Idle Task 1",
                      TASK_STACK_SIZE, (void *)pcTextForTask1, 1, &xtask1_handle);
    if (ret != pdPASS)
    {
        xtask1_handle = NULL;
        vPrintStringAndNumber("Idle Task 1 create failed: ", ret);
    }
    /* ... and the second task at priority 2.  The priority is the second to
    last parameter. */
    ret = xTaskCreate(vTaskFunction, "Idle Task 2",
                      TASK_STACK_SIZE, (void *)pcTextForTask2, 2, &xtask2_handle);
    if (ret != pdPASS)
    {
        xtask1_handle = NULL;
        vPrintStringAndNumber("Idle Task 2 create failed: ", ret);
    }

}

void DeleteTasksForForIdleTask(void)
{
    if (xtask1_handle)
    {
        vTaskDelete(xtask1_handle);
        vPrintString("Idle Task 1 deletion \r\n");
    }

    if (xtask2_handle)
    {
        vTaskDelete(xtask2_handle);
        vPrintString("Idle Task 2 deletion \r\n");
    }
}

/*-----------------------------------------------------------*/

static void vTaskFunction(void *pvParameters)
{
    char *pcTaskName;
    const TickType_t xDelay = pdMS_TO_TICKS(5000UL);

    /* The string to print out is passed in via the parameter.  Cast this to a
    character pointer. */
    pcTaskName = (char *) pvParameters;

    /* As per most tasks, this task is implemented in an infinite loop. */
    for (;;)
    {
        /* Print out the name of this task AND the number of times ulIdleCycleCount
        has been incremented. */
        vPrintf("%s, count: %lu\n", pcTaskName, ulIdleCycleCount);

        /* Delay for a period.  This time we use a call to vTaskDelay() which
        puts the task into the Blocked state until the delay period has expired.
        The delay period is specified in 'ticks'. */
        vTaskDelay(xDelay);
    }
}
/*-----------------------------------------------------------*/

/* Idle hook functions MUST be called vApplicationIdleHook(), take no parameters,
and return void. */
void vApplicationIdleHook(void)
{
    /* This hook function does nothing but increment a counter. */
    ulIdleCycleCount++;
}
