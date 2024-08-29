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
 * FilePath: task_notify_cmd.c
 * Date: 2022-06-17 10:41:45
 * LastEditTime: 2022-06-17 10:41:45
 * Description:  This file is for task notifiy command interface
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2022/08/09  first commit
 */
#include "shell.h"
#include <string.h>
#include <stdio.h>
#include "feature_task_notify.h"

typedef enum
{
    NOTIFY_TAKE_TRUE_TASK_INDEX = 0,
    NOTIFY_TAKE_FALSE_TASK_INDEX = 1,
    NOTIFY_FEATURE_LENGTH
} FreeRtosTaskNotifyFeatureSelect;

static void TaskNotifyCmdUsage(void)
{
    printf("Usage:\r\n");
    printf(" notify true_cre \r\n");
    printf("    -- Create task notify test now, use notify take true.\r\n");
    printf(" notify true_del \r\n");
    printf("    -- Del notify take true tasks now.\r\n");
    printf(" notify false_cre \r\n");
    printf("    -- Create task notify test now, use notify take false.\r\n");
    printf(" notify false_del \r\n");
    printf("    -- Del notify take false tasks now.\r\n");

}

int TaskNotifyCmd(int argc, char *argv[])
{
    static int create_flg[NOTIFY_FEATURE_LENGTH] = {0}; /* 1 is tasks has been created*/

    if (argc < 2)
    {
        TaskNotifyCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "true_cre"))
    {
        if (create_flg[NOTIFY_TAKE_TRUE_TASK_INDEX] == 0)
        {
            CreateNotifyTakeTrueTasks();
            create_flg[NOTIFY_TAKE_TRUE_TASK_INDEX] = 1;
        }
        else
        {
            printf("Please use true_del cmd first. \r\n");
        }
    }
    else if (!strcmp(argv[1], "true_del"))
    {
        if (create_flg[NOTIFY_TAKE_TRUE_TASK_INDEX] == 1)
        {
            DeleteNotifyTakeTrueTasks();
            create_flg[NOTIFY_TAKE_TRUE_TASK_INDEX]  = 0;
        }
        else
        {
            printf("Please use true_cre cmd first. \r\n");
        }
    }
    else if (!strcmp(argv[1], "false_cre"))
    {
        if (create_flg[NOTIFY_TAKE_FALSE_TASK_INDEX] == 0)
        {
            CreateNotifyTakeFalseTasks();
            create_flg[NOTIFY_TAKE_FALSE_TASK_INDEX] = 1;
        }
        else
        {
            printf("Please use false_del cmd first. \r\n");
        }
    }
    else if (!strcmp(argv[1], "false_del"))
    {
        if (create_flg[NOTIFY_TAKE_FALSE_TASK_INDEX] == 1)
        {
            DeleteNotifyTakeFalseTasks();
            create_flg[NOTIFY_TAKE_FALSE_TASK_INDEX]  = 0;
        }
        else
        {
            printf("Please use false_cre cmd first. \r\n");
        }
    }
    else
    {
        printf("Error: Invalid arguments. \r\n");
        TaskNotifyCmdUsage();
    }
    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), notify, TaskNotifyCmd, task notify test);


