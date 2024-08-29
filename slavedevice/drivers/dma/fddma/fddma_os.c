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
 * FilePath: fddma_os.c
 * Date: 2022-07-18 08:51:25
 * LastEditTime: 2022-07-18 08:51:25
 * Description:  This file is for required function implementations of ddma driver used in FreeRTOS.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/7/27   init commit
 */

/***************************** Include Files *********************************/
#include <string.h>

#include "fassert.h"
#include "fdebug.h"
#include "finterrupt.h"
#include "fcpu_info.h"
#include "fsleep.h"

#include "fddma_os.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static FFreeRTOSDdma ddma[FDDMA_INSTANCE_NUM];

/***************** Macros (Inline Functions) Definitions *********************/
#define FDDMA_DEBUG_TAG "DDMA-OS"
#define FDDMA_ERROR(format, ...)   FT_DEBUG_PRINT_E(FDDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDDMA_WARN(format, ...)    FT_DEBUG_PRINT_W(FDDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDDMA_INFO(format, ...)    FT_DEBUG_PRINT_I(FDDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDDMA_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FDDMA_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/
static inline FError FDdmaOsTakeSema(SemaphoreHandle_t locker)
{
    FASSERT_MSG((NULL != locker), "Locker not exists.");
    if (pdFALSE == xSemaphoreTake(locker, portMAX_DELAY))
    {
        FDDMA_ERROR("Failed to give locker!!!");
        return FFREERTOS_DDMA_SEMA_ERR;
    }

    return FFREERTOS_DDMA_OK;
}

static inline void FDdmaOsGiveSema(SemaphoreHandle_t locker)
{
    FASSERT_MSG((NULL != locker), "Locker not exists.");
    if (pdFALSE == xSemaphoreGive(locker))
    {
        FDDMA_ERROR("Failed to give locker!!!");
    }

    return;
}

/*****************************************************************************/
static void FDdmaOsSetupInterrupt(FDdma *const instance)
{
    FASSERT(instance);
    FDdmaConfig *config = &instance->config;
    uintptr base_addr = config->base_addr;
    u32 cpu_id = 0;

    GetCpuId(&cpu_id);
    FDDMA_INFO("cpu_id is cpu_id %d", cpu_id);
    InterruptSetTargetCpus(config->irq_num, cpu_id);
    InterruptSetPriority(config->irq_num, config->irq_prority);

    /* register intr callback */
    InterruptInstall(config->irq_num,
                     FDdmaIrqHandler,
                     instance,
                     NULL);

    /* enable ddma0 irq */
    InterruptUmask(config->irq_num);

    FDDMA_INFO("ddma interrupt setup done!!!");
    return;
}

/**
 * @name: FFreeRTOSDdmaInit
 * @msg: init and get ddma instance
 * @return {FFreeRTOSDdma *} return NULL if failed, otherwise success and return instance
 * @param {u32} id, ddma instance id
 * @param {FFreeRTOSDdmaConfig} *input_config, freertos ddma config
 */
FFreeRTOSDdma *FFreeRTOSDdmaInit(u32 id, const FFreeRTOSDdmaConfig *input_config)
{
    FASSERT(id < FDDMA_INSTANCE_NUM);
    FFreeRTOSDdma *instance = &ddma[id];
    FDdma *ctrl = &instance->ctrl;
    FDdmaConfig config;
    FError err = FT_SUCCESS;

    if (FT_COMPONENT_IS_READY == ctrl->is_ready)
    {
        FDDMA_WARN("ddma-%d already init", config.id);
        return instance;
    }

    /* no scheduler during init */
    taskENTER_CRITICAL();

    instance->config = *input_config;
    config = *FDdmaLookupConfig(id);
    config.irq_prority = FFREERTOS_DDMA_IRQ_PRIORITY;
    err = FDdmaCfgInitialization(ctrl, &config);
    if (FDDMA_SUCCESS != err)
    {
        FDDMA_ERROR("Init ddma-%d failed, err: 0x%x!!!", id, err);
        goto err_exit;
    }

    FASSERT_MSG(NULL == instance->locker, "Locker exists!!!");
    FASSERT_MSG((instance->locker = xSemaphoreCreateMutex()) != NULL, "Create mutex failed!!!");

    FDdmaOsSetupInterrupt(ctrl);

err_exit:
    taskEXIT_CRITICAL(); /* allow schedule after init */
    return (FT_SUCCESS == err) ? instance : NULL; /* exit with NULL if failed */
}

/**
 * @name: FFreeRTOSDdmaDeinit
 * @msg: deinit ddma instance
 * @return {FError} FT_SUCCESS if deinit success
 * @param {FFreeRTOSDdma} *instance, freertos ddma instance
 */
FError FFreeRTOSDdmaDeinit(FFreeRTOSDdma *const instance)
{
    FASSERT(instance);
    FDdma *ctrl = &instance->ctrl;
    FDdmaConfig *config = &ctrl->config;
    FError err = FT_SUCCESS;

    if (FT_COMPONENT_IS_READY != ctrl->is_ready)
    {
        FDDMA_ERROR("ddma-%d not yet init!!!", ctrl->config.id);
        return FFREERTOS_DDMA_NOT_INIT;
    }

    /* no scheduler during deinit */
    taskENTER_CRITICAL();

    FASSERT_MSG(NULL != instance->locker, "Locker not exists!!!");

    /* disable ddma irq */
    FDdmaStop(ctrl);

    FDdmaDeInitialization(ctrl);

    vSemaphoreDelete(instance->locker);
    instance->locker = NULL;

    taskEXIT_CRITICAL(); /* allow schedule after deinit */

    memset(instance, 0U, sizeof(*instance));

    return err;
}

/**
 * @name: FFreeRTOSDdmaSetupChannel
 * @msg: setup ddma channel before transfer
 * @return {FError} FT_SUCCESS if setup success
 * @param {FFreeRTOSDdma} *instance, freertos ddma instance
 * @param {FFreeRTOSRequest} *request, ddma request
 */
FError FFreeRTOSDdmaSetupChannel(FFreeRTOSDdma *const instance, u32 chan_id, const FFreeRTOSRequest *request)
{
    FASSERT(instance && request);
    FASSERT(chan_id < FDDMA_NUM_OF_CHAN);
    FDdma *ctrl = &instance->ctrl;
    FDdmaConfig *config = &ctrl->config;
    FDdmaChan *chan = &(instance->chan[chan_id]);
    FDdmaChanConfig *chan_config = &(chan->config);
    FError err = FT_SUCCESS;

    err = FDdmaOsTakeSema(instance->locker);
    if (FFREERTOS_DDMA_OK != err)
    {
        return err;
    }

    /* parpare channel configs */
    chan_config->id = chan_id;
    chan_config->slave_id = request->slave_id;
    FASSERT_MSG((0 != request->mem_addr), "Invaild memory address.");
    chan_config->ddr_addr = request->mem_addr;
    chan_config->dev_addr = request->dev_addr;
    chan_config->req_mode = request->is_rx ? FDDMA_CHAN_REQ_RX : FDDMA_CHAN_REQ_TX;
    chan_config->timeout = 0xffff;
    chan_config->trans_len = request->trans_len;

    FDDMA_INFO("channel: %d, slave id: %d, ddr: 0x%x, dev: 0x%x, req mode: %s, trans len: %d",
               chan_config->id,
               chan_config->slave_id,
               chan_config->ddr_addr,
               chan_config->dev_addr,
               (FDDMA_CHAN_REQ_TX == chan_config->req_mode) ? "mem=>dev" : "dev=>mem",
               chan_config->trans_len);

    /* allocate channel */
    err = FDdmaAllocateChan(ctrl, chan, chan_config);
    if (FDDMA_SUCCESS != err)
    {
        FDDMA_ERROR("Channel bind failed: 0x%x", err);
        goto err_exit;
    }

    FDdmaRegisterChanEvtHandler(chan, FDDMA_CHAN_EVT_REQ_DONE,
                                request->req_done_handler, request->req_done_args);

err_exit:
    FDdmaOsGiveSema(instance->locker);
    return err;
}

/**
 * @name: FFreeRTOSDdmaRevokeChannel
 * @msg: revoke channel setup
 * @return {FError} FT_SUCCESS if revoke channel setting success
 * @param {FFreeRTOSDdma} *instance, freertos ddma instance
 * @param {u32} chan_id, id of ddma channel
 */
FError FFreeRTOSDdmaRevokeChannel(FFreeRTOSDdma *const instance, u32 chan_id)
{
    FASSERT(instance);
    FASSERT(chan_id < FDDMA_NUM_OF_CHAN);
    FDdma *ctrl = &instance->ctrl;
    FDdmaChan *chan = &(instance->chan[chan_id]);
    FError err = FT_SUCCESS;

    err = FDdmaOsTakeSema(instance->locker);
    if (FFREERTOS_DDMA_OK != err)
    {
        return err;
    }

    err = FDdmaDeactiveChan(chan); /* deactive channel in use */
    if (FDDMA_SUCCESS != err)
    {
        FDDMA_ERROR("Channel deactive failed: 0x%x", err);
        goto err_exit;
    }

    err = FDdmaDellocateChan(chan); /* free channel resource */
    if (FDDMA_SUCCESS != err)
    {
        FDDMA_ERROR("Channel deallocate failed: 0x%x", err);
        goto err_exit;
    }

err_exit:
    FDdmaOsGiveSema(instance->locker);
    return err;
}

/**
 * @name: FFreeRTOSDdmaStartChannel
 * @msg: start dma transfer
 * @return {FError} FT_SUCCESS if start success
 * @param {FFreeRTOSDdma} *instance, freertos ddma instance
 */
FError FFreeRTOSDdmaStartChannel(FFreeRTOSDdma *const instance, u32 chan_id)
{
    FASSERT(instance);
    FDdma *ctrl = &instance->ctrl;
    FDdmaConfig *config = &ctrl->config;
    FError err = FT_SUCCESS;
    FDdmaChan *chan;

    err = FDdmaOsTakeSema(instance->locker);
    if (FFREERTOS_DDMA_OK != err)
    {
        return err;
    }

    /* active all channel allocated */
    chan = ctrl->chan[chan_id];
    if (NULL != chan) /* skip if channel not in use */
    {
        err = FDdmaActiveChan(chan); /* active channel if in use */
        if (FDDMA_SUCCESS != err)
        {
            FDDMA_ERROR("Channel start failed: 0x%x", err);
            goto err_exit;
        }
    }

    err = FDdmaStart(ctrl); /* start ddma controller */
    if (FDDMA_SUCCESS != err)
    {
        FDDMA_ERROR("Start ddma failed: 0x%x", err);
        goto err_exit;
    }

err_exit:
    FDdmaOsGiveSema(instance->locker);
    return err;
}

FError FFreeRTOSDdmaStopChannel(FFreeRTOSDdma *const instance, u32 chan_id)
{
    FASSERT(instance);
    FDdma *ctrl = &instance->ctrl;
    FDdmaConfig *config = &ctrl->config;
    FError err = FT_SUCCESS;
    FDdmaChan *chan;

    err = FDdmaOsTakeSema(instance->locker);
    if (FFREERTOS_DDMA_OK != err)
    {
        return err;
    }

    /* deactive all channel allocated */
    chan = ctrl->chan[chan_id];
    if (NULL != chan) /* skip if channel not in use */
    {
        err = FDdmaDeactiveChan(chan); /* deactive channel if in use */
        if (FDDMA_SUCCESS != err)
        {
            FDDMA_ERROR("Channel start failed: 0x%x", err);
            goto err_exit;
        }
    }

err_exit:
    FDdmaOsGiveSema(instance->locker);
    return err;
}

/**
 * @name: FFreeRTOSDdmaStop
 * @msg: stop current dma transfer
 * @return {FError} FT_SUCCESS if stop success
 * @param {FFreeRTOSDdma} *instance, freertos ddma instance
 */
FError FFreeRTOSDdmaStop(FFreeRTOSDdma *const instance)
{
    FASSERT(instance);
    FDdma *ctrl = &instance->ctrl;
    FDdmaConfig *config = &ctrl->config;
    FError err = FT_SUCCESS;
    FDdmaChan *chan;
    u32 chan_id;

    err = FDdmaOsTakeSema(instance->locker);
    if (FFREERTOS_DDMA_OK != err)
    {
        return err;
    }

    err = FDdmaStop(ctrl); /* stop ddma controller */
    if (FDDMA_SUCCESS != err)
    {
        FDDMA_ERROR("Stop ddma failed: 0x%x", err);
        goto err_exit;
    }

err_exit:
    FDdmaOsGiveSema(instance->locker);
    return err;
}