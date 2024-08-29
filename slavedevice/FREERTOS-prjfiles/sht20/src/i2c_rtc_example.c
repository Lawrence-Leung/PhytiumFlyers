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
 * FilePath: i2c_example.c
 * Date: 2022-11-10 11:35:23
 * LastEditTime: 2022-11-10 11:35:24
 * Description:  This file is for i2c rtc test example functions.
 *
 * Modify History:
 *  Ver       Who            Date                 Changes
 * -----    ------         --------     --------------------------------------
 *  1.0    liushengming   2022/11/25             init commit
 *  1.0    liushengming   2023/09/25             rtc commit  
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

#if defined(CONFIG_E2000D_DEMO_BOARD) || defined(CONFIG_E2000Q_DEMO_BOARD) || defined(CONFIG_FIREFLY_DEMO_BOARD)
#define DS_1339_MIO FMIO9_ID
#else
#define DS_1339_MIO FMIO0_ID
#endif

/*
 * RTC register addresses
 */
#define DS1339_SEC_REG      0x0
#define DS1339_MIN_REG      0x1
#define DS1339_HOUR_REG     0x2
#define DS1339_DAY_REG      0x3
#define DS1339_DATE_REG     0x4
#define DS1339_MONTH_REG    0x5
#define DS1339_YEAR_REG     0x6

#define BCD_TO_BIN(bcd) (( ((((bcd)&0xf0)>>4)*10) + ((bcd)&0xf) ) & 0xff)
#define BIN_TO_BCD(bin) (( (((bin)/10)<<4) + ((bin)%10) ) & 0xff)

/* write and read task delay in milliseconds */
#define TASK_DELAY_MS   1000UL

/* rtc address */

#define RTC_ADDR            0x68

#define DAT_LENGTH  15

static char data_r0[DAT_LENGTH];


static xTaskHandle init_handle;
static xTaskHandle read_handle;
static xTaskHandle write_handle;


static FFreeRTOSI2c *os_i2c_master;

typedef struct
{
    u16 year;               /* year */
    u8 month;           /* month */
    u8 day_of_month;    /* day of month */
    u8 day_of_week;     /* day of week */
    u8 hour;                /* hour */
    u8 minute;          /* minute */
    u8 second;          /* second */
} FRtcDateTimer;

static FError DsRtcDateCheck(FRtcDateTimer *rtc_time)
{
    FASSERT(rtc_time != NULL);

    /* Check validity of seconds value */
    if (rtc_time->second > 59)
    {
        return FI2C_ERR_INVAL_PARM;
    }

    /* Check validity of minutes value */
    if (rtc_time->minute > 59)
    {
        return FI2C_ERR_INVAL_PARM;
    }

    /* Check validity of day of week value */
    if (rtc_time->day_of_week < 1 || rtc_time->day_of_week > 7)
    {
        return FI2C_ERR_INVAL_PARM;
    }

    /* Check validity of hours value */
    if (rtc_time->hour > 23)
    {
        return FI2C_ERR_INVAL_PARM;
    }

    /* Check validity of day of month value */
    if (rtc_time->day_of_month < 1 || rtc_time->day_of_month > 31)
    {
        return FI2C_ERR_INVAL_PARM;
    }

    /* Check validity of month value */
    if (rtc_time->month < 1 || rtc_time->month > 12)
    {
        return FI2C_ERR_INVAL_PARM;
    }

    /* Check validity of year value */
    if (rtc_time->year > 2099)
    {
        return FI2C_ERR_INVAL_PARM;
    }
    return FREERTOS_I2C_SUCCESS;
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
    message.mem_byte_len = 1;
    message.mem_addr = 0x0;/* 地址偏移0x0的位置poll方式读取数据 */
    message.buf_length = 7;
    message.buf = data_r0;
    message.mode = FI2C_READ_DATA_POLL;

    ret = FFreeRTOSI2cTransfer(os_i2c_read_p, &message);
    if (ret != FREERTOS_I2C_SUCCESS)
    {
        vPrintf("FFreeRTOSI2cTransfer read poll task error,i2c id:%d.\r\n", os_i2c_read_p->i2c_device.config.instance_id);
    }
    u16 year;
    if (data_r0[5] & 0x80)
    {
        year = BCD_TO_BIN(data_r0[6]) + 2000;
    }
    else
    {
        year = BCD_TO_BIN(data_r0[6]) + 1900;
    }
    printf("Date_time: %d-%d-%d week:%d time:%d:%d:%d\r\n",
           year,
           BCD_TO_BIN(data_r0[5] & 0x1F),
           BCD_TO_BIN(data_r0[4] & 0x3F),
           BCD_TO_BIN((data_r0[3] - 1) & 0x7),
           BCD_TO_BIN(data_r0[2] & 0x3F),
           BCD_TO_BIN(data_r0[1] & 0x7F),
           BCD_TO_BIN(data_r0[0] & 0x7F)
          );
    vTaskDelay(xDelay);
    FFreeRTOSI2cDeinit(os_i2c_read_p);/*写入再读取完成后去初始化FFreeRTOSI2c主机设置*/
    printf("I2cReadTask is over.\r\n ");
    vTaskDelete(NULL);
}

static void I2cWriteTask(void *pvParameters)
{
    const TickType_t xDelay = pdMS_TO_TICKS(TASK_DELAY_MS);
    vTaskDelay(xDelay);
    const char *pcWriteTaskName = "\r\n*****I2cWriteTask is running...\r\n";
    vPrintf(pcWriteTaskName);
    FError ret = FREERTOS_I2C_SUCCESS;
    u8 i,century;
    u8 data_buf[7] = {0};
    /* Master mode for send or receive data */
    FFreeRTOSI2cMessage message;

    /* The FFreeRTOSI2c to use is passed in via the parameter.
    Cast this to a FFreeRTOSI2c pointer. */
    FFreeRTOSI2c *os_i2c_write_p = (FFreeRTOSI2c *) pvParameters;

    message.slave_addr = os_i2c_write_p->i2c_device.config.slave_addr;
    FRtcDateTimer date_time = {2023, 9, 25, 1, 20, 13, 30};
    printf("Set:year: %d, month: %d, day: %d,week: %d, hour: %d, minute: %d, second: %d\r\n",
            date_time.year,
            date_time.month,
            date_time.day_of_month,
            date_time.day_of_week,
            date_time.hour,
            date_time.minute,
            date_time.second);
    ret = DsRtcDateCheck(&date_time);
    if (FREERTOS_I2C_SUCCESS != ret)
    {
        vPrintf("Time data param error.\r\n");
        return;
    }
    data_buf[0] = BIN_TO_BCD(date_time.second);

    data_buf[1] = BIN_TO_BCD(date_time.minute);

    data_buf[2] = BIN_TO_BCD(date_time.hour);

    data_buf[3] = BIN_TO_BCD(date_time.day_of_week + 1);

    data_buf[4] = BIN_TO_BCD(date_time.day_of_month);

    if (date_time.year >= 2000)
    {
        century = 0x80;
    }
    else
    {
        century = 0x0;
    }
    data_buf[5] = (BIN_TO_BCD(date_time.month) | century);

    data_buf[6] = BIN_TO_BCD(date_time.year % 100);

    /*8位地址*/
    message.mem_byte_len = 1;
    message.buf = &data_buf;
    message.buf_length = sizeof(data_buf);
    message.mem_addr = 0x0;/* 地址偏移0x0的位置poll方式写入数据 */
    message.mode = FI2C_WRITE_DATA_POLL;
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
    else
    {
        printf("error_work_mode!\r\n");
        return FREERTOS_I2C_INVAILD_PARAM_ERROR;
    }
    return FREERTOS_I2C_SUCCESS;
}

static void I2cInitTask(void *pvParameters)
{
    FError err;
    BaseType_t xReturn = pdPASS;

    taskENTER_CRITICAL(); //进入临界区

    err = FFreeRTOSI2cInitSet(DS_1339_MIO, FI2C_MASTER, RTC_ADDR);
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

    taskEXIT_CRITICAL(); //退出临界区
    vTaskDelete(NULL);
}

BaseType_t FFreeRTOSI2cRtcCreate(void)
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




