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
 * FilePath: eventgroup_cmd.c
 * Date: 2022-06-17 10:42:40
 * LastEditTime: 2022-06-17 10:42:40
 * Description:  This file is for eventgroup command interface
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2022/08/09  first commit
 */

#include "shell.h"
#include <string.h>
#include <stdio.h>
#include "feature_eventgroup.h"

typedef enum
{
    MANAGE_TASK_INDEX = 0,
    SYNC_TASK_INDEX = 1,
    EVENTGROUP_FEATURE_LENGTH
} FreeRtosEventgroupFeatureSelect;

static void EventTasksCmdUsage(void)
{
    printf("Usage:\r\n");
    printf(" event manage_cre \r\n");
    printf("    -- Create manage tasks now.\r\n");
    printf(" event manage_del \r\n");
    printf("    -- Del manage tasks now.\r\n");
    printf(" event sync_cre \r\n");
    printf("    -- Create sync tasks now.\r\n");
    printf(" event sync_del \r\n");
    printf("    -- Del sync tasks now.\r\n");

}

int CreateEventCmd(int argc, char *argv[])
{
    static int create_flg[EVENTGROUP_FEATURE_LENGTH] = {0}; /* 1 is tasks has been created*/

    if (argc < 2)
    {
        EventTasksCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "manage_cre"))
    {
        if (create_flg[MANAGE_TASK_INDEX] == 0)
        {
            CreateManagementTasks();
            create_flg[MANAGE_TASK_INDEX] = 1;
        }
        else
        {
            printf("Please use manage_del cmd first. \r\n");
        }
    }
    else if (!strcmp(argv[1], "manage_del"))
    {
        if (create_flg[MANAGE_TASK_INDEX] == 1)
        {
            DeleteManagementTasks();
            create_flg[MANAGE_TASK_INDEX]  = 0;
        }
        else
        {
            printf("Please use manage_cre cmd first. \r\n");
        }
    }
    else if (!strcmp(argv[1], "sync_cre"))
    {
        if (create_flg[SYNC_TASK_INDEX] == 0)
        {
            CreateSyncTasks();
            create_flg[SYNC_TASK_INDEX] = 1;
        }
        else
        {
            printf("Please use sync_del cmd first. \r\n");
        }
    }
    else if (!strcmp(argv[1], "sync_del"))
    {
        if (create_flg[SYNC_TASK_INDEX] == 1)
        {
            DeleteSyncTasks();
            create_flg[SYNC_TASK_INDEX]  = 0;
        }
        else
        {
            printf("Please use sync_cre cmd first. \r\n");
        }
    }
    else
    {
        printf("Error: Invalid arguments. \r\n");
        EventTasksCmdUsage();
    }
    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), event, CreateEventCmd, event group creating test);


