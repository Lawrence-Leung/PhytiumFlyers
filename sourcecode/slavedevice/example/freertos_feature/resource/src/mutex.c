/*
This example demonstrates:
Rewriting vPrintString() to use a mutex semaphore.
*/
#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "croutine.h"

#define TASK_STACK_SIZE         1024

static xTaskHandle xtask1_handle;
static xTaskHandle xtask2_handle;

static xSemaphoreHandle xMutex;

static void prvNewPrintString(const char *pcString)
{
    xSemaphoreTake(xMutex, portMAX_DELAY);
    printf("Mutex pcString = %s\n", pcString);
    xSemaphoreGive(xMutex);
}

static void prvPrintTask(void *pvParameters)
{
    char *pcStringToPrint;
    pcStringToPrint = (char *)pvParameters;

    for (;;)
    {
        prvNewPrintString(pcStringToPrint);

        /* Just delay with random time,
        Don't use rand() in secure applications. It's not reentrant!*/
        vTaskDelay(/*rand() & 0x3FF*/5000);
    }
}

void CreateResourceTasks(void)
{

    xMutex = xSemaphoreCreateMutex();

    if (xMutex != NULL)
    {

        xTaskCreate(prvPrintTask, "Mutex Print1", TASK_STACK_SIZE,
                    "Task 1 ******************************\n", 1, &xtask1_handle);
        xTaskCreate(prvPrintTask, "Mutex Print2", TASK_STACK_SIZE,
                    "Task 2 ==============================\n", 2, &xtask2_handle);
    }

}

void DeleteResourceTasks(void)
{
    if (xtask1_handle)
    {
        vTaskDelete(xtask1_handle);
        printf("Resource Task1 deletion \r\n");
    }

    if (xtask2_handle)
    {
        vTaskDelete(xtask2_handle);
        printf("Resource Task2 deletion \r\n");
    }

    vSemaphoreDelete(xMutex);
}

