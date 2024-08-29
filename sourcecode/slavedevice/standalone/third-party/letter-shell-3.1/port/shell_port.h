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
 * Date: 2022-02-10 14:53:43
 * LastEditTime: 2022-02-25 11:45:44
 * Description:  This files is for letter shell port
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2021/9/6    init commit
 * 1.1   huanghe    2022/1/8    support shell interrupt
 */

#ifndef __SHELL_PORT_H__
#define __SHELL_PORT_H__

#include "../src/shell.h"
#include "ftypes.h"


#ifdef __cplusplus
extern "C"
{
#endif


extern Shell shell;
void LSUserShellLoop(void);
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

void LSUserShellInit(void);
void LSuserShellNoWaitLoop(void);

#ifdef __cplusplus
}
#endif

#endif
