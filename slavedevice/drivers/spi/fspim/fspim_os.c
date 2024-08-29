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
 * FilePath: fspim_os.c
 * Date: 2022-07-18 09:05:53
 * LastEditTime: 2022-07-18 09:05:53
 * Description:  This file is for required function implementations of spi master driver used in FreeRTOS.
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
#include "fparameters.h"
#include "fdebug.h"
#include "fassert.h"
#include "fsleep.h"
#include "fcpu_info.h"
#include "fio_mux.h"

#include "fspim_os.h"
#include "fspim_hw.h"
/************************** Constant Definitions *****************************/
#define FSPIM_OS_MAX_SPEED          12000000U

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static FFreeRTOSSpim spim[FSPI_NUM];

/***************** Macros (Inline Functions) Definitions *********************/
#define FSPIM_DEBUG_TAG "FSPIM-OS"
#define FSPIM_ERROR(format, ...) FT_DEBUG_PRINT_E(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_WARN(format, ...)  FT_DEBUG_PRINT_W(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_INFO(format, ...)  FT_DEBUG_PRINT_I(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_DEBUG(format, ...) FT_DEBUG_PRINT_D(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/
static void FSpimOsAckTransDone(void *instance_p, void *param);

/*****************************************************************************/

static inline FError FSpimOsTakeSema(SemaphoreHandle_t locker)
{
    FASSERT_MSG((NULL != locker), "Locker not exists.");
    if (pdFALSE == xSemaphoreTake(locker, portMAX_DELAY))
    {
        FSPIM_ERROR("Failed to give locker!!!");
        return FFREERTOS_SPIM_SEMA_ERR;
    }

    return FFREERTOS_SPIM_OK;
}

static inline void FSpimOsGiveSema(SemaphoreHandle_t locker)
{
    FASSERT_MSG((NULL != locker), "Locker not exists.");
    if (pdFALSE == xSemaphoreGive(locker))
    {
        FSPIM_ERROR("Failed to give locker!!!");
    }

    return;
}

static void FSpimOSSetupInterrupt(FSpim *ctrl)
{
    FSpimConfig *config_p = &ctrl->config;
    uintptr base_addr = config_p->base_addr;
    u32 evt;
    u32 mask;
    u32 cpu_id = 0;

    GetCpuId(&cpu_id);
    FSPIM_INFO("cpu_id is cpu_id %d", cpu_id);
    InterruptSetTargetCpus(config_p->irq_num, cpu_id);

    InterruptSetPriority(config_p->irq_num, config_p->irq_prority);

    /* register intr callback */
    InterruptInstall(config_p->irq_num,
                     FSpimInterruptHandler,
                     ctrl,
                     NULL);

    /* disable spi irq */
    FSpimMaskIrq(base_addr, FSPIM_IMR_ALL_BITS);

    /* enable irq */
    InterruptUmask(config_p->irq_num);

    return;
}

/**
 * @name: FFreeRTOSSpimInit
 * @msg: init and get spi instance
 * @return {FFreeRTOSSpim *} return
 * @param {u32} id, spim instance id
 * @param {FFreeRTOSSpimConifg} *input_config, freertos spim config
 */
FFreeRTOSSpim *FFreeRTOSSpimInit(u32 id, const FFreeRTOSSpimConifg *input_config)
{
    FASSERT(input_config);
    FASSERT_MSG(id < FSPI_NUM, "Invalid spim id.");
    FFreeRTOSSpim *instance = &spim[id];
    FSpim *ctrl = &instance->ctrl;
    FSpimConfig config;
    FError err = FFREERTOS_SPIM_OK;

    if (FT_COMPONENT_IS_READY == ctrl->is_ready)
    {
        FSPIM_ERROR("spi-%d already init.", id);
        return instance;
    }

    /* no scheduler during init */
    taskENTER_CRITICAL();

    instance->config = *input_config;
    config = *FSpimLookupConfig(id);
    config.slave_dev_id = FSPIM_SLAVE_DEV_0;

    if (FFREERTOS_SPIM_MODE_0 == instance->config.spi_mode) /* mode 0 */
    {
        config.cpha = FSPIM_CPHA_1_EDGE;
        config.cpol = FSPIM_CPOL_LOW;
    }
    else if (FFREERTOS_SPIM_MODE_1 == instance->config.spi_mode) /* mode 1 */
    {
        config.cpha = FSPIM_CPHA_2_EDGE;
        config.cpol = FSPIM_CPOL_LOW;
    }
    else if (FFREERTOS_SPIM_MODE_2 == instance->config.spi_mode) /* mode 2 */
    {
        config.cpha = FSPIM_CPHA_1_EDGE;
        config.cpol = FSPIM_CPOL_HIGH;
    }
    else if (FFREERTOS_SPIM_MODE_3 == instance->config.spi_mode) /* mode 3 */
    {
        config.cpha = FSPIM_CPHA_2_EDGE;
        config.cpol = FSPIM_CPOL_HIGH;
    }

    config.n_bytes = FSPIM_1_BYTE;
    config.en_test = instance->config.inner_loopback;
    config.en_dma = instance->config.en_dma;
    config.irq_prority = FFREERTOS_SPIM_IRQ_PRIORITY;
    config.max_freq_hz = FSPIM_OS_MAX_SPEED;

    FIOPadSetSpimMux(id);

    FSPIM_INFO("init spi-%d @ 0x%x", config.instance_id, config.base_addr);
    err = FSpimCfgInitialize(ctrl, &config);
    if (FSPIM_SUCCESS != err)
    {
        FSPIM_ERROR("Init spim-%d failed, err: 0x%x!!!", id, err);
        goto err_exit;
    }

    FASSERT_MSG(NULL == instance->locker, "Locker exists!!!");
    FASSERT_MSG((instance->locker = xSemaphoreCreateMutex()) != NULL, "Create mutex failed!!!");

    FASSERT_MSG(NULL == instance->evt, "Event group exists!!!");
    FASSERT_MSG((instance->evt = xEventGroupCreate()) != NULL, "Create event group failed!!!");

    FSpimOSSetupInterrupt(ctrl);
    FSpimRegisterIntrruptHandler(ctrl, FSPIM_INTR_EVT_RX_DONE, FSpimOsAckTransDone, instance);

    FSPIM_INFO("Init spi-%d success!!!", id);

err_exit:
    taskEXIT_CRITICAL(); /* allow schedule after init */
    return (FSPIM_SUCCESS == err) ? instance : NULL; /* exit with NULL if failed */
}

/**
 * @name: FFreeRTOSSpimDeInit
 * @msg: deinit spi instance
 * @return {FError} return FFREERTOS_SPIM_OK
 * @param {FFreeRTOSSpim} *instance, spi instance
 */
FError FFreeRTOSSpimDeInit(FFreeRTOSSpim *const instance)
{
    FASSERT(instance);
    FSpim *ctrl = &instance->ctrl;
    FError err = FFREERTOS_SPIM_OK;

    if (FT_COMPONENT_IS_READY != ctrl->is_ready)
    {
        FSPIM_ERROR("ddma-%d already init.");
        return FFREERTOS_SPIM_NOT_INIT;
    }

    /* no scheduler during deinit */
    taskENTER_CRITICAL();

    FSpimDeInitialize(ctrl);

    FASSERT_MSG(NULL != instance->locker, "Locker not exists!!!");
    vSemaphoreDelete(instance->locker);
    instance->locker = NULL;

    FASSERT_MSG(NULL != instance->evt, "Event group not exists!!!");
    vEventGroupDelete(instance->evt);
    instance->evt = NULL;

    taskEXIT_CRITICAL(); /* allow schedule after deinit */

    return err;
}

/* ack transfer finish from interrupt */
static void FSpimOsAckTransDone(void *instance_p, void *param)
{
    FASSERT(param);
    FFreeRTOSSpim *instance = (FFreeRTOSSpim *)param;
    BaseType_t xhigher_priority_task_woken = pdFALSE;
    BaseType_t x_result = pdFALSE;

    FSPIM_DEBUG("ack sfud trans done");
    FASSERT(instance->evt);
    x_result = xEventGroupSetBitsFromISR(instance->evt, FFREERTOS_TRANS_DONE,
                                         &xhigher_priority_task_woken);

    return;
}

/* wait transfer finish ack from task */
static FError FSpimOsWaitTransDone(FFreeRTOSSpim *const instance)
{
    const TickType_t wait_delay = pdMS_TO_TICKS(5000UL); /* wait for 5 seconds */
    EventBits_t ev;
    FError err = FFREERTOS_SPIM_OK;

    /* block task to wait finish signal */
    FASSERT_MSG(instance->evt, "Evt not exists.");
    ev = xEventGroupWaitBits(instance->evt, FFREERTOS_TRANS_DONE,
                             pdTRUE, pdFALSE, wait_delay); /* wait for trans done */
    if ((ev & FFREERTOS_TRANS_DONE))
    {
        FSPIM_DEBUG("Trans done.");
    }
    else
    {
        FSPIM_ERROR("Trans timeout.");
        err = FFREERTOS_SPIM_WAIT_EVT_TIMOUT;
    }

    return err;
}

/* send bytes on spi */
static FError FSpimOsTx(FFreeRTOSSpim *const instance, const u8 *tx_buf, fsize_t tx_len)
{
    FSpim *ctrl = &instance->ctrl;
    FError err = FFREERTOS_SPIM_OK;

    err = FSpimTransferByInterrupt(ctrl, tx_buf, NULL, tx_len); /* start transfer */
    if (FSPIM_SUCCESS != err)
    {
        FSPIM_ERROR("Spim transfer failed: 0x%x", err);
    }
    else
    {
        /* wait transfer completed */
        err = FSpimOsWaitTransDone(instance);
    }

    return err;
}

/* receive bytes on spi */
static FError FSpimOsRx(FFreeRTOSSpim *const instance, u8 *rx_buf, fsize_t rx_len)
{
    FSpim *ctrl = &instance->ctrl;
    FError err = FFREERTOS_SPIM_OK;

    err = FSpimTransferByInterrupt(ctrl, NULL, rx_buf, rx_len); /* start transfer */
    if (FSPIM_SUCCESS != err)
    {
        FSPIM_ERROR("Spim transfer failed: 0x%x", err);
    }
    else
    {
        /* wait transfer completed */
        err = FSpimOsWaitTransDone(instance);
    }

    return err;
}

/**
 * @name: FFreeRTOSSpimTransfer
 * @msg: start spim transfer in Non-DMA/DMA mode
 * @return {*}
 * @param {FFreeRTOSSpim} *instance, freertos spim instance
 * @param {FFreeRTOSSpiMessage} *message, spim transfer message
 */
FError FFreeRTOSSpimTransfer(FFreeRTOSSpim *const instance, const FFreeRTOSSpiMessage *message)
{
    FASSERT(instance);
    FASSERT(message);
    FSpim *ctrl = &instance->ctrl;
    FError err = FFREERTOS_SPIM_OK;

    err = FSpimOsTakeSema(instance->locker); /* in case other tasks try to do transfer */
    if (FFREERTOS_SPIM_OK != err)
    {
        return err;
    }

    FSpimSetChipSelection(ctrl, TRUE); /* toggle on chip selection */
    FSPIM_INFO("tx_buf@%p, len: %d, rx_buf@%p, len: %d",
               message->tx_buf, message->tx_len, message->rx_buf, message->rx_len);

    if (instance->config.en_dma) /* dma-mode */
    {
        FSPIM_INFO("Start DMA tx: %d, rx: %d", message->tx_len, message->rx_len);
        err = FSpimTransferDMA(ctrl, (0U != message->tx_len), (0U != message->rx_len));
        if (FSPIM_SUCCESS != err)
        {
            FSPIM_ERROR("Spim DMA transfer failed: 0x%x", err);
        }

    }
    else /* no-dma mode */
    {
        if (message->tx_buf)
        {
            err = FSpimOsTx(instance, message->tx_buf, message->tx_len);
        }

        if (message->rx_buf)
        {
            if (FFREERTOS_SPIM_OK == err)
            {
                err = FSpimOsRx(instance, message->rx_buf, message->rx_len);
            }
        }
    }

    FSpimSetChipSelection(ctrl, FALSE); /* toggle off chip selection */

err_exit:
    FSpimOsGiveSema(instance->locker);
    return err;
}