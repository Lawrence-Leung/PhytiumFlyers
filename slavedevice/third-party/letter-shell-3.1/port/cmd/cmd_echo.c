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
 * FilePath: cmd_echo.c
 * Date: 2022-02-24 18:24:53
 * LastEditTime: 2022-03-21 17:03:49
 * Description:  This file is for the echo command functions
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2022/3/25   first release
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../src/shell.h"
#include "fio.h"
#include "ftypes.h"

static void EchoCmdUsage()
{
    printf("usage:\r\n");
    printf("    echo [str] [str] \r\n");
    printf("         printf to shell\r\n");
}

static int EchoCmdEntry(int argc, char *argv[])
{
    int loop;

    if (argc < 2)
    {
        EchoCmdUsage();
        return -1;
    }

    for (loop = 1; loop < argc; loop++)
    {
        printf("%s", argv[loop]);
    }

    printf("\r\n");
    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), echo, EchoCmdEntry, printf string to shell prompt);