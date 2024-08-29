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
 * FilePath: main.c
 * Date: 2022-07-18 14:39:10
 * LastEditTime: 2022-07-18 14:39:10
 * Description:  This file is for i2c main entry.
 *
 * Modify History:
 *  Ver       Who            Date                 Changes
 * -----    ------         --------     --------------------------------------
 *  1.0    liushengming   2022/11/25             init commit
 */

#include <stdio.h>
#include "shell.h"
// #include "shell_port.h"
// #include "i2c_example.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "XFS5152.h"
#include "fi2c.h"
#include "fi2c_os.h"

extern FFreeRTOSI2c *os_i2c_master;

int main(void)
{
//     FFreeRTOSI2cInitSet(XFS5152_MIO, FI2C_MASTER, I2C_ADDR);

//     BaseType_t ret;

//     ret = FFreeRTOSI2cLoopbackCreate();
//     if (ret != pdPASS)
//     {
//         goto FAIL_EXIT;
//     }

//     vTaskStartScheduler(); /* 启动任务，开启调度 */
//     while (1); /* 正常不会执行到这里 */

// FAIL_EXIT:
//     printf("Failed,the ret value is 0x%x. \r\n", ret);
//     return 0;


    // printf("init!");
    // FError err;
    
    // err = FFreeRTOSI2cInitSet(XFS5152_MIO, FI2C_MASTER, I2C_ADDR);
    // f_printk("i2c_slave:0x%x\r\n",os_i2c_master->i2c_device.config.slave_addr);
    // if (err != FREERTOS_I2C_SUCCESS)
    // {
    //     vPrintf("I2c FFreeRTOSI2cInitSet failed.\r\n");
    //     return 0;
    // }
    // else{
    //     f_printk("init end!");
    // }

    // // SetVolume(1);
    // // SetReader(Reader_XiaoPing);
    // // GetChipStatus();
    // // f_printk("start 0x 70");
    // // speech_text("你好亚博智能科技",GB2312);
    // // f_printk("end!");
    u8 state = 0;
    
    
    while (1)
    {
        GetChipStatus();
        // f_printk("state:%x\r\n",state);
    }
    
    return 0;
}