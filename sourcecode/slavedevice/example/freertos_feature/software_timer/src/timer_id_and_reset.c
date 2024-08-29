/*
This example demonstrates:
how to use the software timer ID as timer specific storage;
each software timer keeps a count of the number of times it has expired in its own ID;
Auto reload timer will stop after it has executed 5 times which period is 1000,
when the one shot timer is run after 9333 ticks, it will reset auto reload timer.
*/
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/* The periods assigned to the one-shot and auto-reload timers respectively. */
#define ONE_SHOT_TIMER_PERIOD       ( pdMS_TO_TICKS( 12000UL ) )
#define AUTO_RELOAD_TIMER_PERIOD    ( pdMS_TO_TICKS( 3000UL ) )

/*-----------------------------------------------------------*/

/*
 * The callback function that is used by both timers.
 */
static void prvTimerCallback(TimerHandle_t xTimer);

/*-----------------------------------------------------------*/

/* The timer handles are used inside the callback function so this time are
given file scope. */
static TimerHandle_t xAutoReloadTimer, xOneShotTimer;

void CreateTimerResetTasks(void)
{
    BaseType_t xTimer1Started, xTimer2Started;

    /* Create the one shot timer, storing the handle to the created timer in
    xOneShotTimer. */
    xOneShotTimer = xTimerCreate("Reset OneShot",                    /* Text name for the timer - not used by FreeRTOS. */
                                 ONE_SHOT_TIMER_PERIOD,    /* The timer's period in ticks. */
                                 pdFALSE,                      /* Set uxAutoRealod to pdFALSE to create a one-shot timer. */
                                 0,                            /* The timer ID is initialised to 0. */
                                 prvTimerCallback);            /* The callback function to be used by the timer being created. */

    /* Create the auto-reload, storing the handle to the created timer in
    xAutoReloadTimer. */
    xAutoReloadTimer = xTimerCreate("Reset AutoReload",                  /* Text name for the timer - not used by FreeRTOS. */
                                    AUTO_RELOAD_TIMER_PERIOD,  /* The timer's period in ticks. */
                                    pdTRUE,                        /* Set uxAutoRealod to pdTRUE to create an auto-reload timer. */
                                    0,                             /* The timer ID is initialised to 0. */
                                    prvTimerCallback);             /* The callback function to be used by the timer being created. */

    /* Check the timers were created. */
    if ((xOneShotTimer != NULL) && (xAutoReloadTimer != NULL))
    {
        /* Start the timers, using a block time of 0 (no block time).  The
        scheduler has not been started yet so any block time specified here
        would be ignored anyway. */
        xTimer1Started = xTimerStart(xOneShotTimer, 0);
        xTimer2Started = xTimerStart(xAutoReloadTimer, 0);

        /* The implementation of xTimerStart() uses the timer command queue, and
        xTimerStart() will fail if the timer command queue gets full.  The timer
        service task does not get created until the scheduler is started, so all
        commands sent to the command queue will stay in the queue until after
        the scheduler has been started.  Check both calls to xTimerStart()
        passed. */
        if ((xTimer1Started != pdPASS) || (xTimer2Started != pdPASS))
        {
            vPrintString("CreateSoftwareTimerTasks xTimerStart failed \r\n");
        }
    }
    else
    {
        vPrintString("CreateSoftwareTimerTasks xTimerCreate failed \r\n");
    }

}

void DeleteTimerResetTasks(void)
{
    BaseType_t xReturn = pdFAIL;
    xReturn = xTimerDelete(xOneShotTimer, 0);
    if (xReturn != pdPASS)
    {
        vPrintString("DeleteTimerResetTasks xTimerDelete OneShot failed.\r\n");
    }
    else
    {
        vPrintString("DeleteTimerResetTasks xTimerDelete OneShot success.\r\n");
    }

    xReturn = xTimerDelete(xAutoReloadTimer, 0);
    if (xReturn != pdPASS)
    {
        vPrintString("DeleteTimerResetTasks xTimerDelete AutoReload failed.\r\n");
    }
    else
    {
        vPrintString("DeleteTimerResetTasks xTimerDelete AutoReload success.\r\n");
    }
}

/*-----------------------------------------------------------*/

static void prvTimerCallback(TimerHandle_t xTimer)
{
    TickType_t xTimeNow;
    uint32_t ulExecutionCount;

    /* The count of the number of times this software timer has expired is
    stored in the timer's ID.  Obtain the ID, increment it, then save it as the
    new ID value.  The ID is a void pointer, so is cast to a uint32_t. */
    ulExecutionCount = (uint32_t)(uintptr) pvTimerGetTimerID(xTimer);
    vPrintStringAndNumber("pvTimerGetTimerID = ", ulExecutionCount);
    ulExecutionCount++;
    vTimerSetTimerID(xTimer, (void *)(uintptr)ulExecutionCount);

    /* Obtain the current tick count. */
    xTimeNow = xTaskGetTickCount();

    /* The handle of the one-shot timer was stored in xOneShotTimer when the
    timer was created.  Compare the handle passed into this function with
    xOneShotTimer to determine if it was the one-shot or auto-reload timer that
    expired, then output a string to show the time at which the callback was
    executed. */
    if (xTimer == xOneShotTimer)
    {
        vPrintStringAndNumber("One-shot timer callback executing", xTimeNow);
        vPrintString("One-shot timer reset Auto-reload timer ");
        xTimerReset(xAutoReloadTimer, 0);
    }
    else
    {
        /* xTimer did not equal xOneShotTimer, so it must be the auto-reload
        timer that expired. */
        vPrintStringAndNumber("Auto-reload timer callback executing", xTimeNow);

        if (ulExecutionCount == 3)
        {
            /* Stop the auto-reload timer after it has executed 5 times.  This
            callback function executes in the context of the RTOS daemon task so
            must not call any functions that might place the daemon task into
            the Blocked state.  Therefore a block time of 0 is used. */
            xTimerStop(xTimer, 0);
            vPrintString("Auto-reload timer stopped!!! ");
        }
    }
}
/*-----------------------------------------------------------*/

