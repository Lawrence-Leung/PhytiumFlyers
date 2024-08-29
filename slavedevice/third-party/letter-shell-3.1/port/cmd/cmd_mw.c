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
 * FilePath: cmd_mw.c
 * Date: 2022-02-24 18:24:53
 * LastEditTime: 2022-03-21 17:03:57
 * Description:  This file is for the mw command functions
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

static void MwUsage()
{
    printf("usage:\r\n");
    printf("    mw [-b|-w|-l|-q] address value [-c count]\r\n");
}

static int MwCmdEntry(int argc, char **argv)
{
    uintptr addr = 0;
    u64 value = 0;
    int size = 1, n = 1;
    int index = 0;
    int i;

    if (argc < 3)
    {
        MwUsage();
        return -1;
    }

    for (i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-b"))
        {
            size = 1;
        }
        else if (!strcmp(argv[i], "-w"))
        {
            size = 2;
        }
        else if (!strcmp(argv[i], "-l"))
        {
            size = 4;
        }
        else if (!strcmp(argv[i], "-q"))
        {
            size = 8;
        }
        else if (!strcmp(argv[i], "-c") && (argc > i + 1))
        {
            n = strtoul(argv[i + 1], NULL, 0);
            if (n == 0)
            {
                printf("mw: the writing count is zero by '-c %s'.", argv[i + 1]);
                return -1;
            }
            i++;
        }
        else if (*argv[i] == '-')
        {
            MwUsage();
            return -1;
        }
        else if (*argv[i] != '-' && strcmp(argv[i], "-") != 0)
        {
            if (index == 0)
            {
                addr = strtoul(argv[i], NULL, 0);
            }
            else if (index == 1)
            {
                value = strtoull(argv[i], NULL, 0);
            }
            else if (index >= 2)
            {
                printf("mw: invalid paramter '%s'\r\n", argv[i]);
                printf("try 'help mw' for more information.\r\n");
                return (-1);
            }
            index++;
        }
    }

    if (size == 1)
    {
        addr &= ~((uintptr)0x0);
    }
    else if (size == 2)
    {
        addr &= ~((uintptr)0x1);
    }
    else if (size == 4)
    {
        addr &= ~((uintptr)0x3);
    }
    else if (size == 8)
    {
        addr &= ~((uintptr)0x7);
    }
    n = n * size;

    for (i = 0; i < n; i += size)
    {
        if (size == 1)
        {
            FtOut8((uintptr)(addr + i), (u8)value);
        }
        else if (size == 2)
        {
            FtOut16((uintptr)(addr + i), (u16)value);
        }
        else if (size == 4)
        {
            FtOut32((uintptr)(addr + i), (u32)value);
        }
        else if (size == 8)
        {
            FtOut64((uintptr)(addr + i), (u64)value);
        }
    }
    printf("Write done.\r\n");

    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), mw, MwCmdEntry, write values to memory region);