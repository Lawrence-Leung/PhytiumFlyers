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
 * FilePath: fcan_os.c
 * Date: 2022-09-15 14:20:19
 * LastEditTime: 2022-09-21 16:59:51
 * Description:  This file is for required function implementations of can driver used in FreeRTOS.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2022/09/23  first commit
 * 1.1 wangxiaodong 2022/11/01  file name adaptation
 * 1.2 zhangyan     2023/2/7    improve functions
 */
#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include "task.h"
#include "ftypes.h"
#include "fassert.h"
#include "fdebug.h"
#include "fcan_os.h"
#include "fcan.h"
#include "finterrupt.h"
#include "fcan_hw.h"

#define FCAN_DEBUG_TAG "FFreeRTOSCan"
#define FCAN_ERROR(format, ...) FT_DEBUG_PRINT_E(FCAN_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCAN_WARN(format, ...)  FT_DEBUG_PRINT_W(FCAN_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCAN_INFO(format, ...)  FT_DEBUG_PRINT_I(FCAN_DEBUG_TAG, format, ##__VA_ARGS__)
#define FCAN_DEBUG(format, ...) FT_DEBUG_PRINT_D(FCAN_DEBUG_TAG, format, ##__VA_ARGS__)

static FFreeRTOSCan os_can[FCAN_NUM] = {0};

/**
 * @name: FFreeRTOSCanInit
 * @msg:  init freeRTOS can instance, include init can and create mutex
 * @param {u32} instance_id, can instance id
 * @return {FFreeRTOSCan *} pointer to os can instance
 */
FFreeRTOSCan *FFreeRTOSCanInit(u32 instance_id)
{
    FASSERT(instance_id < FCAN_NUM);
    FASSERT(FT_COMPONENT_IS_READY != os_can[instance_id].can_ctrl.is_ready);

    FCanConfig pconfig;
    pconfig = *FCanLookupConfig(instance_id);
    pconfig.irq_prority = FREERTOS_CAN_IRQ_PRIORITY;

    FASSERT(FCanCfgInitialize(&os_can[instance_id].can_ctrl, &pconfig) == FT_SUCCESS);
    FASSERT((os_can[instance_id].can_semaphore = xSemaphoreCreateMutex()) != NULL);
    
    return (&os_can[instance_id]);
}

/**
 * @name: FFreeRTOSCanDeinit
 * @msg:  deinit freeRTOS can instance, include stop can, deinit can and delete mutex
 * @param {FFreeRTOSCan} *os_can_p, pointer to os can instance
 * @return err code information, FCAN_SUCCESS indicates success，others indicates failed
 */
FError FFreeRTOSCanDeinit(FFreeRTOSCan *os_can_p)
{
    FASSERT(os_can_p);
    FASSERT(os_can_p->can_semaphore != NULL);
    
    FCanDeInitialize(&os_can_p->can_ctrl);
    vSemaphoreDelete(os_can_p->can_semaphore);
    memset(os_can_p, 0, sizeof(*os_can_p));
    
    return FCAN_SUCCESS;
}

/**
 * @name: FFreeRTOSCanControl
 * @msg:  control freeRTOS can instance
 * @param {FFreeRTOSCan} *os_can_p, pointer to os can instance
 * @param {int} cmd, control cmd
 * @param {void} *args, pointer to control cmd arguments
 * @return err code information, FCAN_SUCCESS indicates success，others indicates failed
 */
FError FFreeRTOSCanControl(FFreeRTOSCan *os_can_p, int cmd, void *arg)
{
    FError ret = FCAN_SUCCESS;
    FCanBaudrateConfig *baudrate_config;
    FCanStatus *status_p;
    FCanIdMaskConfig *id_mask_p;
    FCanIntrEventConfig *intr_event_p;
    boolean use_canfd;
    u32 *tran_mode;

    /* New contrl can be performed only after current one is finished */
    if (pdFALSE == xSemaphoreTake(os_can_p->can_semaphore, portMAX_DELAY))
    {
        FCAN_ERROR("Can xSemaphoreTake failed.");
        /* We could not take the semaphore, exit with 0 data received */
        return FREERTOS_CAN_SEM_ERROR;
    }

    switch (cmd)
    {
        case FREERTOS_CAN_CTRL_ENABLE:
            FCanEnable(&os_can_p->can_ctrl, TRUE);
            break;

        case FREERTOS_CAN_CTRL_DISABLE:
            FCanEnable(&os_can_p->can_ctrl, FALSE);
            break;

        case FREERTOS_CAN_CTRL_BAUDRATE_SET:
            baudrate_config = (FCanBaudrateConfig *)arg;
            ret = FCanBaudrateSet(&os_can_p->can_ctrl, baudrate_config);
            break;

        case FREERTOS_CAN_CTRL_STATUS_GET:
            status_p = (FCanStatus *)arg;
            ret = FCanStatusGet(&os_can_p->can_ctrl, status_p);
            break;
        
        case FREERTOS_CAN_CTRL_ID_MASK_SET:
            id_mask_p = (FCanIdMaskConfig *)arg;
            ret = FCanIdMaskFilterSet(&os_can_p->can_ctrl, id_mask_p);
            break;
            
        case FREERTOS_CAN_CTRL_ID_MASK_ENABLE:
            FCanIdMaskFilterEnable(&os_can_p->can_ctrl);
            break;
        
        case FREERTOS_CAN_CTRL_MODE_SET:
            tran_mode = (u32*)arg;
            FCanSetMode(&os_can_p->can_ctrl, *tran_mode);
            break;

        case FREERTOS_CAN_CTRL_INTR_SET:
            intr_event_p = (FCanIntrEventConfig *)arg;
            FCanRegisterInterruptHandler(&os_can_p->can_ctrl, intr_event_p);
            FCanInterruptEnable(&os_can_p->can_ctrl, intr_event_p->type);
            break;

        case FREERTOS_CAN_CTRL_FD_ENABLE:
            use_canfd = (boolean)(uintptr)arg;
            FCanFdEnable(&os_can_p->can_ctrl, use_canfd);
            break;    
        default:
            FCAN_ERROR("Invalid cmd.");
            ret = FCAN_INVAL_PARAM;
            break;
    }

    /* Enable next contrl. Current one is finished */
    if (pdFALSE == xSemaphoreGive(os_can_p->can_semaphore))
    {
        /* We could not post the semaphore, exit with error */
        FCAN_ERROR("Can xSemaphoreGive failed.");
        return FREERTOS_CAN_SEM_ERROR;
    }

    return ret;

}

/**
 * @name: FFreeRTOSCanSend
 * @msg:  send can frame
 * @param {FFreeRTOSCan} *os_can_p, pointer to os can instance
 * @param {FCanFrame} *frame_p, pointer to can frame
 * @return err code information, FCAN_SUCCESS indicates success，others indicates failed
 */
FError FFreeRTOSCanSend(FFreeRTOSCan *os_can_p, FCanFrame *frame_p)
{
    FASSERT(os_can_p);
    FASSERT(os_can_p->can_semaphore != NULL);
    FASSERT(frame_p);

    FError ret = FCAN_SUCCESS;

    /* New contrl can be performed only after current one is finished */
    if (pdFALSE == xSemaphoreTake(os_can_p->can_semaphore, portMAX_DELAY))
    {
        FCAN_ERROR("Can xSemaphoreTake failed.");
        /* We could not take the semaphore, exit with 0 data received */
        return FREERTOS_CAN_SEM_ERROR;
    }

    ret = FCanSend(&os_can_p->can_ctrl, frame_p);
    if (ret != FCAN_SUCCESS)
    {
        FCAN_ERROR("Can send failed.");
    }

    /* Enable next contrl. Current one is finished */
    if (pdFALSE == xSemaphoreGive(os_can_p->can_semaphore))
    {
        /* We could not post the semaphore, exit with error */
        FCAN_ERROR("Can xSemaphoreGive failed.");
        return FREERTOS_CAN_SEM_ERROR;
    }

    return ret;

}

/**
 * @name: FFreeRTOSCanRecv
 * @msg:  receive can frame
 * @param {FFreeRTOSCan} *os_can_p, pointer to os can instance
 * @param {FCanFrame} *frame_p, pointer to can frame
 * @return err code information, FCAN_SUCCESS indicates success，others indicates failed
 */
FError FFreeRTOSCanRecv(FFreeRTOSCan *os_can_p, FCanFrame *frame_p)
{
    FASSERT(os_can_p);
    FASSERT(os_can_p->can_semaphore != NULL);
    FASSERT(frame_p);

    FError ret = FCAN_SUCCESS;

    /* New contrl can be performed only after current one is finished */
    if (pdFALSE == xSemaphoreTake(os_can_p->can_semaphore, portMAX_DELAY))
    {
        FCAN_ERROR("Can xSemaphoreTake failed.");
        /* We could not take the semaphore, exit with 0 data received */
        return FREERTOS_CAN_SEM_ERROR;
    }

    ret = FCanRecv(&os_can_p->can_ctrl, frame_p);
    if (ret != FCAN_SUCCESS)
    {
        FCAN_ERROR("Can recv failed.");
    }

    /* Enable next contrl. Current one is finished */
    if (pdFALSE == xSemaphoreGive(os_can_p->can_semaphore))
    {
        /* We could not post the semaphore, exit with error */
        FCAN_ERROR("Can xSemaphoreGive failed.");
        return FREERTOS_CAN_SEM_ERROR;
    }
    
    return ret;

}