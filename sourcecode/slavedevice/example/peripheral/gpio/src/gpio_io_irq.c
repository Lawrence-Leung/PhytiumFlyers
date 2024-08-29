/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
 * All Rights Reserved.
 *
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,
 * either version 1.0 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details.
 *
 *
 * FilePath: gpio_io_irq.c
 * Date: 2022-07-22 13:57:42
 * LastEditTime: 2022-07-22 13:57:43 
 * Description:  This file is for gpio io irq implementation.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  zhugengyu  2022/8/26    init commit
 */
/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "fdebug.h"
#include "fsleep.h"
#include "fio_mux.h"

#include "fgpio_os.h"
#include "gpio_io_irq.h"
/************************** Constant Definitions *****************************/
#define PIN_IRQ_OCCURED     (0x1 << 0)
#define GPIO_WORK_TASK_NUM  2U
#if defined(CONFIG_TARGET_E2000D) || defined(CONFIG_TARGET_E2000Q)
#define IN_PIN_INDEX FFREERTOS_GPIO_PIN_INDEX(3, 0, 5)  /* GPIO 3-A-5 */
#define OUT_PIN_INDEX FFREERTOS_GPIO_PIN_INDEX(3, 0, 4)
#endif

#ifdef CONFIG_TARGET_PHYTIUMPI
#define IN_PIN_INDEX FFREERTOS_GPIO_PIN_INDEX(3, 0, 2)
#define OUT_PIN_INDEX FFREERTOS_GPIO_PIN_INDEX(3, 0, 1)
#endif
/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static FFreeRTOSFGpio *in_gpio = NULL;
static FFreeRTOSGpioConfig in_gpio_cfg;
static FFreeRTOSFGpio *out_gpio = NULL;
static FFreeRTOSGpioConfig out_gpio_cfg;
static xSemaphoreHandle init_locker = NULL;
static u32 in_pin = IN_PIN_INDEX; 
static u32 out_pin = OUT_PIN_INDEX;
static FFreeRTOSGpioPinConfig in_pin_config =
{
    .pin_idx = IN_PIN_INDEX,
    .mode = FGPIO_DIR_INPUT,
    .en_irq = TRUE,
    .irq_type = FGPIO_IRQ_TYPE_EDGE_RISING,
    .irq_handler = NULL,
    .irq_args = NULL
};
static FFreeRTOSGpioPinConfig out_pin_config =
{
    .pin_idx = OUT_PIN_INDEX,
    .mode = FGPIO_DIR_OUTPUT,
    .en_irq = FALSE
};
static EventGroupHandle_t event = NULL;
static TaskHandle_t output_task = NULL;
static TaskHandle_t input_task = NULL;
static TimerHandle_t exit_timer = NULL;
static boolean is_running = FALSE;

/***************** Macros (Inline Functions) Definitions *********************/
#define FGPIO_DEBUG_TAG "GPIO-IO"
#define FGPIO_ERROR(format, ...) FT_DEBUG_PRINT_E(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGPIO_WARN(format, ...)  FT_DEBUG_PRINT_W(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGPIO_INFO(format, ...)  FT_DEBUG_PRINT_I(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGPIO_DEBUG(format, ...) FT_DEBUG_PRINT_D(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/

/*****************************************************************************/
static void GpioIOIrqExitCallback(TimerHandle_t timer)
{
    printf("Exiting...\r\n");

    if (output_task) /* stop and delete send task */
    {
        vTaskDelete(output_task);
        output_task = NULL;
    }

    if (input_task) /* stop and delete recv task */
    {
        vTaskDelete(input_task);
        input_task = NULL;
    }

    if (FT_SUCCESS != FFreeRTOSGpioDeInit(in_gpio))
    {
        FGPIO_ERROR("Delete gpio failed.");
    }
    in_gpio = NULL;

    if (FFREERTOS_GPIO_PIN_CTRL_ID(out_pin) != FFREERTOS_GPIO_PIN_CTRL_ID(in_pin)) /* check if pin in diff ctrl */
    {
        if (FT_SUCCESS != FFreeRTOSGpioDeInit(out_gpio))
        {
            FGPIO_ERROR("Delete gpio failed.");
        }
    }
    out_gpio = NULL;

    if (event)
    {
        vEventGroupDelete(event);
        event = NULL;
    }

    if (init_locker)
    {
        vSemaphoreDelete(init_locker);
        init_locker = NULL;
    }

    if (exit_timer)
    {
        if (pdPASS != xTimerDelete(exit_timer, 0))
        {
            FGPIO_ERROR("Delete exit timer failed.");
        }
        exit_timer = NULL;
    }

    is_running = FALSE;
}

static void GpioIOAckPinIrq(s32 vector, void *param)
{
    BaseType_t xhigher_priority_task_woken = pdFALSE;
    BaseType_t x_result = pdFALSE;

    FGPIO_INFO("Ack pin irq.");
    x_result = xEventGroupSetBitsFromISR(event, PIN_IRQ_OCCURED,
                                         &xhigher_priority_task_woken);
}

static void GdmaInitTask(void *args)
{
    FError err = FT_SUCCESS;
    FGpioPinId pin_id;
    static const char *irq_type_str[] = {"failling-edge", "rising-edge", "low-level", "high-level"};

    FGPIO_INFO("out_pin: 0x%x, in_pin: 0x%x", out_pin, in_pin);

    out_gpio = FFreeRTOSGpioInit(FFREERTOS_GPIO_PIN_CTRL_ID(out_pin), &out_gpio_cfg);
    if (FFREERTOS_GPIO_PIN_CTRL_ID(out_pin) != FFREERTOS_GPIO_PIN_CTRL_ID(in_pin))
    {
        in_gpio = FFreeRTOSGpioInit(FFREERTOS_GPIO_PIN_CTRL_ID(in_pin), &in_gpio_cfg);
    }
    else
    {
        in_gpio = out_gpio; /* no need to init if in-pin and out-pin under same ctrl */
    }

    /* init output/input pin */
    FIOPadSetGpioMux(FFREERTOS_GPIO_PIN_CTRL_ID(out_pin), FFREERTOS_GPIO_PIN_ID(out_pin)); /* set io pad */
    FIOPadSetGpioMux(FFREERTOS_GPIO_PIN_CTRL_ID(in_pin), FFREERTOS_GPIO_PIN_ID(in_pin)); /* set io pad */

    out_pin_config.pin_idx = out_pin;
    err = FFreeRTOSSetupPin(out_gpio, &out_pin_config);
    FASSERT_MSG(FT_SUCCESS == err, "Init output gpio pin failed.");

    in_pin_config.pin_idx = in_pin;
    in_pin_config.irq_handler = GpioIOAckPinIrq;
    in_pin_config.irq_args = NULL;
    printf("Config input pin interrupt type as %s\r\n", irq_type_str[in_pin_config.irq_type]);
    err = FFreeRTOSSetupPin(in_gpio, &in_pin_config);
    FASSERT_MSG(FT_SUCCESS == err, "Init input gpio pin failed.");

    FASSERT_MSG(init_locker, "Init locker NULL");
    for (u32 loop = 0U; loop < GPIO_WORK_TASK_NUM; loop++)
    {
        xSemaphoreGive(init_locker);
    }

    vTaskDelete(NULL);
}

static void GpioIOIrqOutputTask(void *args)
{
    FASSERT(init_locker);
    xSemaphoreTake(init_locker, portMAX_DELAY);

    const TickType_t toggle_delay = pdMS_TO_TICKS(500UL); /* toggle every 500 ms */
    FGpioPinVal out_val = FGPIO_PIN_LOW;

    printf("Gpio ouptut task started. \r\n");

    for (;;)
    {
        printf(" ==> Set GPIO-%d-%c-%d as %s\r\n",
               FFREERTOS_GPIO_PIN_CTRL_ID(out_pin),
               (FGPIO_PORT_A == FFREERTOS_GPIO_PIN_PORT_ID(out_pin)) ? 'a' : 'b',
               FFREERTOS_GPIO_PIN_ID(out_pin),
               (out_val == FGPIO_PIN_LOW) ? "low" : "high");

        FFreeRTOSPinWrite(out_gpio, out_pin, out_val); /* start with low level */
        vTaskDelay(toggle_delay);
        out_val = (FGPIO_PIN_LOW == out_val) ? FGPIO_PIN_HIGH : FGPIO_PIN_LOW; /* toggle level */
    }
}

static boolean GpioIOWaitIrqOccurr(void)
{
    const TickType_t wait_delay = pdMS_TO_TICKS(2000U); /* just check 2sec wait */
    boolean ok = FALSE;
    EventBits_t ev = xEventGroupWaitBits(event,
                                         PIN_IRQ_OCCURED,
                                         pdTRUE, pdFALSE, wait_delay);

    if ((ev & PIN_IRQ_OCCURED))
    {
        ok = TRUE;
    }

    return ok;
}

static void GpioIOIrqInputTask(void *args)
{
    FASSERT(init_locker);
    xSemaphoreTake(init_locker, portMAX_DELAY);

    FGpioPinVal in_val;
    FError err = FT_SUCCESS;
    const TickType_t input_delay = pdMS_TO_TICKS(100UL); /* input every 500 ms */

    printf("Gpio input task started. \r\n");
    (void)FFreeRTOSSetIRQ(in_gpio, in_pin, TRUE);

    for (;;)
    {
        in_val = FFreeRTOSPinRead(in_gpio, in_pin);
        printf(" <== Get GPIO-%d-%c-%d in %s\r\n",
               FFREERTOS_GPIO_PIN_CTRL_ID(in_pin),
               (FGPIO_PORT_A == FFREERTOS_GPIO_PIN_PORT_ID(in_pin)) ? 'a' : 'b',
               FFREERTOS_GPIO_PIN_ID(in_pin),
               (in_val == FGPIO_PIN_LOW) ? "low" : "high");

        /* check for interrupt event */
        if (GpioIOWaitIrqOccurr())
        {
            printf("GPIO-%d-%c-%d, Interrrupt Asserted. \r\n",
                   FFREERTOS_GPIO_PIN_CTRL_ID(in_pin),
                   (FGPIO_PORT_A == FFREERTOS_GPIO_PIN_PORT_ID(in_pin)) ? 'a' : 'b',
                   FFREERTOS_GPIO_PIN_ID(in_pin));

            (void)FFreeRTOSSetIRQ(in_gpio, in_pin, TRUE); /* enable irq to recv next one */
        }
        else
        {
            printf("None Interrupt Assert.\r\n");
            continue;
        }

        vTaskDelay(input_delay);
    }
}

BaseType_t FFreeRTOSRunGpioIOIrq(u32 out_pin_idx, u32 in_pin_idx)
{
    BaseType_t ret = pdPASS;
    const TickType_t total_run_time = pdMS_TO_TICKS(10000UL); /* loopback run for 5 secs deadline */

    if (is_running)
    {
        FGPIO_ERROR("The task is running.");
        return pdPASS;
    }

    is_running = TRUE;

    FASSERT_MSG(NULL == event, "Event group exists.");
    FASSERT_MSG((event = xEventGroupCreate()) != NULL, "Create event group failed.");

    FASSERT_MSG(NULL == init_locker, "Init locker exists.");
    FASSERT_MSG((init_locker = xSemaphoreCreateCounting(GPIO_WORK_TASK_NUM, 0U)) != NULL, "Create event group failed.");

    out_pin = out_pin_idx;
    in_pin = in_pin_idx;

    taskENTER_CRITICAL(); /* no schedule when create task */

    ret = xTaskCreate((TaskFunction_t)GdmaInitTask,  /* task entry */
                      (const char *)"GdmaInitTask",/* task name */
                      (uint16_t)1024,  /* task stack size in words */
                      NULL, /* task params */
                      (UBaseType_t)configMAX_PRIORITIES - 1,  /* task priority */
                      NULL); /* task handler */

    FASSERT_MSG(pdPASS == ret, "Create task failed.");

    ret = xTaskCreate((TaskFunction_t)GpioIOIrqOutputTask,  /* task entry */
                      (const char *)"GpioIOIrqOutputTask",/* task name */
                      (uint16_t)1024,  /* task stack size in words */
                      NULL, /* task params */
                      (UBaseType_t)configMAX_PRIORITIES - 1,  /* task priority */
                      (TaskHandle_t *)&output_task); /* task handler */

    FASSERT_MSG(pdPASS == ret, "Create task failed.");

    ret = xTaskCreate((TaskFunction_t)GpioIOIrqInputTask,  /* task entry */
                      (const char *)"GpioIOIrqInputTask",/* task name */
                      (uint16_t)1024,  /* task stack size in words */
                      NULL, /* task params */
                      (UBaseType_t)configMAX_PRIORITIES - 2,  /* task priority */
                      (TaskHandle_t *)&input_task); /* task handler */

    FASSERT_MSG(pdPASS == ret, "Create task failed.");

    exit_timer = xTimerCreate("Exit-Timer",                 /* Text name for the software timer - not used by FreeRTOS. */
                              total_run_time,                 /* The software timer's period in ticks. */
                              pdFALSE,                        /* Setting uxAutoRealod to pdFALSE creates a one-shot software timer. */
                              NULL,                       /* use timer id to pass task data for reference. */
                              GpioIOIrqExitCallback);   /* The callback function to be used by the software timer being created. */

    FASSERT_MSG(NULL != exit_timer, "Create exit timer failed.");

    taskEXIT_CRITICAL(); /* allow schedule since task created */

    ret = xTimerStart(exit_timer, 0); /* start */

    FASSERT_MSG(pdPASS == ret, "Start exit timer failed.");

    return ret;
}
