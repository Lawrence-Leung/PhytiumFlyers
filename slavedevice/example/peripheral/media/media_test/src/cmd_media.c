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
 * FilePath: cmd_meida.c
 * Date: 2023-02-15 14:34:44
 * LastEditTime: 2023-02-16 14:34:45
 * Description:  This file is for media shell command.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangzq  2023/02/16  init commit
 */
#include <string.h>
#include <stdio.h>
#include "strto.h"
#include "sdkconfig.h"

#include "FreeRTOS.h"
#include "../src/shell.h"

#include "fdc_common_hw.h"
#include "media_example.h"

static InputParm input_config;

InputParm *InputParaReturn(void)
{
    return &input_config;
}
static void FFreeRTOSMediaCmdUsage(void)
{
    printf("Usage:\r\n");
    printf("    Media init <channel> <width> <height> <multi_mode> <color_depth> <refresh_rate\r\n");
    printf("        -- init media, run the demo\r\n");
    printf("        -- <channel> 0/1/2, 0:channel 0; 1:channel 1; 2: channel 0 and channel 1,all channel can use\r\n");
    printf("        -- <width> the  resolution of width\r\n");
    printf("        -- <height> the  resolution of height\r\n");
    printf("        -- <multi_mode>  the sigle screen or multi-display \r\n");
    printf("        -- <color_depth> the color_depth of screen ,default color_depth is 32\r\n");
    printf("        -- <refresh_rate> the refresh_rate of screen ,default refresh_rate is 60\r\n");
    printf("    Media deinit <channel>\r\n");
    printf("    Media demo\r\n");

}
static int MediaCmdEntry(int argc, char *argv[])
{
    u32 id ;
    static boolean inited = FALSE;
    if (argc < 2)
    {
        FFreeRTOSMediaCmdUsage();
        return -1;
    }
    if (!strcmp(argv[1], "init"))
    {
        if (argc >= 3)
        {
            input_config.channel  = (u32)simple_strtoul(argv[2], NULL, 10);
            if (input_config.channel > FDCDP_INSTANCE_NUM)
            {
                printf("please insert the correct num,such as 0,1 or 2 \r\n");
            }
            input_config.width = (u32)simple_strtoul(argv[3], NULL, 10);
            input_config.height = (u32)simple_strtoul(argv[4], NULL, 10);
            input_config.multi_mode = (u32)simple_strtoul(argv[5], NULL, 10);
            input_config.color_depth = (u32)simple_strtoul(argv[6], NULL, 10);
            input_config.refresh_rate = (u32)simple_strtoul(argv[7], NULL, 10);
        }
        else
        {
            input_config.channel = 0;
            input_config.width = 1024;
            input_config.height = 768;
            input_config.multi_mode = 0;
            input_config.color_depth = 32;
            input_config.refresh_rate = 60;
        }
        BaseType_t task_ret = FFreeRTOSMediaCreate(&input_config);

        if (pdPASS != task_ret)
        {
            return -2;
        }
        inited = TRUE;
    }
    if (!strcmp(argv[1], "demo"))
    {
        if (inited != TRUE)
        {
            printf("please ensure the media has been inited \r\n");
            return -2;
        }
        FMediaDisplayDemo();
    }
    if (!strcmp(argv[1], "deinit"))
    {
        if (inited != TRUE)
        {
            printf("please ensure the media has been inited \r\n");
            return -2;
        }
        if (argc >= 3)
        {
            id = (u32)simple_strtoul(argv[2], NULL, 10);
        }
        FFreeRTOSMediaChannelDeinit(id);
    }

    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), Media, MediaCmdEntry, test freertos media driver);