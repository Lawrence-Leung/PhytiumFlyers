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
 * FilePath: cmd_sleep.c
 * Date: 2022-02-10 14:53:43
 * LastEditTime: 2022-02-25 11:47:39
 * Description:  This files is for sleep command implmentation
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  zhugengyu   2021/9/6    init commit 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../src/shell.h"
#include "fio.h"
#include "ftypes.h"
#include "fsleep.h"

static void SleepCmdUsage()
{
	printf("usage:\r\n");
	printf("    sleep [-s | -m | -u] [num] \r\n");
    printf("         sleep for num of seconds (-s), mill-seconds(-m), micro-seconds (-u)\r\n");
}

static int SleepCmdEntry(int argc, char *argv[])
{
    u32 time = 0;

    if (argc < 3)
    {
		SleepCmdUsage();
        return -1;
	}

    if(!strcmp(argv[1], "-s"))
    {
        time = strtoul(argv[2], NULL, 0);
        fsleep_seconds(time);
    }
    else if(!strcmp(argv[1], "-m"))
    {
        time = strtoul(argv[2], NULL, 0);
        fsleep_millisec(time);
    }
    else if(!strcmp(argv[1], "-u"))
    {
        time = strtoul(argv[2], NULL, 0);
        fsleep_microsec(time);
    }
    else
    {
        return -2;
    }

    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), sleep, SleepCmdEntry, blocking sleep for a period of time);