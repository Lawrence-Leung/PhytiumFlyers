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
 * FilePath: resource_cmd.c
 * Date: 2022-06-17 10:41:45
 * LastEditTime: 2022-06-17 10:41:45
 * Description:  This file is for resource command interface
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2022/08/09  first commit
 */
#include "shell.h"
#include "feature_resource.h"
#include <string.h>
#include <stdio.h>

typedef enum
{
    MUTEX_TASK_INDEX = 0,
    GATEKEEPER_TEST_INDEX = 1,
    RESOURCE_FEATURE_LENGTH
} FreeRtosResourceFeatureSelect;

static void ResourceTasksCmdUsage(void)
{
    printf("Usage:\r\n");
    printf(" resource mutex_cre \r\n");
    printf("    -- Create mutex tasks now. \r\n");
    printf(" resource mutex_del \r\n");
    printf("    -- Del mutex tasks now. \r\n");
    printf(" resource gate_cre \r\n");
    printf("    -- Create gatekeeper tasks now. \r\n");
    printf(" resource gate_del \r\n");
    printf("    -- Del gatekeeper tasks now. \r\n");

}

int ResourceTasksCmd(int argc, char *argv[])
{
    static int create_flg[RESOURCE_FEATURE_LENGTH] = {0}; /* 1 is tasks has been created*/

    if (argc < 2)
    {
        ResourceTasksCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "mutex_cre"))
    {
        if (create_flg[MUTEX_TASK_INDEX]  == 0)
        {
            CreateResourceTasks();
            create_flg[MUTEX_TASK_INDEX] = 1;
        }
        else
        {
            printf("Please use mutex_del cmd first. \r\n");
        }
    }
    else if (!strcmp(argv[1], "mutex_del"))
    {
        if (create_flg[MUTEX_TASK_INDEX]  == 1)
        {
            DeleteResourceTasks();
            create_flg[MUTEX_TASK_INDEX]  = 0;
        }
        else
        {
            printf("Please use mutex_cre cmd first. \r\n");
        }
    }
    else if (!strcmp(argv[1], "gate_cre"))
    {
        if (create_flg[GATEKEEPER_TEST_INDEX]  == 0)
        {
            CreateGatekeeperTasks();
            create_flg[GATEKEEPER_TEST_INDEX] = 1;
        }
        else
        {
            printf("Please use gate_del cmd first. \r\n");
        }
    }
    else if (!strcmp(argv[1], "gate_del"))
    {
        if (create_flg[GATEKEEPER_TEST_INDEX]  == 1)
        {
            DeleteGatekeeperTasks();
            create_flg[GATEKEEPER_TEST_INDEX]  = 0;
        }
        else
        {
            printf("Please use gate_cre cmd first. \r\n");
        }
    }
    else
    {
        printf("Error: Invalid arguments. \r\n");
        ResourceTasksCmdUsage();
    }
    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), resource, ResourceTasksCmd, Resource Management test);


