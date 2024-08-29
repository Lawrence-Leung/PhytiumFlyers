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
 * FilePath: shell_port.c
 * Date: 2022-02-24 22:03:27
 * LastEditTime: 2022-02-24 22:03:28
 * Description:  This file is for the shell port related functions
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2022/3/25   first release
 */


#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "fassert.h"
#include "shell_port.h"
#include "shell.h"
#include "FreeRTOS.h"
#include "task.h"


Shell shell_object;
u32 shell_last_result = 0;
char shell_buffer[4096];
extern void LSUserShellWrite(char data);
extern signed char LSUserShellRead(char *data);
extern void LSSerialConfig();
extern void LSSerialWaitLoop();


void LSUserShellTaskCreate(void *args)
{
    BaseType_t ret;
    LSSerialConfig();

    shell_object.write = LSUserShellWrite;
    shell_object.read = LSUserShellRead;
    shellInit(&shell_object, shell_buffer, 4096);


    ret = xTaskCreate((TaskFunction_t)LSSerialWaitLoop,  /* 任务入口函数 */
                      (const char *)"LSSerialWaitLoop",/* 任务名字 */
                      (uint16_t)1024,  /* 任务栈大小 */
                      (void *)NULL,/* 任务入口函数参数 */
                      (UBaseType_t)2,  /* 任务的优先级 */
                      NULL); /* 任务控制块指针 */

    FASSERT_MSG(ret == pdPASS, "LSUserShellTask create is failed");

    vTaskDelete(NULL);
}

/**
 * @brief 用户shell初始化
 *
 * Note: Call this function will lead to infinite create freertos task
 */
BaseType_t LSUserShellTask(void)
{
    return xTaskCreate((TaskFunction_t)LSUserShellTaskCreate,  /* 任务入口函数 */
                       (const char *)"LSUserShellTaskCreate",/* 任务名字 */
                       (uint16_t)1024,  /* 任务栈大小 */
                       (void *)NULL,/* 任务入口函数参数 */
                       (UBaseType_t)2,  /* 任务的优先级 */
                       NULL); /* 任务控制块指针 */
}



/**
 * @name: LSUserGetLastRet
 * @msg: 获取上一条命令的执行返回值
 * @return {*}
 */
int LSUserGetLastRet(void)
{
    return shell_object.lastRet;
}

/**
 * @name: 通过letter shell执行一行命令
 * @msg:
 * @return {*}
 * @param {char} *cmd
 */
void LSUserExec(const char *cmd)
{
    LSUserPrintf("exec: '%s'==========\r\n", cmd);
    shellRun(&shell_object, cmd);
    LSUserPrintf("================\r\n");
    return;
}

void LSUserSetResult(u32 result)
{
    shell_last_result = result;
}

u32 LSUserGetResult()
{
    return shell_last_result;
}

void LSUserSlient(boolean slient)
{
    shell_object.slient = slient ? 1 : 0;
}

boolean LSUserIsSlient(void)
{
    return (1 == shell_object.slient);
}

void LSUShowVersion(void)
{
    shellWriteVersion(&shell_object);
}