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
 * FilePath: fi2c_os.h
 * Date: 2022-07-15 10:43:51
 * LastEditTime: 2022-07-15 10:43:51
 * Description:  This file is for providing function related definitions of i2c driver used in FreeRTOS.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  liushengming 2022/07/15 init
 */

#ifndef FI2C_OS_H
#define FI2C_OS_H
/***************************** Include Files *********************************/
#include <FreeRTOS.h>
#include <event_groups.h>
#include <semphr.h>
#include "ferror_code.h"
#include "fi2c.h"
#include "fi2c_hw.h"
#include "ftypes.h"
/************************** Constant Definitions *****************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*Error code from standalone i2c driver*/
#define FREERTOS_I2C_SUCCESS                    FI2C_SUCCESS
#define FREERTOS_I2C_INVAILD_PARAM_ERROR        FI2C_ERR_INVAL_PARM
#define FREERTOS_I2C_NOT_READY_ERROR            FI2C_ERR_NOT_READY
#define FREERTOS_I2C_TIMEOUT_ERROR              FI2C_ERR_TIMEOUT
#define FREERTOS_I2C_NOT_SUPPORT_ERROR          FI2C_ERR_NOT_SUPPORT
#define FREERTOS_I2C_INVAL_STATE_ERROR          FI2C_ERR_INVAL_STATE

/*Error code depend on OS standard*/
#define FREERTOS_I2C_TASK_ERROR                 FT_CODE_ERR(ErrModPort, ErrBspI2c, 0x1)
#define FREERTOS_I2C_MESG_ERROR                 FT_CODE_ERR(ErrModPort, ErrBspI2c, 0x2)
#define FREERTOS_I2C_TIME_ERROR                 FT_CODE_ERR(ErrModPort, ErrBspI2c, 0x3)
#define FREERTOS_I2C_MEMY_ERROR                 FT_CODE_ERR(ErrModPort, ErrBspI2c, 0x4)

/*!
* @cond RTOS_PRIVATE
* @name I2C FreeRTOS handler
*
* These are the only valid states for txEvent and rxEvent
*/
/*@{*/
/*! @brief Event flag - transfer complete. */
#define RTOS_I2C_READ_DONE 0x1
/*! @brief Event flag - hardware buffer overrun. */
#define RTOS_I2C_WRITE_DONE 0x2
/*！ @brief Event flag Receive is error */
#define RTOS_I2C_TRANS_ABORTED 0x4

#define I2C_MASTER_IRQ_PRORITY 0xc
#define I2C_SLAVE_IRQ_PRORITY  0xb

#define IO_BUF_LEN 256

/************************** Variable Definitions *****************************/
/**
 * iic message structure
 */
typedef struct
{
    void *buf;              /* i2c read or write buffer */
    size_t buf_length;      /* i2c read or write buffer length */
    u32 slave_addr;         /* i2c slave addr, you can change slave_addr to send different device in the bus*/
    u32 mem_addr;           /* i2c slave address offset to read or write */
    u8 mem_byte_len;        /* sizeof slave address */
    volatile u8 mode;       /* transport mode */
} FFreeRTOSI2cMessage;

typedef struct
{
    FI2c i2c_device;
    SemaphoreHandle_t wr_semaphore; /* i2c read and write semaphore for resource sharing */
    EventGroupHandle_t trx_event;   /* i2c TX/RX completion event */
} FFreeRTOSI2c;

enum /*选择操作I2C的方式*/
{
    FI2C_READ_DATA_POLL,
    FI2C_READ_DATA_INTR,
    FI2C_WRITE_DATA_POLL,
    FI2C_WRITE_DATA_INTR,

    FI2C_READ_DATA_MODE_NUM
};

/************************** Function Prototypes ******************************/
/* init freeRTOS i2c instance */
FFreeRTOSI2c *FFreeRTOSI2cInit(u32 instance_id, u32 work_mode, u32 slave_address, u32 speed_rate);

/* deinit freeRTOS i2c instance */
void FFreeRTOSI2cDeinit(FFreeRTOSI2c *os_i2c_p);

/* tranfer i2c mesage */
FError FFreeRTOSI2cTransfer(FFreeRTOSI2c *os_i2c_p, FFreeRTOSI2cMessage *message);

#ifdef __cplusplus
}
#endif

#endif
