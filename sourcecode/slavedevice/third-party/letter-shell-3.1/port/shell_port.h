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
 * FilePath: shell_port.h
 * Date: 2022-02-24 22:03:34
 * LastEditTime: 2022-02-24 22:03:34
 * Description:  This file is for the shell port related functions definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2022/3/25   first release
 */


#ifndef SHELL_PORT_H
#define SHELL_PORT_H

#include "shell.h"
#include "ftypes.h"
#include "FreeRTOS.h"
#include "task.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern Shell shell;
BaseType_t LSUserShellTask(void);
void LSUserExec(const char *cmd);
int LSUserGetLastRet(void);
void LSUserSetResult(u32 result);
u32 LSUserGetResult(void);
void LSUserSlient(boolean slient);
boolean LSUserIsSlient(void);
void LSUShowVersion(void);
#define LSUserPrintf(format, ...)      \
    if (!LSUserIsSlient())             \
    {                                  \
        printf(format, ##__VA_ARGS__); \
    }

#ifdef __cplusplus
}
#endif

#endif
