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
 * FilePath: cmd_i2c.c
 * Date: 2023-09-25 14:42:53
 * LastEditTime: 2023-09-25 14:42:53
 * Description:  This file is for i2c shell command implmentation.
 *
 * Modify History:
 *  Ver       Who            Date                 Changes
 * -----    ------         --------     --------------------------------------
 *  1.0    liushengming   2023/09/25             init commit
 */
/***************************** Include Files *********************************/
#include <string.h>
#include <stdio.h>
#include "strto.h"
#include "sdkconfig.h"
#include "FreeRTOS.h"
#include "shell.h"
#include "fi2c_os.h"
#include "i2c_example.h"

/************************** Function Prototypes ******************************/

/*****************************************************************************/
static void FI2cExampleUsage()
{
    printf("Usage:\r\n");
#if defined(CONFIG_E2000D_DEMO_BOARD)||defined(CONFIG_E2000Q_DEMO_BOARD)
    printf("    i2c rtc\r\n");
    printf("        -- E2000 demo board set time and read it.\r\n");
#endif
#if defined(CONFIG_FIREFLY_DEMO_BOARD)
    printf("    i2c rw\r\n");
    printf("        -- firefly board,Two i2c controllers are used for master-slave communication.\r\n");
#endif
}

static int I2cCmdEntry(int argc, char *argv[])
{
    int ret = 0;
    if (argc < 2)
    {
        FI2cExampleUsage();
        return -1;
    }
#if defined(CONFIG_E2000D_DEMO_BOARD)||defined(CONFIG_E2000Q_DEMO_BOARD)
    else if (!strcmp(argv[1], "rtc"))
    {
        ret = FFreeRTOSI2cRtcCreate();
        if (ret != pdPASS)
        {
            printf("FFreeRTOSI2cRtcCreate error :0x%x!\n",ret);
            return ret;
        }
    }
#endif
#if defined(CONFIG_FIREFLY_DEMO_BOARD)
    else if (!strcmp(argv[1], "rw"))
    {
        ret = FFreeRTOSI2cLoopbackCreate();
        if (ret != pdPASS)
        {
            printf("FFreeRTOSI2cLoopbackCreate error :0x%x!\n",ret);
            return ret;
        }
    }
#endif
    return ret;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), i2c, I2cCmdEntry, test freertos i2c driver);
