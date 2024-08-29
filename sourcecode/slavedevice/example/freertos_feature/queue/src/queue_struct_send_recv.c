/*
This example is similar to queue_int_send_recv, but the task priorities are reversed, so the receiving task
has a lower priority than the sending tasks. Also, the queue is used to pass structures, rather than integers.
*/
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define TASK_STACK_SIZE         2048

#define mainSENDER_1 0
#define mainSENDER_2 1

static xTaskHandle xtask1_handle;
static xTaskHandle xtask2_handle;
static xTaskHandle xtask3_handle;

typedef struct
{
    unsigned char ucValue;
    unsigned char ucSource;
} xData;

static const xData xStructsToSend[2] =
{
    {1, mainSENDER_1},/* Used by Sender1. */
    {2, mainSENDER_2} /* Used by Sender2. */
};

/* Declare a variable of type QueueHandle_t.  This is used to store the queue
that is accessed by all three tasks. */
static QueueHandle_t xQueue;

static void vSenderTask(void *pvParameters)
{
    portBASE_TYPE xStatus;
    const portTickType xTicksToWait = 100 / portTICK_RATE_MS;

    /* As per most tasks, this task is implemented within an infinite loop. */
    for (;;)
    {
        /* The first parameter is the queue to which data is being sent.  The
        queue was created before the scheduler was started, so before this task
        started to execute.

        The second parameter is the address of the structure being sent.  The
        address is passed in as the task parameter.

        The third parameter is the Block time - the time the task should be kept
        in the Blocked state to wait for space to become available on the queue
        should the queue already be full.  A block time is specified as the queue
        will become full.  Items will only be removed from the queue when both
        sending tasks are in the Blocked state.. */
        xStatus = xQueueSendToBack(xQueue, pvParameters, xTicksToWait);

        /* We could not write to the queue because it was full - this must
            be an error as the receiving task should make space in the queue
            as soon as both sending tasks are in the Blocked state. */
        if (xStatus != pdPASS)
        {
            vPrintString("Could not send to the queue!\n");
        }

        vTaskDelay(5000);
    }
}

static void vReceiverTask(void *pvParameters)
{
    /* Declare the structure that will hold the values received from the queue. */
    xData xReceivedStructure;
    portBASE_TYPE xStatus;

    /* This task is also defined within an infinite loop. */
    for (;;)
    {
        /* As this task only runs when the sending tasks are in the Blocked state,
        and the sending tasks only block when the queue is full, this task should
        always find the queue to be full.  3 is the queue length. */
        if (uxQueueMessagesWaiting(xQueue) == 3)
        {
            vPrintString("Queue should is full!\n");
        }

        /* The first parameter is the queue from which data is to be received.  The
        queue is created before the scheduler is started, and therefore before this
        task runs for the first time.

        The second parameter is the buffer into which the received data will be
        placed.  In this case the buffer is simply the address of a variable that
        has the required size to hold the received structure.

        The last parameter is the block time - the maximum amount of time that the
        task should remain in the Blocked state to wait for data to be available
        should the queue already be empty.  A block time is not necessary as this
        task will only run when the queue is full so data will always be available. */
        xStatus = xQueueReceive(xQueue, &xReceivedStructure, /*0*/portMAX_DELAY);

        /* Data was successfully received from the queue, print out the received
            value and the source of the value. */
        if (xStatus == pdPASS)
        {
            if (xReceivedStructure.ucSource == mainSENDER_1)
            {
                vPrintStringAndNumber("Received From Sender 1 = ", xReceivedStructure.ucValue);
            }
            else
            {
                vPrintStringAndNumber("Received From Sender 2 = ", xReceivedStructure.ucValue);
            }
        }
        else
        {
            /* We did not receive anything from the queue.  This must be an error
            as this task should only run when the queue is full. */
            vPrintString("Could not receive from the queue!\n");
        }
    }
}

void CreateStructTasks(void)
{
    BaseType_t ret;

    /* The queue is created to hold a maximum of 3 structures of type xData. */
    xQueue = xQueueCreate(3, sizeof(xData));
    if (xQueue != NULL)
    {
        ret = xTaskCreate(vReceiverTask, "Struct Receiver", TASK_STACK_SIZE, NULL, 2, &xtask1_handle);
        if (ret != pdPASS)
        {
            xtask1_handle = NULL;
            vPrintStringAndNumber("Receiver creation failed: ", ret);
        }

        ret = xTaskCreate(vSenderTask, "Struct Sender 1", TASK_STACK_SIZE, (void *) & (xStructsToSend[0]), 2, &xtask2_handle);
        if (ret != pdPASS)
        {
            xtask2_handle = NULL;
            vPrintStringAndNumber("Sender 1 creation failed: ", ret);
        }

        ret = xTaskCreate(vSenderTask, "Struct Sender 2", TASK_STACK_SIZE, (void *) & (xStructsToSend[1]), 2, &xtask3_handle);
        if (ret != pdPASS)
        {
            xtask3_handle = NULL;
            vPrintStringAndNumber("Sender 2 creation failed: ", ret);
        }
    }
}

void DeleteStructTasks(void)
{
    if (xtask1_handle)
    {
        vTaskDelete(xtask1_handle);
        vPrintString("Struct Receiver deletion \r\n");
    }

    if (xtask2_handle)
    {
        vTaskDelete(xtask2_handle);
        vPrintString("Struct Sender 1 deletion \r\n");
    }

    if (xtask3_handle)
    {
        vTaskDelete(xtask3_handle);
        vPrintString("Struct Sender 2 deletion \r\n");
    }
}