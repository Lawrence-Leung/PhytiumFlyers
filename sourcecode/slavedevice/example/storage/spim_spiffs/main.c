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
 * Description:  This file is for the main functions.
 * 
 * Modify History: 
 *  Modify History: 
 *  Ver     Who      Date         Changes
 * -----   ------  --------   --------------------------------------
 * 1.0 liqiaozhong 2022/11/2  first commit
 */

#include "shell.h"
#include "shell_port.h"
#include <stdio.h>
#include "spim_spiffs_example.h"
#if defined(CONFIG_TARGET_E2000D)||defined(CONFIG_TARGET_E2000Q)
#define SPIM_TEST_ID  FSPI2_ID
#elif defined(CONFIG_TARGET_PHYTIUMPI)
#define SPIM_TEST_ID  FSPI0_ID
#endif
int main(void)
{
    BaseType_t ret;

    ret = FFreeRTOSSpimSpiffsCreate(SPIM_TEST_ID);
    if(ret != pdPASS)
        goto FAIL_EXIT;

    ret = LSUserShellTask() ;
    if(ret != pdPASS)
        goto FAIL_EXIT;

    vTaskStartScheduler(); /* 启动任务，开启调度 */   
    while (1); /* 正常不会执行到这里 */
    
FAIL_EXIT:
    printf("failed 0x%x \r\n", ret);  
    return 0;
}
