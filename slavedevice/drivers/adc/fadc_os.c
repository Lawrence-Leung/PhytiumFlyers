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
 * FilePath: fadc_os.c
 * Date: 2022-08-24 16:50:19
 * LastEditTime: 2022-08-26 16:59:51
 * Description:  This file is for required function implementations of adc driver used in FreeRTOS.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2022/08/31  first commit
 * 1.1 wangxiaodong 2022/11/01  file name adaptation
 */
#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include "task.h"
#include "ftypes.h"
#include "fassert.h"
#include "fdebug.h"
#include "fadc_os.h"
#include "fadc.h"
#include "finterrupt.h"
#include "fadc_hw.h"

#define FADC_DEBUG_TAG "FFreeRTOSAdc"
#define FADC_ERROR(format, ...) FT_DEBUG_PRINT_E(FADC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FADC_WARN(format, ...)  FT_DEBUG_PRINT_W(FADC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FADC_INFO(format, ...)  FT_DEBUG_PRINT_I(FADC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FADC_DEBUG(format, ...) FT_DEBUG_PRINT_D(FADC_DEBUG_TAG, format, ##__VA_ARGS__)

/* adc control type */
#define FREERTOS_ADC_CTRL_SET                   (1) /* set adc control convert configuration */
#define FREERTOS_ADC_CTRL_START                 (2) /* start adc convert */
#define FREERTOS_ADC_CTRL_STOP                  (3) /* stop adc convert */
#define FREERTOS_ADC_CTRL_CHANNEL_ENABLE        (4) /* enable adc channel */
#define FREERTOS_ADC_CTRL_CHANNEL_DISABLE       (5) /* disable adc channel */
#define FREERTOS_ADC_CTRL_CHANNEL_THRESHOLD_SET (6) /* set adc channel threshold */
#define FREERTOS_ADC_CTRL_READ                  (7) /* read adc channel convert value */
#define FREERTOS_ADC_CTRL_INTR_ENABLE           (8) /* enable adc interrupt */
#define FREERTOS_ADC_CTRL_INTR_DISABLE          (9) /* disable adc interrupt */

static FFreeRTOSAdc os_adc[FADC_NUM] = {0};

/**
 * @name: FFreeRTOSAdcControl
 * @msg:  control freeRTOS adc instance
 * @param {FFreeRTOSAdc} *os_adc_p, pointer to os adc instance
 * @param {int} cmd, control cmd
 * @param {void} *args, pointer to control cmd arguments
 * @return err code information, FADC_SUCCESS indicates success，others indicates failed
 */
static FError FFreeRTOSAdcControl(FFreeRTOSAdc *os_adc_p, int cmd, void *arg)
{
    FError ret = FADC_SUCCESS;
    FFreeRTOSAdcConfig *configuration = (FFreeRTOSAdcConfig *)arg;

    /* New contrl can be performed only after current one is finished */
    if (pdFALSE == xSemaphoreTake(os_adc_p->adc_semaphore, portMAX_DELAY))
    {
        FADC_ERROR("Adc xSemaphoreTake failed.");
        /* We could not take the semaphore, exit with 0 data received */
        return FREERTOS_ADC_SEM_ERROR;
    }

    switch (cmd)
    {
        case FREERTOS_ADC_CTRL_SET:
            ret = FAdcVariableConfig(&os_adc_p->adc_ctrl, &configuration->convert_config);
            break;

        case FREERTOS_ADC_CTRL_START:
            FAdcConvertStart(&os_adc_p->adc_ctrl);
            break;

        case FREERTOS_ADC_CTRL_STOP:
            FAdcConvertStop(&os_adc_p->adc_ctrl);
            break;

        case FREERTOS_ADC_CTRL_CHANNEL_ENABLE:
            FAdcChannelEnable(&os_adc_p->adc_ctrl, configuration->channel, TRUE);
            break;

        case FREERTOS_ADC_CTRL_CHANNEL_DISABLE:
            FAdcChannelEnable(&os_adc_p->adc_ctrl, configuration->channel, FALSE);
            break;

        case FREERTOS_ADC_CTRL_CHANNEL_THRESHOLD_SET:
            FAdcChannelThresholdSet(&os_adc_p->adc_ctrl, configuration->channel, &configuration->threshold_config);
            break;

        case FREERTOS_ADC_CTRL_READ:
            ret = FAdcReadConvertResult(&os_adc_p->adc_ctrl, configuration->channel, &configuration->value);
            break;

        case FREERTOS_ADC_CTRL_INTR_ENABLE:
            ret = FAdcInterruptEnable(&os_adc_p->adc_ctrl, configuration->channel, configuration->event_type);
            break;

        case FREERTOS_ADC_CTRL_INTR_DISABLE:
            ret = FAdcInterruptDisable(&os_adc_p->adc_ctrl, configuration->channel, configuration->event_type);
            break;

        default:
            FADC_ERROR("Invalid cmd.");
            ret = FADC_ERR_NOT_SUPPORT;
            break;
    }

    /* Enable next contrl. Current one is finished */
    if (pdFALSE == xSemaphoreGive(os_adc_p->adc_semaphore))
    {
        /* We could not post the semaphore, exit with error */
        FADC_ERROR("Adc xSemaphoreGive failed.");
        return FREERTOS_ADC_SEM_ERROR;
    }

    return ret;
}

/**
 * @name: FFreeRTOSAdcSet
 * @msg:  set freeRTOS adc channel config, include div, mode
 * @param {FFreeRTOSAdc} *os_adc_p, pointer to os adc instance
 * @param {FAdcConvertConfig} *adc_cfg_p, adc config parameters
 * @return err code information, FADC_SUCCESS indicates success，others indicates failed
 */
static FError FFreeRTOSAdcControllerSet(FFreeRTOSAdc *os_adc_p, FAdcConvertConfig *adc_cfg_p)
{
    FASSERT(os_adc_p);
    FASSERT(os_adc_p->adc_semaphore != NULL);
    FASSERT(adc_cfg_p != NULL);
    FError ret = FADC_SUCCESS;

    FFreeRTOSAdcConfig configuration;
    memset(&configuration, 0, sizeof(configuration));
    configuration.convert_config = *adc_cfg_p;

    ret = FFreeRTOSAdcControl(os_adc_p, FREERTOS_ADC_CTRL_SET, &configuration);
    return ret;
}

/**
 * @name: FFreeRTOSAdcConvertStart
 * @msg:  adc convert start or stop
 * @param {FFreeRTOSAdc} *os_adc_p, pointer to os adc instance
 * @param {boolean} state, TRUE-start, FALSE-stop
 * @return err code information, FADC_SUCCESS indicates success，others indicates failed
 */
static FError FFreeRTOSAdcConvertStart(FFreeRTOSAdc *os_adc_p, boolean state)
{
    FASSERT(os_adc_p);
    FASSERT(os_adc_p->adc_semaphore != NULL);
    FError ret = FADC_SUCCESS;

    if (state == TRUE)
    {
        ret = FFreeRTOSAdcControl(os_adc_p, FREERTOS_ADC_CTRL_START, NULL);
    }
    else
    {
        ret = FFreeRTOSAdcControl(os_adc_p, FREERTOS_ADC_CTRL_STOP, NULL);
    }
    return ret;
}

/**
 * @name: FFreeRTOSAdcEnable
 * @msg:  set adc channel threshold, include high_threshold and low_threshold
 * @param {FFreeRTOSAdc} *os_adc_p, pointer to os adc instance
 * @param {FAdcChannel} channel, adc channel
 * @param {FAdcThresholdConfig} *threshold_config_p, adc channel threshold config
 * @return err code information, FADC_SUCCESS indicates success，others indicates failed
 */
static FError FFreeRTOSAdcChannelThresholdSet(FFreeRTOSAdc *os_adc_p, FAdcChannel channel, FAdcThresholdConfig *threshold_config_p)
{
    FASSERT(os_adc_p);
    FASSERT(os_adc_p->adc_semaphore != NULL);
    FASSERT(channel < FADC_CHANNEL_NUM);
    FError ret = FADC_SUCCESS;

    FFreeRTOSAdcConfig configuration;
    memset(&configuration, 0, sizeof(configuration));
    configuration.channel = channel;
    configuration.threshold_config = *threshold_config_p;

    ret = FFreeRTOSAdcControl(os_adc_p, FREERTOS_ADC_CTRL_CHANNEL_THRESHOLD_SET, &configuration);

    return ret;
}

/**
 * @name: FFreeRTOSAdcEnable
 * @msg:  enable or disable freeRTOS adc channel output
 * @param {FFreeRTOSAdc} *os_adc_p, pointer to os adc instance
 * @param {FAdcChannel} channel, adc channel
 * @param {boolean} state, TRUE-enable, FALSE-disable
 * @return err code information, FADC_SUCCESS indicates success，others indicates failed
 */
static FError FFreeRTOSAdcChannelEnable(FFreeRTOSAdc *os_adc_p, FAdcChannel channel, boolean state)
{
    FASSERT(os_adc_p);
    FASSERT(os_adc_p->adc_semaphore != NULL);
    FASSERT(channel < FADC_CHANNEL_NUM);
    FError ret = FADC_SUCCESS;

    FFreeRTOSAdcConfig configuration;
    memset(&configuration, 0, sizeof(configuration));
    configuration.channel = channel;

    if (state == TRUE)
    {
        ret = FFreeRTOSAdcControl(os_adc_p, FREERTOS_ADC_CTRL_CHANNEL_ENABLE, &configuration);
    }
    else
    {
        ret = FFreeRTOSAdcControl(os_adc_p, FREERTOS_ADC_CTRL_CHANNEL_DISABLE, &configuration);
    }

    return ret;
}

/**
 * @name: FFreeRTOSAdcRead
 * @msg:  read adc channel convert value.
 * @param {FFreeRTOSAdc} *os_adc_p, pointer to os adc instance
 * @param {FAdcChannel} channel, adc channel
 * @param {FAdcIntrEventType} event_type, adc interrupt event type
 * @param {boolean} state, TRUE-enable, FALSE-disable
 * @return err code information, FADC_SUCCESS indicates success，others indicates failed
 */
static FError FFreeRTOSAdcInterruptEnable(FFreeRTOSAdc *os_adc_p, FAdcChannel channel, FAdcIntrEventType event_type, boolean state)
{
    FASSERT(os_adc_p);
    FASSERT(os_adc_p->adc_semaphore != NULL);
    FASSERT(channel < FADC_CHANNEL_NUM);
    FError ret = FADC_SUCCESS;

    FFreeRTOSAdcConfig configuration;
    memset(&configuration, 0, sizeof(configuration));
    configuration.channel = channel;
    configuration.event_type = event_type;

    if (state == TRUE)
    {
        ret = FFreeRTOSAdcControl(os_adc_p, FREERTOS_ADC_CTRL_INTR_ENABLE, &configuration);
    }
    else
    {
        ret = FFreeRTOSAdcControl(os_adc_p, FREERTOS_ADC_CTRL_INTR_DISABLE, &configuration);
    }
    return ret;
}


/**
 * @name: FFreeRTOSAdcInit
 * @msg:  init freeRTOS adc instance, include init adc and create mutex
 * @param {u32} instance_id, adc instance id
 * @return {FFreeRTOSAdc *} pointer to os adc instance
 */
FFreeRTOSAdc *FFreeRTOSAdcInit(u32 instance_id)
{
    FASSERT(instance_id < FADC_NUM);
    FASSERT(FT_COMPONENT_IS_READY != os_adc[instance_id].adc_ctrl.is_ready);

    FAdcConfig pconfig;
    pconfig = *FAdcLookupConfig(instance_id);
    pconfig.irq_prority = FREERTOS_ADC_IRQ_PRIORITY;

    FASSERT(FAdcCfgInitialize(&os_adc[instance_id].adc_ctrl, &pconfig) == FT_SUCCESS);
    FASSERT((os_adc[instance_id].adc_semaphore = xSemaphoreCreateMutex()) != NULL);

    return (&os_adc[instance_id]);
}

/**
 * @name: FFreeRTOSAdcDeinit
 * @msg:  deinit freeRTOS adc instance, include stop adc, deinit adc and delete mutex
 * @param {FFreeRTOSAdc} *os_adc_p, pointer to os adc instance
 * @return err code information, FADC_SUCCESS indicates success，others indicates failed
 */
FError FFreeRTOSAdcDeinit(FFreeRTOSAdc *os_adc_p)
{
    FASSERT(os_adc_p);
    FASSERT(os_adc_p->adc_semaphore != NULL);

    /* stop adc convert */
    FFreeRTOSAdcConvertStart(os_adc_p, FALSE);

    FAdcDeInitialize(&os_adc_p->adc_ctrl);
    vSemaphoreDelete(os_adc_p->adc_semaphore);
    memset(os_adc_p, 0, sizeof(*os_adc_p));

    return FADC_SUCCESS;
}

/**
 * @name: FFreeRTOSAdcRead
 * @msg:  read adc channel convert value.
 * @param {FFreeRTOSAdc} *os_adc_p, pointer to os adc instance
 * @param {FAdcChannel} channel, adc channel
 * @param {u16} *val, adc convert value
 * @return err code information, FADC_SUCCESS indicates success，others indicates failed
 */
FError FFreeRTOSAdcRead(FFreeRTOSAdc *os_adc_p, FAdcChannel channel, u16 *val)
{
    FASSERT(os_adc_p);
    FASSERT(os_adc_p->adc_semaphore != NULL);
    FASSERT(channel < FADC_CHANNEL_NUM);
    FError ret = FADC_SUCCESS;

    FFreeRTOSAdcConfig configuration;
    memset(&configuration, 0, sizeof(configuration));
    configuration.channel = channel;

    ret = FFreeRTOSAdcControl(os_adc_p, FREERTOS_ADC_CTRL_READ, &configuration);
    if (ret == FADC_SUCCESS)
    {
        *val = configuration.value;
    }
    return ret;
}

/**
 * @name: FFreeRTOSAdcSet
 * @msg:  set adc controller and channel configuration
 * @param {FFreeRTOSAdc} *os_adc_p, pointer to os adc instance
 * @param {FFreeRTOSAdcConfig} *adc_cfg_p, adc controller and channel configuration
 * @return err code information, FADC_SUCCESS indicates success，others indicates failed
 */
FError FFreeRTOSAdcSet(FFreeRTOSAdc *os_adc_p, FFreeRTOSAdcConfig *adc_cfg_p)
{
    FASSERT(os_adc_p);
    FASSERT(os_adc_p->adc_semaphore != NULL);
    FASSERT(adc_cfg_p);
    FError ret = FADC_SUCCESS;
    FAdcChannel channel = adc_cfg_p->channel;
    FASSERT(channel < FADC_CHANNEL_NUM);

    ret = FFreeRTOSAdcControllerSet(os_adc_p, &adc_cfg_p->convert_config);
    if (FADC_SUCCESS != ret)
    {
        FADC_ERROR("FFreeRTOSAdcControllerSet failed.");
        return FADC_ERR_CMD_FAILED;
    }

    ret = FFreeRTOSAdcChannelThresholdSet(os_adc_p, channel, &adc_cfg_p->threshold_config);
    if (FADC_SUCCESS != ret)
    {
        FADC_ERROR("FAdcChannelThresholdSet failed.");
        return FADC_ERR_CMD_FAILED;
    }

    /* enable adc channel */
    ret = FFreeRTOSAdcChannelEnable(os_adc_p, channel, TRUE);
    if (FADC_SUCCESS != ret)
    {
        FADC_ERROR("FFreeRTOSAdcChannelEnable failed.");
        return FADC_ERR_CMD_FAILED;
    }

    /* enable adc convert finish interrupt to know whether the convert is completed */
    ret = FFreeRTOSAdcInterruptEnable(os_adc_p, channel, adc_cfg_p->event_type, TRUE);
    if (FADC_SUCCESS != ret)
    {
        FADC_ERROR("FFreeRTOSAdcInterruptEnable failed.");
        return FADC_ERR_CMD_FAILED;
    }

    /* start adc convert */
    ret = FFreeRTOSAdcConvertStart(os_adc_p, TRUE);
    if (FADC_SUCCESS != ret)
    {
        FADC_ERROR("FFreeRTOSAdcConvertStart failed.");
        return FADC_ERR_CMD_FAILED;
    }

    return ret;
}