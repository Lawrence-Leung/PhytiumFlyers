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
 * FilePath: i2c_ms_example.c
 * Date: 2022-11-10 11:35:23
 * LastEditTime: 2022-11-10 11:35:24
 * Description:  This file is for i2c master and slave test example functions.
 *
 * Modify History:
 *  Ver       Who            Date                 Changes
 * -----    ------         --------     --------------------------------------
 *  1.0    liushengming   2023/09/25             init commit
 */
#include <string.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "fi2c.h"
#include "fi2c_hw.h"
#include "fi2c_os.h"
#include "timers.h"
#include "fcpu_info.h"
#include "i2c_example.h"
#include "fparameters.h"
#include "fio_mux.h"
#include "sdkconfig.h"
#include "fdebug.h"
#include "ftypes.h"
#include "finterrupt.h"

/* write and read task delay in milliseconds */
#define TASK_DELAY_MS   1000UL

/* slave address */
/* Notice! Using addresses above 0x50 may cause the loopback test to fail  */
#define MASTER_SLAVE_ADDR   0x01

#define DAT_LENGTH  15
static char data_w[DAT_LENGTH] = {0};
static char data_r0[DAT_LENGTH];
static char data_r1[DAT_LENGTH];

static xTaskHandle init_handle;
static xTaskHandle read_handle;
static xTaskHandle write_handle;
static xTaskHandle slave_handle;

static FFreeRTOSI2c *os_i2c_master;
static FFreeRTOSI2c *os_i2c_slave;

typedef struct data
{
    boolean first_write;/*IIC首次写入，在初始化时置位，用来指示当前传输的首个字节数据是用户需要读写的地址偏移*/
    u32 buff_idx;/* PC 指向的地址偏移 */
    u8 buff[IO_BUF_LEN];/*虚拟内存块*/
} FI2cSlaveData;

/* Slave mode for virtual eeprom memory ，size: IO_BUF_LEN in fi2c_os.h*/
static FI2cSlaveData slave;

/**
 * @name: FFreeRTOSI2cSlaveDump
 * @msg: dump buffer of slave
 * @return {*}
 * @param {FFreeRTOSI2c} *os_i2c_p
 */
void FFreeRTOSI2cSlaveDump(FFreeRTOSI2c *os_i2c_p)
{
    FASSERT(os_i2c_p);
    FASSERT(os_i2c_p->wr_semaphore != NULL);
    FI2cSlaveData *slave_p = &slave;
    FtDumpHexByte(slave_p->buff, IO_BUF_LEN);
}

/**
 * @name: FI2cSlaveCb
 * @msg: 从机内存操作
 * @return {*},无
 * @param {void} *instance_p
 * @param {void} *para
 * @param {u32} evt
 */
void FI2cOsSlaveCb(void *instance_p, void *para, u32 evt)
{
    FI2cSlaveData *slave_p = &slave;
    u8 *val = (u8 *)para;
    /*
    *Do not increment buffer_idx here,because we set maximum lenth is IO_BUF_LEN
    */
    if (slave_p->buff_idx >= IO_BUF_LEN)
    {
        slave_p->buff_idx = slave_p->buff_idx % IO_BUF_LEN;
    }
    switch (evt)
    {
        case FI2C_EVT_SLAVE_WRITE_RECEIVED:
            if (slave_p->first_write)
            {
                slave_p->buff_idx = *val;
                slave_p->first_write = FALSE;
            }
            else
            {
                slave_p->buff[slave_p->buff_idx++] = *val;
            }

            break;
        case FI2C_EVT_SLAVE_READ_PROCESSED:
            /* The previous byte made it to the bus, get next one */
            slave_p->buff_idx++;
            /* fallthrough */
            break;
        case FI2C_EVT_SLAVE_READ_REQUESTED:
            *val = slave_p->buff[slave_p->buff_idx++];
            break;
        case FI2C_EVT_SLAVE_STOP:
        case FI2C_EVT_SLAVE_WRITE_REQUESTED:
            slave_p->first_write = TRUE;
            break;
        default:
            break;
    }

    return;
}

/**
 * @name: FI2cSlaveWriteReceived
 * @msg: Slave收到主机发送的数据，需要存下
 * @return {*} 无
 * @param {void} *instance_p
 * @param {void} *para
 */
void FI2cOsSlaveWriteReceived(void *instance_p, void *para)
{
    FI2cOsSlaveCb(instance_p, para, FI2C_EVT_SLAVE_WRITE_RECEIVED);
}

/**
 * @name: FI2cSlaveReadProcessed
 * @msg: 在Slave发送模式下，发送完数据的最后一个字节后，在规定时间内没有收到 Master 端的回应
 * @return {*} 无
 * @param {void} *instance_p
 * @param {void} *para
 */
void FI2cOsSlaveReadProcessed(void *instance_p, void *para)
{
    FI2cOsSlaveCb(instance_p, para, FI2C_EVT_SLAVE_READ_PROCESSED);
}

/**
 * @name: FI2cSlaveReadRequest
 * @msg: slave收到主机读取内容的请求
 * @return {*} 无
 * @param {void} *instance_p
 * @param {void} *para
 */
void FI2cOsSlaveReadRequest(void *instance_p, void *para)
{
    FI2cOsSlaveCb(instance_p, para, FI2C_EVT_SLAVE_READ_REQUESTED);
}

/**
 * @name: FI2cSlaveStop
 * @msg: I2C总线接口上是否产生了STOP。与控制器工作在Master模式还是 Slave 模式无关。
 * @return {*}
 * @param {void} *instance_p
 * @param {void} *para
 */
void FI2cOsSlaveStop(void *instance_p, void *para)
{
    FI2cOsSlaveCb(instance_p, para, FI2C_EVT_SLAVE_STOP);
}

/**
 * @name: FI2cSlaveWriteRequest
 * @msg: slave收到主机发送的写请求
 * @return {*}
 * @param {void} *instance_p
 * @param {void} *para
 */
void FI2cOsSlaveWriteRequest(void *instance_p, void *para)
{
    FI2cOsSlaveCb(instance_p, para, FI2C_EVT_SLAVE_WRITE_REQUESTED);
}

/*
 * @name: FI2cIntrTxDone
 * @msg:user transmit FIFO done interrupt callback.
 * @param {void} *instance_p
 */
static void FI2cIntrTxDonecallback(void *instance, void *param)
{
    BaseType_t x_result = pdFALSE;
    BaseType_t xhigher_priority_task_woken = pdFALSE;

    FI2c *instance_p = (FI2c *)instance;
    x_result = xEventGroupSetBitsFromISR(os_i2c_master[instance_p->config.instance_id].trx_event, RTOS_I2C_WRITE_DONE, &xhigher_priority_task_woken);
    if (x_result != pdFAIL)
    {
        portYIELD_FROM_ISR(xhigher_priority_task_woken);
    }
}

/*
 * @name: FI2cIntrRxDonecallback
 * @msg:user receive fifo level done interrupt callback.
 * @param {void} *instance_p
 */
static void FI2cIntrRxDonecallback(void *instance, void *param)
{
    BaseType_t x_result = pdFALSE;
    BaseType_t xhigher_priority_task_woken = pdFALSE;

    FI2c *instance_p = (FI2c *)instance;
    x_result = xEventGroupSetBitsFromISR(os_i2c_master[instance_p->config.instance_id].trx_event, RTOS_I2C_READ_DONE, &xhigher_priority_task_woken);
    if (x_result != pdFAIL)
    {
        portYIELD_FROM_ISR(xhigher_priority_task_woken);
    }
}

/*
 * @name: FI2cIntrTxAbrtcallback
 * @msg:user transmit abort interrupt callback.
 * @param {void} *instance_p
 */
static void FI2cIntrTxAbrtcallback(void *instance, void *param)
{
    BaseType_t x_result = pdFALSE;
    BaseType_t xhigher_priority_task_woken = pdFALSE;

    FI2c *instance_p = (FI2c *)instance;
    x_result = xEventGroupSetBitsFromISR(os_i2c_master[instance_p->config.instance_id].trx_event, RTOS_I2C_TRANS_ABORTED, &xhigher_priority_task_woken);
    if (x_result != pdFAIL)
    {
        portYIELD_FROM_ISR(xhigher_priority_task_woken);
    }
}

static void I2cSlaveTask(void *pvParameters)
{
    const char *pcTaskName = "\r\n*****I2cSlaveTask is running...\r\n";
    const TickType_t xDelay = pdMS_TO_TICKS(TASK_DELAY_MS);
    vTaskDelay(xDelay);
    FError ret = FREERTOS_I2C_SUCCESS;

    /* The FFreeRTOSI2c to use is passed in via the parameter.
    Cast this to a FFreeRTOSI2c pointer. */
    FFreeRTOSI2c *os_i2c_write_p = (FFreeRTOSI2c *) pvParameters;

    vPrintf(pcTaskName);
    /* 获取到信号，打印内存块 */
    FFreeRTOSI2cSlaveDump(os_i2c_write_p);
    vTaskDelete(NULL);
}


static void I2cReadTask(void *pvParameters)
{
    const TickType_t xDelay = pdMS_TO_TICKS(TASK_DELAY_MS);
    vTaskDelay(xDelay);
    const char *pcReadTaskName = "\r\n*****I2cReadTask is running...\r\n";
    vPrintf(pcReadTaskName);
    FError ret = FREERTOS_I2C_SUCCESS;

    /* Master mode for send or receive data */
    FFreeRTOSI2cMessage message;

    /* The FFreeRTOSI2c to use is passed in via the parameter.
    Cast this to a FFreeRTOSI2c pointer. */
    FFreeRTOSI2c *os_i2c_read_p = (FFreeRTOSI2c *) pvParameters;

    message.slave_addr = os_i2c_read_p->i2c_device.config.slave_addr;

    /*8位地址*/
    message.mem_byte_len = 1;
    message.mem_addr = 0x1;/* 地址偏移0x1的位置poll方式读取数据 */
    message.buf_length = DAT_LENGTH;
    message.buf = data_r0;
    message.mode = FI2C_READ_DATA_POLL;

    ret = FFreeRTOSI2cTransfer(os_i2c_read_p, &message);
    if (ret != FREERTOS_I2C_SUCCESS)
    {
        vPrintf("FFreeRTOSI2cTransfer read poll task error,i2c id:%d.\r\n", os_i2c_read_p->i2c_device.config.instance_id);
    }
    message.mem_addr = 0x31;/* 地址偏移0x35的位置poll方式读取数据 */
    message.buf = data_r1;
    message.buf_length = DAT_LENGTH;
    ret = FFreeRTOSI2cTransfer(os_i2c_read_p, &message);
    if (ret != FREERTOS_I2C_SUCCESS)
    {
        vPrintf("FFreeRTOSI2cTransfer read poll task error,i2c id:%d.\r\n", os_i2c_read_p->i2c_device.config.instance_id);
    }
    vPrintf("data_r0:\r\n");
    FtDumpHexByte(data_r0, DAT_LENGTH);
    vPrintf("data_r1:\r\n");
    FtDumpHexByte(data_r1, DAT_LENGTH);

    FFreeRTOSI2cDeinit(os_i2c_read_p);/*写入再读取完成后去初始化FFreeRTOSI2c主机设置*/
    printf("I2cReadTask is over.\r\n ");
    vTaskDelete(NULL);
}

static void I2cWriteTask(void *pvParameters)
{
    const char *pcWriteTaskName = "\r\n*****I2cWriteTask is running...\r\n";
    vPrintf(pcWriteTaskName);
    FError ret = FREERTOS_I2C_SUCCESS;
    u8 i;
    /* Master mode for send or receive data */
    FFreeRTOSI2cMessage message;

    /* The FFreeRTOSI2c to use is passed in via the parameter.
    Cast this to a FFreeRTOSI2c pointer. */
    FFreeRTOSI2c *os_i2c_write_p = (FFreeRTOSI2c *) pvParameters;

    message.slave_addr = os_i2c_write_p->i2c_device.config.slave_addr;
    for (i = 0; i < DAT_LENGTH; i++)
    {
        data_w[i] = i ;
    }
    /*8位地址*/
    message.mem_byte_len = 1;
    message.buf = data_w;
    message.buf_length = DAT_LENGTH;
    message.mem_addr = 0x01;/* 地址偏移0x1的位置poll方式写入数据 */
    message.mode = FI2C_WRITE_DATA_POLL;
    ret = FFreeRTOSI2cTransfer(os_i2c_write_p, &message);
    if (ret != FREERTOS_I2C_SUCCESS)
    {
        vPrintf("FFreeRTOSI2cTransfer write poll task error,i2c id:%d.\r\n", os_i2c_write_p->i2c_device.config.instance_id);
    }

    message.mem_addr = 0x31;/* 地址偏移0x35的位置poll方式写入数据 */
    ret = FFreeRTOSI2cTransfer(os_i2c_write_p, &message);
    if (ret != FREERTOS_I2C_SUCCESS)
    {
        vPrintf("FFreeRTOSI2cTransfer write poll task error,i2c id:%d.\r\n", os_i2c_write_p->i2c_device.config.instance_id);
    }
    printf("I2cWriteTask is over.\r\n ");
    vTaskDelete(NULL);
}

static FError FFreeRTOSI2cInitSet(uint32_t id, uint32_t work_mode, uint32_t slave_address)
{
    FError err;

    FIOPadSetMioMux(id);
    /* init i2c controller */
    if (work_mode == FI2C_MASTER) /* 主机初始化默认使用poll模式 */
    {
        os_i2c_master = FFreeRTOSI2cInit(id, work_mode, slave_address, FI2C_SPEED_STANDARD_RATE);
        /* register intr callback */
        InterruptInstall(os_i2c_master->i2c_device.config.irq_num, FI2cMasterIntrHandler, &os_i2c_master->i2c_device, "fi2cmaster");
        /* register intr handler func */
        FI2cMasterRegisterIntrHandler(&os_i2c_master->i2c_device, FI2C_EVT_MASTER_TRANS_ABORTED, FI2cIntrTxAbrtcallback);
        FI2cMasterRegisterIntrHandler(&os_i2c_master->i2c_device, FI2C_EVT_MASTER_READ_DONE, FI2cIntrRxDonecallback);
        FI2cMasterRegisterIntrHandler(&os_i2c_master->i2c_device, FI2C_EVT_MASTER_WRITE_DONE, FI2cIntrTxDonecallback);
    }
    else if (work_mode == FI2C_SLAVE)
    {
        os_i2c_slave = FFreeRTOSI2cInit(id, work_mode, slave_address, FI2C_SPEED_STANDARD_RATE);
        /* register intr callback */
        InterruptInstall(os_i2c_slave->i2c_device.config.irq_num, FI2cSlaveIntrHandler, &os_i2c_slave->i2c_device, "fi2cslave");
        /* slave mode intr set,must set before master data come in. */
        err = FI2cSlaveSetupIntr(&os_i2c_slave->i2c_device);
        if (err != FREERTOS_I2C_SUCCESS)
        {
            vPrintf("I2c slave intr init failed.\r\n");
            return FREERTOS_I2C_INVAL_STATE_ERROR;
        }
        FI2cSlaveData *slave_p = &slave;
        memset(slave_p, 0, sizeof(*slave_p));
        slave_p->first_write = TRUE;
        FI2cSlaveRegisterIntrHandler(&os_i2c_slave->i2c_device, FI2C_EVT_SLAVE_WRITE_RECEIVED, FI2cOsSlaveWriteReceived);
        FI2cSlaveRegisterIntrHandler(&os_i2c_slave->i2c_device, FI2C_EVT_SLAVE_READ_PROCESSED, FI2cOsSlaveReadProcessed);
        FI2cSlaveRegisterIntrHandler(&os_i2c_slave->i2c_device, FI2C_EVT_SLAVE_READ_REQUESTED, FI2cOsSlaveReadRequest);
        FI2cSlaveRegisterIntrHandler(&os_i2c_slave->i2c_device, FI2C_EVT_SLAVE_STOP, FI2cOsSlaveStop);
        FI2cSlaveRegisterIntrHandler(&os_i2c_slave->i2c_device, FI2C_EVT_SLAVE_WRITE_REQUESTED, FI2cOsSlaveWriteRequest);
    }
    return FREERTOS_I2C_SUCCESS;
}

static void I2cInitTask(void *pvParameters)
{
    FError err;
    BaseType_t xReturn = pdPASS;

    taskENTER_CRITICAL(); //进入临界区

    err = FFreeRTOSI2cInitSet(FMIO1_ID, FI2C_MASTER, MASTER_SLAVE_ADDR);
    if (err != FREERTOS_I2C_SUCCESS)
    {
        vPrintf("I2c FFreeRTOSI2cInitSet failed.\r\n");
        return;
    }

    err = FFreeRTOSI2cInitSet(FMIO2_ID, FI2C_SLAVE, MASTER_SLAVE_ADDR);
    if (err != FREERTOS_I2C_SUCCESS)
    {
        vPrintf("I2c FFreeRTOSI2cInitSet failed.\r\n");
        return;
    }

    xReturn = xTaskCreate((TaskFunction_t)I2cReadTask,  /* 任务入口函数 */
                          (const char *)"I2cReadTask",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          (void *)os_i2c_master,/* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 2, /* 任务的优先级 */
                          (TaskHandle_t *)&read_handle); /* 任务控制 */
    FASSERT_MSG(xReturn == pdPASS, "I2cReadTask creation is failed.");

    xReturn = xTaskCreate((TaskFunction_t)I2cWriteTask,  /* 任务入口函数 */
                          (const char *)"I2cWriteTask",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          (void *)os_i2c_master,/* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 1, /* 任务的优先级 */
                          (TaskHandle_t *)&write_handle); /* 任务控制 */
    FASSERT_MSG(xReturn == pdPASS, "I2cWriteTask creation is failed.");

     xReturn = xTaskCreate((TaskFunction_t)I2cSlaveTask,  /* 任务入口函数 */
                          (const char *)"I2cSlaveTask",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          (void *)os_i2c_slave,/* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 3, /* 任务的优先级 */
                          (TaskHandle_t *)&slave_handle); /* 任务控制 */
    FASSERT_MSG(xReturn == pdPASS, "I2cSlaveTask creation is failed.");

    taskEXIT_CRITICAL(); //退出临界区
    vTaskDelete(NULL);
}

BaseType_t FFreeRTOSI2cLoopbackCreate(void)
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */
    BaseType_t xTimerStarted = pdPASS;

    taskENTER_CRITICAL(); //进入临界区

    xReturn = xTaskCreate((TaskFunction_t)I2cInitTask,  /* 任务入口函数 */
                          (const char *)"I2cInitTask",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          (void *)NULL,/* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 1,  /* 任务的优先级 */
                          (TaskHandle_t *)&init_handle); /* 任务控制 */
    FASSERT_MSG(xReturn == pdPASS, "I2cInitTask creation is failed.");

    taskEXIT_CRITICAL(); //退出临界区
    printf("I2c task is created successfully.\r\n");
    vTaskDelay(TASK_DELAY_MS*3);
    return xReturn;
}




