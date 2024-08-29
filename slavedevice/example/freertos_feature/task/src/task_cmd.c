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
 * FilePath: task_cmd.c
 * Date: 2022-06-17 10:41:45
 * LastEditTime: 2022-06-17 10:41:45
 * Description:  This file is for task example command interface
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2022/08/09  first commit
 */
#include "shell.h"
#include "feature_task.h"
#include <string.h>
#include <stdio.h>

typedef enum
{
    CREATING_TASK_INDEX = 0,
    PARAMETERS_TEST_INDEX = 1,
    PRIORITY_TEST_INDEX,
    BLOCK_STATE_TEST_INDEX,
    DELAY_UNTIL_TEST_INDEX,
    BLOCKING_NON_BLOCKING_INDEX,
    IDLE_TEST_INDEX,
    CHANGE_PRIORITY_TEST_INDEX,
    TASK_FEATURE_LENGTH
} FreeRtosTaskFeatureSelect;

static void CreateTasksCmdUsage(void)
{
    printf("Usage:\r\n");
    printf(" task cre \r\n");
    printf("    -- Create tasks now. \r\n");
    printf(" task del \r\n");
    printf("    -- Del tasks now. \r\n");
    printf(" task para_cre \r\n");
    printf("    -- Create parameter test tasks now. \r\n");
    printf(" task para_del \r\n");
    printf("    -- Del parameter test tasks now. \r\n");
    printf(" task pri_cre \r\n");
    printf("    -- Create priority test tasks now. \r\n");
    printf(" task pri_del \r\n");
    printf("    -- Del priority test tasks now. \r\n");
    printf(" task blo_cre \r\n");
    printf("    -- Create block state test tasks now. \r\n");
    printf(" task blo_del \r\n");
    printf("    -- Del block state test tasks now. \r\n");
    printf(" task dn_cre \r\n");
    printf("    -- Create task until test tasks now. \r\n");
    printf(" task dn_del \r\n");
    printf("    -- Del task until test tasks now. \r\n");
    printf(" task bn_cre \r\n");
    printf("    -- Create task combining blocking non blocking tasks now. \r\n");
    printf(" task bn_del \r\n");
    printf("    -- Del task combining blocking non blocking tasks now. \r\n");
    printf(" task idle_cre \r\n");
    printf("    -- Create task idle tasks now. \r\n");
    printf(" task idle_del \r\n");
    printf("    -- Del task idle tasks now \r\n");
    printf(" task cha_cre \r\n");
    printf("    -- Create task change priority tasks now. \r\n");
    printf(" task cha_del \r\n");
    printf("    -- Del task change priority tasks now. \r\n");
}


int CreateTasksCmd(int argc, char *argv[])
{
    static int create_flg[TASK_FEATURE_LENGTH] = {0}; /* 1 is tasks has been created*/

    if (argc < 2)
    {
        CreateTasksCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "cre"))
    {
        if (create_flg[CREATING_TASK_INDEX]  == 0)
        {
            CreateTasks();
            create_flg[CREATING_TASK_INDEX] = 1;
        }
        else
        {
            printf("Please use del cmd first. \r\n");
        }
    }
    else if (!strcmp(argv[1], "del"))
    {
        if (create_flg[CREATING_TASK_INDEX]  == 1)
        {
            DeleteTasks();
            create_flg[CREATING_TASK_INDEX]  = 0;
        }
        else
        {
            printf("Please use cre cmd first. \r\n");
        }
    }
    else if (!strcmp(argv[1], "para_cre"))
    {
        if (create_flg[PARAMETERS_TEST_INDEX]  == 0)
        {
            CreateTasksForParamterTest();
            create_flg[PARAMETERS_TEST_INDEX] = 1;
        }
        else
        {
            printf("Please use task para_del cmd first. \r\n");
        }
    }
    else if (!strcmp(argv[1], "para_del"))
    {
        if (create_flg[PARAMETERS_TEST_INDEX]  == 1)
        {
            DeleteTasksForParamterTest();
            create_flg[PARAMETERS_TEST_INDEX]  = 0;
        }
        else
        {
            printf("Please use task para_cre cmd first. \r\n");
        }
    }
    else if (!strcmp(argv[1], "pri_cre"))
    {
        if (create_flg[PRIORITY_TEST_INDEX]  == 0)
        {
            CreateTasksForPriorityTest();
            create_flg[PRIORITY_TEST_INDEX] = 1;
        }
        else
        {
            printf("Please use task pri_del cmd first. \r\n");
        }
    }
    else if (!strcmp(argv[1], "pri_del"))
    {
        if (create_flg[PRIORITY_TEST_INDEX]  == 1)
        {
            DeleteTasksForPriorityTest();
            create_flg[PRIORITY_TEST_INDEX]  = 0;
        }
        else
        {
            printf("Please use task pri_cre cmd first. \r\n");
        }
    }
    else if (!strcmp(argv[1], "blo_cre"))
    {
        if (create_flg[BLOCK_STATE_TEST_INDEX]  == 0)
        {
            CreateTasksForBlockTest();
            create_flg[BLOCK_STATE_TEST_INDEX] = 1;
        }
        else
        {
            printf("Please use task blo_del cmd first. \r\n");
        }
    }
    else if (!strcmp(argv[1], "blo_del"))
    {
        if (create_flg[BLOCK_STATE_TEST_INDEX]  == 1)
        {
            DeleteTasksForBlockTest();
            create_flg[BLOCK_STATE_TEST_INDEX]  = 0;
        }
        else
        {
            printf("Please use task blo_cre cmd first. \r\n");
        }
    }
    else if (!strcmp(argv[1], "dn_cre"))
    {
        if (create_flg[DELAY_UNTIL_TEST_INDEX]  == 0)
        {
            CreateTasksForDelayUntilTest();
            create_flg[DELAY_UNTIL_TEST_INDEX] = 1;
        }
        else
        {
            printf("Please use task dn_del cmd first. \r\n");
        }
    }
    else if (!strcmp(argv[1], "dn_del"))
    {
        if (create_flg[DELAY_UNTIL_TEST_INDEX]  == 1)
        {
            DeleteTasksForDelayUntilTest();
            create_flg[DELAY_UNTIL_TEST_INDEX]  = 0;
        }
        else
        {
            printf("Please use task dn_cre cmd first. \r\n");
        }
    }
    else if (!strcmp(argv[1], "bn_cre"))
    {
        if (create_flg[BLOCKING_NON_BLOCKING_INDEX]  == 0)
        {
            CreateTasksForBlockingOrNone();
            create_flg[BLOCKING_NON_BLOCKING_INDEX] = 1;
        }
        else
        {
            printf("Please use task bn_del cmd first. \r\n");
        }
    }
    else if (!strcmp(argv[1], "bn_del"))
    {
        if (create_flg[BLOCKING_NON_BLOCKING_INDEX]  == 1)
        {
            DeleteTasksForBlockingOrNone();
            create_flg[BLOCKING_NON_BLOCKING_INDEX]  = 0;
        }
        else
        {
            printf("Please use task bn_cre cmd first. \r\n");
        }
    }
    else if (!strcmp(argv[1], "idle_cre"))
    {
        if (create_flg[IDLE_TEST_INDEX]  == 0)
        {
            CreateTasksForIdleTask();
            create_flg[IDLE_TEST_INDEX] = 1;
        }
        else
        {
            printf("Please use task idle_del cmd first. \r\n");
        }
    }
    else if (!strcmp(argv[1], "idle_del"))
    {
        if (create_flg[IDLE_TEST_INDEX]  == 1)
        {
            DeleteTasksForForIdleTask();
            create_flg[IDLE_TEST_INDEX]  = 0;
        }
        else
        {
            printf("Please use task idle_cre cmd first. \r\n");
        }
    }
    else if (!strcmp(argv[1], "cha_cre"))
    {
        if (create_flg[CHANGE_PRIORITY_TEST_INDEX]  == 0)
        {
            CreateTasksForChangePriorityTest();
            create_flg[CHANGE_PRIORITY_TEST_INDEX] = 1;
        }
        else
        {
            printf("Please use task cha_del cmd first. \r\n");
        }
    }
    else if (!strcmp(argv[1], "cha_del"))
    {
        if (create_flg[CHANGE_PRIORITY_TEST_INDEX]  == 1)
        {
            DeleteTasksForChangePriorityTest();
            create_flg[CHANGE_PRIORITY_TEST_INDEX]  = 0;
        }
        else
        {
            printf("Please use task cha_cre cmd first. \r\n");
        }
    }
    else
    {
        printf("Error: Invalid arguments. \r\n");
        CreateTasksCmdUsage();
    }
    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), task, CreateTasksCmd, task creating test);


