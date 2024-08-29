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
 * FilePath: fi2c_os.c
 * Date: 2022-07-15 10:43:29
 * LastEditTime: 2022-07-15 10:43:29
 * Description:  This file is for required function implementations of i2c driver used in FreeRTOS.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 liushengming 2022/11/25  first commit
 */

#include <stdio.h>
#include <string.h>
#include "fi2c_os.h"
#include "finterrupt.h"
#include "ftypes.h"
#include "sdkconfig.h"
#include "fcpu_info.h"
#include "fparameters.h"
#include "fi2c.h"
#include "fi2c_hw.h"
#include "fdebug.h"
#include "fio_mux.h"

#include "fmio_hw.h"
#include "fmio.h"

static FMioCtrl i2c_master;
static FMioCtrl i2c_slave;
static FFreeRTOSI2c os_i2c[FMIO_NUM] = {0};

/* virtual eeprom memory */

/**
 * @name: FI2cOsSetupInterrupt
 * @msg: 设置i2c中断
 * @return {*}
 * @param {FI2c} *pctrl
 */
static void FI2cOsSetupInterrupt(FI2c *pctrl)
{
    FASSERT(pctrl);
    FI2cConfig *pconfig = &pctrl->config;
    u32 cpu_id;
    FError err = FREERTOS_I2C_SUCCESS;

    GetCpuId(&cpu_id);
    vPrintf("cpu_id is %d \r\n", cpu_id);
    InterruptSetTargetCpus(pconfig->irq_num, cpu_id);

    /* interrupt init */
    /* umask i2c irq */
    InterruptSetPriority(pconfig->irq_num, pconfig->irq_prority);
    /* enable irq */
    InterruptUmask(pconfig->irq_num);
}

static void FI2cOsResetInterrupt(FI2c *pctrl)
{
    FASSERT(pctrl);
    /* disable irq */
    InterruptMask(pctrl->config.irq_num);
}

/**
 * @name: FFreeRTOSI2cInit
 * @msg: init freeRTOS i2c instance
 * @return {FFreeRTOSI2c *}pointer to os i2c instance
 * @param {u32} instance_id,i2c instance_id
 */
FFreeRTOSI2c *FFreeRTOSI2cInit(u32 instance_id, u32 work_mode, u32 slave_address, u32 speed_rate)
{
    FASSERT((os_i2c[instance_id].wr_semaphore = xSemaphoreCreateMutex()) != NULL);
    FASSERT((os_i2c[instance_id].trx_event = xEventGroupCreate()) != NULL);

    FError err = FREERTOS_I2C_SUCCESS;

    FI2cConfig i2c_config;

    /* E2000 use MIO -> I2C */
    FASSERT(instance_id < FMIO_NUM);

    if (FT_COMPONENT_IS_READY == os_i2c[instance_id].i2c_device.is_ready)
    {
        vPrintf("I2c device %d is already initialized.\r\n", instance_id);
        return NULL;
    }

    const FMioConfig *mio_config_p ;
    FMioCtrl *pctrl;
    if (work_mode == FI2C_MASTER)
    {
        pctrl = &i2c_master;
    }
    else
    {
        pctrl = &i2c_slave;
    }

    i2c_config = *FI2cLookupConfig(0);
    /* Setup iomux */
    mio_config_p = FMioLookupConfig(instance_id);
    if (NULL == mio_config_p)
    {
        vPrintf("Config of mio_i2c instance %d non found.\r\n", instance_id);
        return NULL;
    }


    pctrl->config = *mio_config_p;

    err = FMioFuncInit(pctrl, FMIO_FUNC_SET_I2C);
    if (err != FREERTOS_I2C_SUCCESS)
    {
        vPrintf("Mio I2c initialize is error.\r\n ");
        return NULL;
    }

    /* Modify configuration */
    i2c_config.work_mode = work_mode;
    i2c_config.slave_addr = slave_address;
    i2c_config.speed_rate = speed_rate;

    if (work_mode == FI2C_MASTER)/* 主机中断优先级低于从机接收 */
    {
        i2c_config.instance_id = i2c_master.config.instance_id;
        i2c_config.base_addr = i2c_master.config.func_base_addr;
        i2c_config.irq_num = i2c_master.config.irq_num;
        i2c_config.irq_prority = I2C_MASTER_IRQ_PRORITY;
    }
    else
    {
        i2c_config.instance_id = i2c_slave.config.instance_id;
        i2c_config.base_addr = i2c_slave.config.func_base_addr;
        i2c_config.irq_num = i2c_slave.config.irq_num;
        i2c_config.irq_prority = I2C_SLAVE_IRQ_PRORITY;
    }
    FIOPadSetMioMux(i2c_config.instance_id);

    err = FI2cCfgInitialize(&os_i2c[instance_id].i2c_device, &i2c_config);
    if (err != FREERTOS_I2C_SUCCESS)
    {
        vPrintf("I2c Init failed.\r\n");
        return NULL;
    }
    /* 从机模式，开中断接收数据 */
    if (work_mode == FI2C_SLAVE)
    {
        FI2cOsSetupInterrupt(&os_i2c[instance_id].i2c_device);
    }
    return (&os_i2c[instance_id]);
}

/**
 * @name: FFreeRTOSI2cDeinit
 * @msg: deinit freeRTOS i2c instance, include deinit i2c and delete mutex semaphore
 * @return {*}无
 * @param {FFreeRTOSI2c} *os_i2c_p,pointer to os i2c instance
 */
void FFreeRTOSI2cDeinit(FFreeRTOSI2c *os_i2c_p)
{
    FASSERT(os_i2c_p);
    FASSERT(os_i2c_p->wr_semaphore != NULL);
    FMioCtrl *pctrl = &i2c_master;
    FMioFuncDeinit(pctrl);
    /* 避免没有关闭中断，存在触发 */
    InterruptMask(os_i2c_p->i2c_device.config.irq_num);
    FI2cDeInitialize(&os_i2c_p->i2c_device);

    FASSERT_MSG(NULL != os_i2c_p->wr_semaphore, "Semaphore not exists!!!");
    vSemaphoreDelete(os_i2c_p->wr_semaphore);
    os_i2c_p->wr_semaphore = NULL;

    FASSERT_MSG(NULL != os_i2c_p->trx_event, "Event group not exists!!!");
    vEventGroupDelete(os_i2c_p->trx_event);
    os_i2c_p->trx_event = NULL;
}

/**
 * @name: FFreeRTOSI2cTransfer
 * @msg: tranfer i2c mesage
 * @return {*}
 * @param {u32} instance_id
 * @param {FFreeRTOSI2cMessage} *message
 * @param {u8} mode
 */
FError FFreeRTOSI2cTransfer(FFreeRTOSI2c *os_i2c_p, FFreeRTOSI2cMessage *message)
{
    FASSERT(os_i2c_p);
    FASSERT(message);
    FASSERT(os_i2c_p->wr_semaphore != NULL);
    FASSERT(message->mode < FI2C_READ_DATA_MODE_NUM);

    if (pdFALSE == xSemaphoreTake(os_i2c_p->wr_semaphore, portMAX_DELAY))
    {
        vPrintf("I2c xSemaphoreTake failed.\r\n");
        return FREERTOS_I2C_MESG_ERROR;
    }

    FError ret = FREERTOS_I2C_SUCCESS;
    FI2c *instance_p = &os_i2c_p->i2c_device;
    EventBits_t ev;

    /* Judge whether the slave address has changed */
    if (instance_p->config.slave_addr != message->slave_addr)
    {
        instance_p->config.slave_addr = message->slave_addr;
    }

    if (message->mode == FI2C_READ_DATA_POLL)
    {
        memset(message->buf, 0, message->buf_length);
        ret = FI2cMasterReadPoll(instance_p, message->mem_addr, message->mem_byte_len, message->buf, message->buf_length);
    }
    else if (message->mode == FI2C_READ_DATA_INTR)
    {
        FI2cOsSetupInterrupt(instance_p);
        memset(message->buf, 0, message->buf_length);
        ret = FI2cMasterReadIntr(instance_p, message->mem_addr, message->mem_byte_len, message->buf, message->buf_length);
        ev = xEventGroupWaitBits(os_i2c_p->trx_event, RTOS_I2C_TRANS_ABORTED | RTOS_I2C_READ_DONE, pdTRUE, pdFALSE, portMAX_DELAY);
        if (ev & RTOS_I2C_TRANS_ABORTED)
        {
            vPrintf("RTOS_I2C_TRANS_ABORTED ");
            ret = FREERTOS_I2C_TASK_ERROR;
        }
        else if (ev & RTOS_I2C_READ_DONE)
        {
            vPrintf("RTOS_I2C_READ_DONE,data_lenth:0d%d.\r\n", message->buf_length);
        }
        FI2cOsResetInterrupt(instance_p);
    }
    else if (message->mode == FI2C_WRITE_DATA_POLL)
    {
        ret = FI2cMasterWritePoll(instance_p, message->mem_addr, message->mem_byte_len, message->buf, message->buf_length);
    }
    else if (message->mode == FI2C_WRITE_DATA_INTR)
    {
        FI2cOsSetupInterrupt(instance_p);
        ret = FI2cMasterWriteIntr(instance_p, message->mem_addr, message->mem_byte_len, message->buf, message->buf_length);
        /*   wait intr is finish */
        ev = xEventGroupWaitBits(os_i2c_p->trx_event, RTOS_I2C_TRANS_ABORTED | RTOS_I2C_WRITE_DONE, pdTRUE, pdFALSE, portMAX_DELAY);
        if (ev & RTOS_I2C_TRANS_ABORTED)
        {
            vPrintf("RTOS_I2C_TRANS_ABORTED ");
            ret = FREERTOS_I2C_TASK_ERROR;
        }
        else if (ev & RTOS_I2C_WRITE_DONE)
        {
            vPrintf("RTOS_I2C_WRITE_DONE,data_lenth:0d%d.\r\n", message->buf_length);
        }
        FI2cOsResetInterrupt(instance_p);
    }

    /* Enable next transfer. Current one is finished */
    if (pdFALSE == xSemaphoreGive(os_i2c_p->wr_semaphore))
    {
        /* We could not post the semaphore, exit with error */
        vPrintf("FFreeRTOSI2cTransfer function xSemaphoreGive failed.\r\n");
        return FREERTOS_I2C_MESG_ERROR;
    }

    if (ret != FREERTOS_I2C_SUCCESS)
    {
        vPrintf("FFreeRTOSI2cTransfer error,id:%d.\r\n", instance_p->config.instance_id);
        return FREERTOS_I2C_TASK_ERROR;
    }
    return ret;
}

