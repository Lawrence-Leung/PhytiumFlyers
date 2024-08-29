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
 * FilePath: atomic_example_cmd.c
 * Date: 2022-06-17 10:41:45
 * LastEditTime: 2022-06-17 10:41:45
 * Description:  This file is for atomic command interface
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2023/06/25 first commit
 */

#include <string.h>
#include <stdio.h>
#include "shell.h"
#include "atomic_example.h"

static void CreateAtomicCmdUsage(void)
{
    printf("Usage:\r\n");
    printf(" atomic cre \r\n");
    printf("    -- Create atomic test task now.\r\n");
    printf(" atomic del \r\n");
    printf("    -- Del atomic test task now.\r\n");
}

int CreateAtomicCmd(int argc, char *argv[])
{
    static int create_flg = 0; /* 1 is tasks has been created*/

    if (argc < 2)
    {
        CreateAtomicCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "cre"))
    {
        if (create_flg == 0)
        {
            CreateAtomicTasks();
            create_flg = 1;
        }
        else
        {
            printf("Please use atomic del cmd first. \r\n");
        }
    }
    else if (!strcmp(argv[1], "del"))
    {
        if (create_flg  == 1)
        {
            DeleteAtomicTasks();
            create_flg  = 0;
        }
        else
        {
            printf("Please use atomic cre cmd first. \r\n");
        }
    }
    else
    {
        printf("Error: Invalid arguments. \r\n");
        CreateAtomicCmdUsage();
    }
    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), atomic, CreateAtomicCmd, atomic task test);


