/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
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
 * Date: 2022-06-17 08:17:59
 * LastEditTime: 2022-06-17 08:17:59
 * Description:  This file is for gpio main entry.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  zhugengyu  2022/8/26    init commit
 */

// #include "shell.h"
// #include "shell_port.h"
#include <stdio.h>
#include "dht11.h"

int main(void)
{
    BaseType_t ret;

    // ret = LSUserShellTask() ;
    ret = AppTask();
    // ret = xTaskCreate((TaskFunction_t)DHT11,  /* task entry */
    //                   (const char *)"DHT11",/* task name */
    //                   (uint16_t)1024,  /* task stack size in words */
    //                   NULL, /* task params */
    //                   (UBaseType_t)configMAX_PRIORITIES - 1,  /* task priority */
    //                   NULL); /* task handler */

    // ret = FFreeRTOSRunGpioIOIrq("3-a-4", "3-a-5");
    if (ret != pdPASS)
    {
        goto FAIL_EXIT;
    }

    vTaskStartScheduler(); /* 启动任务，开启调度 */
    while (1)
    {
        f_printk("failed\r\n");
    } /* 正常不会执行到这里 */

FAIL_EXIT:
    printf("Failed,the ret value is 0x%x. \r\n", ret);
    return 0;
}
