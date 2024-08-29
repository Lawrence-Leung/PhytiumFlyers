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
 * FilePath: cmd_sf.c
 * Date: 2022-07-12 09:33:12
 * LastEditTime: 2022-07-12 09:33:12
 * Description:  This file is for providing user command functions.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  zhugengyu  2022/8/26    first commit
 */
/***************************** Include Files *********************************/
#include <string.h>
#include <stdio.h>
#include "strto.h"
#include "sdkconfig.h"

#include "FreeRTOS.h"

#include "../src/shell.h"
#include "sfud_read_write.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
static void SfudCmdUsage()
{
    printf("Usage:\r\n");
    printf("    sf probe\r\n");
    printf("        -- Probe and init SPI flash\r\n");
    printf("    sf rw <inchip-addr>\r\n");
    printf("        -- Demo read and write by sfud\r\n");
}

static int SfudCmdEntry(int argc, char *argv[])
{
    int ret = 0;
    static boolean inited = FALSE;

    if (argc < 2)
    {
        SfudCmdUsage();
        return -1;
    }

    if ((FALSE == inited) || (!strcmp(argv[1], "probe")))
    {
        if (pdPASS != FFreeRTOSSfudInit())
        {
            return -2;
        }

        inited = TRUE;
    }

    if (!strcmp(argv[1], "read"))
    {
        u32 in_chip_addr = 0x0;

        if (argc > 2)
        {
            in_chip_addr = (u32)simple_strtoul(argv[2], NULL, 16);
        }

        BaseType_t task_ret = FFreeRTOSSfudRead(in_chip_addr);
        if (pdPASS != task_ret)
        {
            return -2;
        }
    }
    else if (!strcmp(argv[1], "write"))
    {
        u32 in_chip_addr = 0x0;
        const char *wr_str = "write flash by sfud";

        if (argc > 2)
        {
            in_chip_addr = (u32)simple_strtoul(argv[2], NULL, 16);
        }

        if (argc > 3)
        {
            wr_str = argv[3];
        }

        BaseType_t task_ret = FFreeRTOSSfudWrite(in_chip_addr, wr_str);
        if (pdPASS != task_ret)
        {
            return -2;
        }
    }

    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), sf, SfudCmdEntry, test freertos sfud);