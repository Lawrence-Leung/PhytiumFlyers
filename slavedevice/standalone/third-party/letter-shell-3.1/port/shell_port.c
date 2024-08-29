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
 * Date: 2022-02-10 14:53:43
 * LastEditTime: 2022-02-25 11:47:29
 * Description:  This files is for letter shell port
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2021/9/6    init commit
 * 1.1   huanghe    2022/1/8    support shell interrupt
 */


#include "../src/shell.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "fassert.h"
#include "fparameters.h"
#include "finterrupt.h"
#include "fpl011.h"
#include "fpl011_hw.h"
#include "shell_port.h"

Shell shell_object;
u32 shell_last_result = 0;
char shell_buffer[4096];
extern void LSUserShellWrite(char data);
extern signed char LSUserShellRead(char *data);
extern void LSSerialConfig();
extern void LSSerialWaitLoop();
extern void LSSerialBNoWaitLoop();

/**
 * @brief 用户shell初始化
 *
 * Note: Call this function will lead to infinite loop
 */
void LSUserShellLoop(void)
{
    u32 reg_val;

    LSSerialConfig();

    shell_object.write = LSUserShellWrite;
    shell_object.read = LSUserShellRead;
    shellInit(&shell_object, shell_buffer, sizeof(shell_buffer));

    LSSerialWaitLoop();
}

void LSUserShellInit(void)
{
    u32 reg_val;

    LSSerialConfig();

    shell_object.write = LSUserShellWrite;
    shell_object.read = LSUserShellRead;
    shellInit(&shell_object, shell_buffer, sizeof(shell_buffer));
}

void LSuserShellNoWaitLoop(void)
{
     LSSerialBNoWaitLoop();
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
    LSUserPrintf("Exec: '%s'==========\r\n", cmd);
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
