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
 * FilePath: software_timer_cmd.c
 * Date: 2022-06-17 10:41:45
 * LastEditTime: 2022-06-17 10:41:45
 * Description:  This file is for software timer command interface
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2022/08/09  first commit
 */
#include "shell.h"
#include <string.h>
#include <stdio.h>
#include "feature_software_timer.h"

typedef enum
{
    CREATE_START_TASK_INDEX = 0,
    TIMER_ID_TASK_INDEX = 1,
    SOFTWARE_TIMER_FEATURE_LENGTH
} FreeRtosSoftTimerFeatureSelect;

static void SoftwareTimerCmdUsage(void)
{
    printf("Usage:\r\n");
    printf(" timer cre \r\n");
    printf("    -- Create and starts a one-shot timer and an auto-reload timer.\r\n");
    printf(" timer del \r\n");
    printf("    -- Delete and starts a one-shot timer and an auto-reload timer.\r\n");
    printf(" timer reset_cre \r\n");
    printf("    -- Create software timer use timer id and reset timer.\r\n");
    printf(" timer reset_del \r\n");
    printf("    -- Del software timer use timer id and reset timer.\r\n");

}

int SoftwareTimerCmd(int argc, char *argv[])
{
    static int create_flg[SOFTWARE_TIMER_FEATURE_LENGTH] = {0}; /* 1 is tasks has been created*/

    if (argc < 2)
    {
        SoftwareTimerCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "cre"))
    {
        if (create_flg[CREATE_START_TASK_INDEX] == 0)
        {
            CreateTimerTasks();
            create_flg[CREATE_START_TASK_INDEX] = 1;
        }
        else
        {
            printf("Please use del cmd first. \r\n");
        }
    }
    else if (!strcmp(argv[1], "del"))
    {
        if (create_flg[CREATE_START_TASK_INDEX] == 1)
        {
            DeleteTimerTasks();
            create_flg[CREATE_START_TASK_INDEX]  = 0;
        }
        else
        {
            printf("Please use cre cmd first. \r\n");
        }
    }
    else if (!strcmp(argv[1], "reset_cre"))
    {
        if (create_flg[TIMER_ID_TASK_INDEX] == 0)
        {
            CreateTimerResetTasks();
            create_flg[TIMER_ID_TASK_INDEX] = 1;
        }
        else
        {
            printf("Please use reset_del cmd first. \r\n");
        }
    }
    else if (!strcmp(argv[1], "reset_del"))
    {
        if (create_flg[TIMER_ID_TASK_INDEX] == 1)
        {
            DeleteTimerResetTasks();
            create_flg[TIMER_ID_TASK_INDEX]  = 0;
        }
        else
        {
            printf("Please use reset_cre cmd first. \r\n");
        }
    }
    else
    {
        printf("Error: Invalid arguments. \r\n");
        SoftwareTimerCmdUsage();
    }
    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), timer, SoftwareTimerCmd, software timer test);


