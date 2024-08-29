/*
This example demonstrates:
creates and starts a one-shot timer and an auto-reload timer in diffirent period.
*/
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/* The periods assigned to the one-shot and auto-reload timers respectively. */
#define ONE_SHOT_TIMER_PERIOD       ( pdMS_TO_TICKS( 3000UL ) )
#define AUTO_RELOAD_TIMER_PERIOD    ( pdMS_TO_TICKS( 3000UL ) )

/*-----------------------------------------------------------*/

/*
 * The callback functions used by the one-shot and auto-reload timers
 * respectively.
 */
static void prvOneShotTimerCallback(TimerHandle_t xTimer);
static void prvAutoReloadTimerCallback(TimerHandle_t xTimer);

/*-----------------------------------------------------------*/
static TimerHandle_t xAutoReloadTimer, xOneShotTimer;

void CreateTimerTasks(void)
{

    BaseType_t xTimer1Started, xTimer2Started;

    /* Create the one shot software timer, storing the handle to the created
    software timer in xOneShotTimer. */
    xOneShotTimer = xTimerCreate("Create OneShot",                   /* Text name for the software timer - not used by FreeRTOS. */
                                 ONE_SHOT_TIMER_PERIOD,    /* The software timer's period in ticks. */
                                 pdFALSE,                      /* Setting uxAutoRealod to pdFALSE creates a one-shot software timer. */
                                 0,                            /* This example does not use the timer id. */
                                 prvOneShotTimerCallback);     /* The callback function to be used by the software timer being created. */

    /* Create the auto-reload software timer, storing the handle to the created
    software timer in xAutoReloadTimer. */
    xAutoReloadTimer = xTimerCreate("Create AutoReload",                     /* Text name for the software timer - not used by FreeRTOS. */
                                    AUTO_RELOAD_TIMER_PERIOD,  /* The software timer's period in ticks. */
                                    pdTRUE,                        /* Set uxAutoRealod to pdTRUE to create an auto-reload software timer. */
                                    0,                             /* This example does not use the timer id. */
                                    prvAutoReloadTimerCallback);   /* The callback function to be used by the software timer being created. */

    /* Check the timers were created. */
    if ((xOneShotTimer != NULL) && (xAutoReloadTimer != NULL))
    {
        /* Start the software timers, using a block time of 0 (no block time).
        The scheduler has not been started yet so any block time specified here
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
            vPrintf("CreateSoftwareTimerTasks xTimerStart failed \r\n");
        }
    }
    else
    {
        vPrintf("CreateSoftwareTimerTasks xTimerCreate failed \r\n");
    }

}

void DeleteTimerTasks(void)
{
    BaseType_t xReturn = pdFAIL;
    xReturn = xTimerDelete(xOneShotTimer, 0);
    if (xReturn != pdPASS)
    {
        vPrintf("DeleteSoftwareTimerTasks xTimerDelete OneShot failed.\r\n");
    }
    else
    {
        vPrintf("DeleteSoftwareTimerTasks xTimerDelete OneShot success.\r\n");
    }

    xReturn = xTimerDelete(xAutoReloadTimer, 0);
    if (xReturn != pdPASS)
    {
        vPrintf("DeleteSoftwareTimerTasks xTimerDelete AutoReload failed.\r\n");
    }
    else
    {
        vPrintf("DeleteSoftwareTimerTasks xTimerDelete AutoReload success.\r\n");
    }
}


/*-----------------------------------------------------------*/

static void prvOneShotTimerCallback(TimerHandle_t xTimer)
{
    static TickType_t xTimeNow;

    /* Obtain the current tick count. */
    xTimeNow = xTaskGetTickCount();

    /* Output a string to show the time at which the callback was executed. */
    vPrintf("One-shot timer callback executing %d ticks.\r\n", xTimeNow);
}
/*-----------------------------------------------------------*/

static void prvAutoReloadTimerCallback(TimerHandle_t xTimer)
{
    static TickType_t xTimeNow;

    /* Obtain the current tick count. */
    xTimeNow = xTaskGetTickCount();

    /* Output a string to show the time at which the callback was executed. */
    vPrintf("Auto-reload timer callback executing %d ticks.\r\n", xTimeNow);
}
/*-----------------------------------------------------------*/








