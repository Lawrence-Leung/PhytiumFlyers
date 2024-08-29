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
 * FilePath: fgpio_os.c
 * Date: 2022-07-22 11:33:51
 * LastEditTime: 2022-07-22 11:33:51
 * Description:  This file is for required function implementations of gpio driver used in FreeRTOS.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/7/27   init commit
 */
/***************************** Include Files *********************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "finterrupt.h"
#include "fdebug.h"
#include "fsleep.h"
#include "fcpu_info.h"

#include "fgpio_os.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static FFreeRTOSFGpio gpio[FGPIO_NUM]; /* instance of all gpio ctrl */

/***************** Macros (Inline Functions) Definitions *********************/
#define FGPIO_DEBUG_TAG "FGPIO-OS"
#define FGPIO_ERROR(format, ...) FT_DEBUG_PRINT_E(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGPIO_WARN(format, ...)  FT_DEBUG_PRINT_W(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGPIO_INFO(format, ...)  FT_DEBUG_PRINT_I(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGPIO_DEBUG(format, ...) FT_DEBUG_PRINT_D(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/

/*****************************************************************************/
static inline FError FGpioOsTakeSema(SemaphoreHandle_t locker)
{
    FASSERT_MSG((NULL != locker), "Locker not exists.");
    if (pdFALSE == xSemaphoreTake(locker, portMAX_DELAY))
    {
        FGPIO_ERROR("Failed to give locker!!!");
        return FFREERTOS_GPIO_SEMA_ERR;
    }

    return FFREERTOS_GPIO_OK;
}

static inline void FGpioOsGiveSema(SemaphoreHandle_t locker)
{
    FASSERT_MSG((NULL != locker), "Locker not exists.");
    if (pdFALSE == xSemaphoreGive(locker))
    {
        FGPIO_ERROR("Failed to give locker!!!");
    }

    return;
}

static inline void FGpioOsGetId(u32 pin_idx, FGpioPinId *pin_id)
{
    FASSERT(pin_id);

    /* convert u32 pin_idx to FGpioPinId pin_id */
    pin_id->ctrl = FFREERTOS_GPIO_PIN_CTRL_ID(pin_idx);
    pin_id->port = FFREERTOS_GPIO_PIN_PORT_ID(pin_idx);
    pin_id->pin = FFREERTOS_GPIO_PIN_ID(pin_idx);
    //FGPIO_INFO("Pin index = 0x%x", pin_idx);
    //FGPIO_INFO("is gpio-%d-%d-%d", pin_id->ctrl, pin_id->port, pin_id->pin);
}

/* setup gpio ctrl interrupt */
static void FGpioOsSetupCtrlIRQ(FFreeRTOSFGpio *const instance)
{
    FGpio *ctrl = &instance->ctrl;
    u32 cpu_id;
    u32 irq_num = ctrl->config.irq_num[0];

    GetCpuId(&cpu_id);
    FGPIO_INFO("cpu_id is cpu_id %d", cpu_id);
    InterruptSetTargetCpus(irq_num, cpu_id);

    /* setup interrupt */
    InterruptSetPriority(irq_num, FFREERTOS_GPIO_IRQ_PRIORITY);

    /* register intr handler */
    InterruptInstall(irq_num,
                     FGpioInterruptHandler,
                     ctrl,
                     NULL);

    InterruptUmask(irq_num);

    return;
}

/**
 * @name: FFreeRTOSGpioInit
 * @msg: init and get gpio instance
 * @return {*}
 * @param {u32} id, gpio instance id
 * @param {FFreeRTOSGpioConfig} *input_config, input configuration
 */
FFreeRTOSFGpio *FFreeRTOSGpioInit(u32 id, const FFreeRTOSGpioConfig *input_config)
{
    FASSERT_MSG(id < FGPIO_NUM, "Invalid gpio id.");
    FFreeRTOSFGpio *instance = &gpio[id];
    FGpio *ctrl = &instance->ctrl;
    FGpioConfig *config = &ctrl->config;
    FError err = FT_SUCCESS;

    if (FT_COMPONENT_IS_READY == ctrl->is_ready)
    {
        // FGPIO_WARN("gpio-%d already init.", config->instance_id);
        return instance;
    }

    /* no scheduler during init */
    taskENTER_CRITICAL();

    *config = *FGpioLookupConfig(id);
    err = FGpioCfgInitialize(ctrl, config);
    if (FGPIO_SUCCESS != err)
    {
        goto err_exit;
    }

    FGpioPinId pin_of_ctrl =
    {
        .ctrl = ctrl->config.instance_id,
        .port = FGPIO_PORT_A,
        .pin = FGPIO_PIN_0
    };

    if (FGPIO_IRQ_BY_CONTROLLER == FGpioGetPinIrqSourceType(pin_of_ctrl)) /* setup for ctrl report interrupt */
    {
        FGpioOsSetupCtrlIRQ(instance);
    }

    FASSERT_MSG(NULL == instance->locker, "Locker exists!!!");
    FASSERT_MSG((instance->locker = xSemaphoreCreateMutex()) != NULL, "Create mutex failed!!!");

err_exit:
    taskEXIT_CRITICAL(); /* allow schedule after init */
    return (FT_SUCCESS == err) ? instance : NULL; /* exit with NULL if failed */
}

/**
 * @name: FFreeRTOSGpioDeInit
 * @msg: deinit gpio instance
 * @return {*}
 * @param {FFreeRTOSFGpio} *instance, freertos gpio instance
 */
FError FFreeRTOSGpioDeInit(FFreeRTOSFGpio *const instance)
{
    FASSERT(instance);
    FGpio *ctrl = &instance->ctrl;
    FError err = FT_SUCCESS;

    if (FT_COMPONENT_IS_READY != ctrl->is_ready)
    {
        FGPIO_WARN("gpio-%d not yet init.", ctrl->config.instance_id);
        return FFREERTOS_GPIO_NOT_INIT;
    }

    /* no scheduler during deinit */
    taskENTER_CRITICAL();

    FGpioDeInitialize(ctrl);

    FASSERT_MSG(NULL != instance->locker, "Locker not exists!!!");
    vSemaphoreDelete(instance->locker);
    instance->locker = NULL;

    taskEXIT_CRITICAL(); /* allow schedule after init */
    return err;
}

/* setup gpio pin interrupt */
static void FGpioOSSetupPinIRQ(FFreeRTOSFGpio *const instance, FGpioPin *const pin, const FFreeRTOSGpioPinConfig *config)
{
    u32 cpu_id;
    FGpio *ctrl = &instance->ctrl;
    u32 irq_num = ctrl->config.irq_num[pin->index.pin];

    GetCpuId(&cpu_id);
    FGPIO_INFO("cpu_id is cpu_id %d", cpu_id);

    InterruptSetTargetCpus(irq_num, cpu_id);

    /* setup interrupt */
    InterruptSetPriority(irq_num, FFREERTOS_GPIO_IRQ_PRIORITY);

    /* register intr handler */
    InterruptInstall(irq_num, config->irq_handler, config->irq_args, NULL);

    InterruptUmask(irq_num);

    return;
}

/**
 * @name: FFreeRTOSSetupPin
 * @msg: config and setup pin
 * @return {*}
 * @param {FFreeRTOSFGpio} *instance, freertos gpio instance
 * @param {FFreeRTOSGpioPinConfig} *config, gpio pin configuration
 */
FError FFreeRTOSSetupPin(FFreeRTOSFGpio *const instance, const FFreeRTOSGpioPinConfig *config)
{
    FASSERT(instance && config);
    FGpio *ctrl = &instance->ctrl;
    FGpioPinId pin_id;
    FGpioOsGetId(config->pin_idx, &pin_id); /* convert pin id */
    u32 ctrl_id = ctrl->config.instance_id;
    FASSERT_MSG((ctrl_id == pin_id.ctrl), "Invalid instance for pin.");
    FGpioPin *const pin = &instance->pins[pin_id.port][pin_id.pin];
    FError err = FT_SUCCESS;
    boolean irq_one_time = TRUE;

    err = FGpioOsTakeSema(instance->locker);
    if (FFREERTOS_GPIO_OK != err)
    {
        return err;
    }

    /* de-init pin if setup */
    if (FT_COMPONENT_IS_READY == pin->is_ready)
    {
        FGpioPinDeInitialize(pin);
    }

    /* init pin */
    err = FGpioPinInitialize(ctrl, pin, pin_id);
    if (FGPIO_SUCCESS != err)
    {
        FGPIO_ERROR("Init pin %d-%d failed, err: 0x%x",
                    pin_id.port,
                    pin_id.pin,
                    err);
        goto err_exit;
    }

    /* setup pin direction */
    FGpioSetDirection(pin, config->mode);
    // FGPIO_INFO("Set GPIO-%d-%c-%d direction %s",
    //            pin_id.ctrl,
    //            (FGPIO_PORT_A == pin_id.port) ? 'a' : 'b',
    //            pin_id.pin,
    //            (FGPIO_DIR_INPUT == config->mode) ? "IN" : "OUT");

    /* setup input-pin irq */
    if (TRUE == config->en_irq)
    {
        FGpioSetInterruptMask(pin, FALSE); /* disable pin irq */
        if (FGPIO_IRQ_BY_PIN == FGpioGetPinIrqSourceType(pin_id)) /* setup for pin report interrupt */
        {
            FGpioOSSetupPinIRQ(instance, pin, config);
        }

        FGpioRegisterInterruptCB(pin, config->irq_handler, config->irq_args, irq_one_time); /* register intr callback */
    }

err_exit:
    FGpioOsGiveSema(instance->locker);
    return err;
}

/**
 * @name: FFreeRTOSSetIRQ
 * @msg: enable/disable interrupt of pin
 * @return {*}
 * @param {FFreeRTOSFGpio} *instance, freertos gpio instance
 * @param {u32} pin_idx, index of pin, use FFREERTOS_GPIO_PIN_INDEX
 * @param {boolean} en_irq, TRUE: enable interrupt, FALSE: disable
 */
FError FFreeRTOSSetIRQ(FFreeRTOSFGpio *const instance, u32 pin_idx, boolean en_irq)
{
    FASSERT(instance);
    FGpioPinId pin_id;
    FGpioOsGetId(pin_idx, &pin_id); /* convert pin id */
    FGpio *ctrl = &instance->ctrl;
    u32 ctrl_id = ctrl->config.instance_id;
    FASSERT_MSG((ctrl_id == pin_id.ctrl), "Invalid instance for pin.");
    FGpioPin *const pin = &instance->pins[pin_id.port][pin_id.pin];
    FError err = FT_SUCCESS;

    err = FGpioOsTakeSema(instance->locker);
    if (FFREERTOS_GPIO_OK != err)
    {
        return err;
    }

    FGpioSetInterruptMask(pin, en_irq);

    FGpioOsGiveSema(instance->locker);
    return err;
}

/**
 * @name: FFreeRTOSPinWrite
 * @msg: set output pin value
 * @return {*}
 * @param {FFreeRTOSFGpio} *instance, freertos gpio instance
 * @param {u32} pin_idx, index of gpio pin
 * @param {u32} value, level set to gpio pin
 */
FError FFreeRTOSPinWrite(FFreeRTOSFGpio *const instance, u32 pin_idx, u32 value)
{
    FASSERT(instance);
    FGpioPinId pin_id;
    FGpioOsGetId(pin_idx, &pin_id); /* convert pin id */
    FGpio *ctrl = &instance->ctrl;
    u32 ctrl_id = ctrl->config.instance_id;
    FASSERT_MSG((ctrl_id == pin_id.ctrl), "Invalid instance for pin.");
    FGpioPin *const pin = &instance->pins[pin_id.port][pin_id.pin];
    FError err = FT_SUCCESS;

    err = FGpioOsTakeSema(instance->locker);
    if (FFREERTOS_GPIO_OK != err)
    {
        return err;
    }

    err = FGpioSetOutputValue(pin, (FGpioPinVal)value);

    FGpioOsGiveSema(instance->locker);
    return err;
}

/**
 * @name: FFreeRTOSPinRead
 * @msg: get input pin value
 * @return {u32} level input by pin
 * @param {FFreeRTOSFGpio} *instance, freertos gpio instance
 * @param {u32} pin_idx, index of gpio pin
 */
u32 FFreeRTOSPinRead(FFreeRTOSFGpio *const instance, u32 pin_idx)
{
    FASSERT(instance);
    FGpio *ctrl = &instance->ctrl;
    FGpioPinId pin_id;
    FGpioOsGetId(pin_idx, &pin_id); /* convert pin id */
    u32 ctrl_id = ctrl->config.instance_id;
    FASSERT_MSG((ctrl_id == pin_id.ctrl), "Invalid instance for pin.");
    FGpioPin *const pin = &instance->pins[pin_id.port][pin_id.pin];
    FError err = FT_SUCCESS;
    FGpioPinVal val = FGPIO_PIN_LOW;

    err = FGpioOsTakeSema(instance->locker);
    if (FFREERTOS_GPIO_OK != err)
    {
        return val;
    }

    val = FGpioGetInputValue(pin);

    FGpioOsGiveSema(instance->locker);
    return val;
}