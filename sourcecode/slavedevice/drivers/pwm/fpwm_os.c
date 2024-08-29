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
 * FilePath: fpwm_os.c
 * Date: 2022-08-15 14:20:19
 * LastEditTime: 2022-08-25 16:59:51
 * Description:  This file is for required function implementations of pwm driver used in FreeRTOS.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2022/08/26  first commit
 */
#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include "task.h"
#include "ftypes.h"
#include "fassert.h"
#include "fdebug.h"
#include "fpwm_os.h"
#include "fpwm.h"
#include "finterrupt.h"
#include "fpwm_hw.h"

#define FPWM_DEBUG_TAG "FFreeRTOSPwm"
#define FPWM_ERROR(format, ...) FT_DEBUG_PRINT_E(FPWM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPWM_WARN(format, ...)  FT_DEBUG_PRINT_W(FPWM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPWM_INFO(format, ...)  FT_DEBUG_PRINT_I(FPWM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPWM_DEBUG(format, ...) FT_DEBUG_PRINT_D(FPWM_DEBUG_TAG, format, ##__VA_ARGS__)

static FFreeRTOSPwm os_pwm[FPWM_NUM] = {0};

/**
 * @name: FFreeRTOSPwmInit
 * @msg:  init freeRTOS pwm instance, include init pwm and create mutex
 * @param {u32} instance_id, pwm instance id
 * @return {FFreeRTOSPwm *} pointer to os pwm instance
 */
FFreeRTOSPwm *FFreeRTOSPwmInit(u32 instance_id)
{
    FASSERT(instance_id < FPWM_NUM);
    FASSERT(FT_COMPONENT_IS_READY != os_pwm[instance_id].pwm_ctrl.is_ready);

    FPwmConfig pconfig;
    pconfig = *FPwmLookupConfig(instance_id);
    pconfig.irq_prority[FPWM_CHANNEL_0] = FREERTOS_PWM_IRQ_PRIORITY;
    pconfig.irq_prority[FPWM_CHANNEL_1] = FREERTOS_PWM_IRQ_PRIORITY;

    FASSERT(FPwmCfgInitialize(&os_pwm[instance_id].pwm_ctrl, &pconfig) == FT_SUCCESS);
    FASSERT((os_pwm[instance_id].pwm_semaphore = xSemaphoreCreateMutex()) != NULL);

    return (&os_pwm[instance_id]);
}

/**
 * @name: FFreeRTOSPwmDeinit
 * @msg:  deinit freeRTOS pwm instance, include stop pwm, deinit pwm and delete mutex
 * @param {FFreeRTOSPwm} *os_pwm_p, pointer to os pwm instance
 * @return err code information, FPWM_SUCCESS indicates success，others indicates failed
 */
FError FFreeRTOSPwmDeinit(FFreeRTOSPwm *os_pwm_p)
{
    FASSERT(os_pwm_p);
    FASSERT(os_pwm_p->pwm_semaphore != NULL);

    FPwmDeInitialize(&os_pwm_p->pwm_ctrl);
    vSemaphoreDelete(os_pwm_p->pwm_semaphore);
    memset(os_pwm_p, 0, sizeof(*os_pwm_p));

    return FPWM_SUCCESS;
}

/**
 * @name: FFreeRTOSPwmControl
 * @msg:  control freeRTOS pwm instance
 * @param {FFreeRTOSPwm} *os_pwm_p, pointer to os pwm instance
 * @param {int} cmd, control cmd
 * @param {void} *args, pointer to control cmd arguments
 * @return err code information, FPWM_SUCCESS indicates success，others indicates failed
 */
static FError FFreeRTOSPwmControl(FFreeRTOSPwm *os_pwm_p, int cmd, void *arg)
{
    FFreeRTOSPwmConfig *configuration = (FFreeRTOSPwmConfig *)arg;
    FError ret = FPWM_SUCCESS;

    /* New contrl can be performed only after current one is finished */
    if (pdFALSE == xSemaphoreTake(os_pwm_p->pwm_semaphore, portMAX_DELAY))
    {
        FPWM_ERROR("Pwm xSemaphoreTake failed.");
        /* We could not take the semaphore, exit with 0 data received */
        return FREERTOS_PWM_SEM_ERROR;
    }

    switch (cmd)
    {
        case FREERTOS_PWM_CTRL_SET:
            ret = FPwmVariableSet(&os_pwm_p->pwm_ctrl, configuration->channel, &configuration->pwm_cfg);
            break;

        case FREERTOS_PWM_CTRL_GET:
            ret = FPwmVariableGet(&os_pwm_p->pwm_ctrl, configuration->channel, &configuration->pwm_cfg);
            break;

        case FREERTOS_PWM_CTRL_ENABLE:
            FPwmEnable(&os_pwm_p->pwm_ctrl, configuration->channel);
            break;

        case FREERTOS_PWM_CTRL_DISABLE:
            FPwmDisable(&os_pwm_p->pwm_ctrl, configuration->channel);
            break;

        case FREERTOS_PWM_CTRL_DB_SET:
            ret = FPwmDbVariableSet(&os_pwm_p->pwm_ctrl, &configuration->db_cfg);
            break;

        case FREERTOS_PWM_CTRL_DB_GET:
            ret = FPwmDbVariableGet(&os_pwm_p->pwm_ctrl, &configuration->db_cfg);
            break;

        case FREERTOS_PWM_CTRL_PULSE_SET:
            ret = FPwmPulseSet(&os_pwm_p->pwm_ctrl, configuration->channel, configuration->pwm_cfg.pwm_pulse);
            break;

        default:
            FPWM_ERROR("Invalid cmd.");
            ret = FPWM_ERR_NOT_SUPPORT;
            break;
    }

    /* Enable next contrl. Current one is finished */
    if (pdFALSE == xSemaphoreGive(os_pwm_p->pwm_semaphore))
    {
        /* We could not post the semaphore, exit with error */
        FPWM_ERROR("Pwm xSemaphoreGive failed.");
        return FREERTOS_PWM_SEM_ERROR;
    }

    return ret;
}

/**
 * @name: FFreeRTOSPwmSet
 * @msg:  set freeRTOS pwm channel config, include div, period and pulse.
 * @param {FFreeRTOSPwm} *os_pwm_p, pointer to os pwm instance
 * @param {FPwmChannel} channel, pwm channel
 * @param {FPwmVariableConfig} pwm_cfg_p, pwm config parameters, include mode and duty
 * @return err code information, FPWM_SUCCESS indicates success，others indicates failed
 */
FError FFreeRTOSPwmSet(FFreeRTOSPwm *os_pwm_p, FPwmChannel channel, FPwmVariableConfig *pwm_cfg_p)
{
    FASSERT(os_pwm_p);
    FASSERT(os_pwm_p->pwm_semaphore != NULL);
    FASSERT(channel < FPWM_CHANNEL_NUM);
    FASSERT(pwm_cfg_p != NULL);
    FError ret = FPWM_SUCCESS;
    FFreeRTOSPwmConfig configuration = {0};
    configuration.pwm_cfg = *pwm_cfg_p;
    configuration.channel = channel;
    ret = FFreeRTOSPwmControl(os_pwm_p, FREERTOS_PWM_CTRL_SET, &configuration);
    return ret;
}

/**
 * @name: FFreeRTOSPwmGet
 * @msg:  get freeRTOS pwm channel config, include div, period and pulse.
 * @param {FFreeRTOSPwm} *os_pwm_p, pointer to os pwm instance
 * @param {FPwmChannel} channel, pwm channel
 * @param {FPwmVariableConfig} *pwm_cfg_p, pwm config parameters, include mode and duty
 * @return err code information, FPWM_SUCCESS indicates success，others indicates failed
 */
FError FFreeRTOSPwmGet(FFreeRTOSPwm *os_pwm_p, FPwmChannel channel, FPwmVariableConfig *pwm_cfg_p)
{
    FASSERT(os_pwm_p);
    FASSERT(os_pwm_p->pwm_semaphore != NULL);
    FASSERT(channel < FPWM_CHANNEL_NUM);
    FASSERT(pwm_cfg_p != NULL);
    FError ret = FPWM_SUCCESS;
    FFreeRTOSPwmConfig configuration = {0};
    configuration.channel = channel;

    ret = FFreeRTOSPwmControl(os_pwm_p, FREERTOS_PWM_CTRL_GET, &configuration);
    if (ret == FPWM_SUCCESS)
    {
        *pwm_cfg_p = configuration.pwm_cfg;
    }
    return ret;
}

/**
 * @name: FFreeRTOSPwmEnable
 * @msg:  enable or disable freeRTOS pwm channel output
 * @param {FFreeRTOSPwm} *os_pwm_p, pointer to os pwm instance
 * @param {FPwmChannel} channel, pwm channel
 * @param {boolean} state, TRUE-enable, FALSE-disable
 * @return err code information, FPWM_SUCCESS indicates success，others indicates failed
 */
FError FFreeRTOSPwmEnable(FFreeRTOSPwm *os_pwm_p, FPwmChannel channel, boolean state)
{
    FASSERT(os_pwm_p);
    FASSERT(os_pwm_p->pwm_semaphore != NULL);
    FASSERT(channel < FPWM_CHANNEL_NUM);

    FError ret = FPWM_SUCCESS;
    FFreeRTOSPwmConfig configuration = {0};
    configuration.channel = channel;

    if (state == TRUE)
    {
        ret = FFreeRTOSPwmControl(os_pwm_p, FREERTOS_PWM_CTRL_ENABLE, &configuration);
    }
    else
    {
        ret = FFreeRTOSPwmControl(os_pwm_p, FREERTOS_PWM_CTRL_DISABLE, &configuration);
    }

    return ret;
}

/**
 * @name: FFreeRTOSPwmDbSet
 * @msg:  set pwm db config, include polarity, input source, delay time.
 * @param {FFreeRTOSPwm} *os_pwm_p, pointer to os pwm instance
 * @param {FPwmDbVariableConfig} db_cfg_p, pwm db config parameters
 * @return err code information, FPWM_SUCCESS indicates success，others indicates failed
 */
FError FFreeRTOSPwmDbSet(FFreeRTOSPwm *os_pwm_p, FPwmDbVariableConfig *db_cfg_p)
{
    FASSERT(os_pwm_p);
    FASSERT(os_pwm_p->pwm_semaphore != NULL);
    FASSERT(db_cfg_p != NULL);
    FError ret = FPWM_SUCCESS;
    FFreeRTOSPwmConfig configuration = {0};
    configuration.db_cfg = *db_cfg_p;
    ret = FFreeRTOSPwmControl(os_pwm_p, FREERTOS_PWM_CTRL_DB_SET, &configuration);
    return ret;
}

/**
 * @name: FFreeRTOSPwmDbGet
 * @msg:  get pwm db config, include polarity, input source, delay time.
 * @param {FFreeRTOSPwm} *os_pwm_p, pointer to os pwm instance
 * @param {FPwmDbVariableConfig} db_cfg_p, pwm db config parameters
 * @return err code information, FPWM_SUCCESS indicates success，others indicates failed
 */
FError FFreeRTOSPwmDbGet(FFreeRTOSPwm *os_pwm_p, FPwmDbVariableConfig *db_cfg_p)
{
    FASSERT(os_pwm_p);
    FASSERT(os_pwm_p->pwm_semaphore != NULL);
    FASSERT(db_cfg_p != NULL);
    FError ret = FPWM_SUCCESS;
    FFreeRTOSPwmConfig configuration = {0};

    ret = FFreeRTOSPwmControl(os_pwm_p, FREERTOS_PWM_CTRL_DB_GET, &configuration);
    if (ret == FPWM_SUCCESS)
    {
        *db_cfg_p = configuration.db_cfg;
    }
    return ret;
}

/**
 * @name: FFreeRTOSPwmPulseSet
 * @msg:  set freeRTOS pwm channel pulse.
 * @param {FFreeRTOSPwm} *os_pwm_p, pointer to os pwm instance
 * @param {FPwmChannel} channel, pwm channel
 * @param {u16} pulse, pwm pulse to set
 * @return err code information, FPWM_SUCCESS indicates success，others indicates failed
 */
FError FFreeRTOSPwmPulseSet(FFreeRTOSPwm *os_pwm_p, FPwmChannel channel, u16 pulse)
{
    FASSERT(os_pwm_p);
    FASSERT(os_pwm_p->pwm_semaphore != NULL);
    FASSERT(channel < FPWM_CHANNEL_NUM);
    FError ret = FPWM_SUCCESS;

    FFreeRTOSPwmConfig configuration = {0};
    configuration.pwm_cfg.pwm_pulse = pulse;

    ret = FFreeRTOSPwmControl(os_pwm_p, FREERTOS_PWM_CTRL_PULSE_SET, &configuration);
    return ret;
}
