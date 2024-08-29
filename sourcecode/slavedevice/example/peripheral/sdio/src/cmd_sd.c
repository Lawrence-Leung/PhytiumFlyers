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
 * FilePath: cmd_sd.c
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
#include "sd_read_write.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
static void SdCmdUsage()
{
    printf("Usage:\r\n");
    printf("    sd wr <id> <medium> <start-blk> <blk-num>\r\n");
    printf("        -- Demo read and write by sdmmc\r\n");
}

static int SdCmdEntry(int argc, char *argv[])
{
    int ret = 0;

    if (argc < 2)
    {
        SdCmdUsage();
        return -1;
    }

    if (!strcmp(argv[1], "wr"))
    {
        u32 sdio_id = FSDIO1_ID;
        boolean is_emmc = FALSE;
        u32 start_blk = 0U;
        u32 blk_num = 2U;

        if (argc > 2)
        {
            sdio_id = (u32)simple_strtoul(argv[2], NULL, 10); /* sdio instance id */
        }

        if (argc > 3)
        {
            if (!strcmp(argv[3], "emmc")) /* probe medium as emmc */
            {
                is_emmc = TRUE;
            }
            else if (!strcmp(argv[3], "tf")) /* probe medium as tf card */
            {
                is_emmc = FALSE;
            }
        }

        if (argc > 4)
        {
            start_blk = (u32)simple_strtoul(argv[4], NULL, 10);
        }

        if (argc > 5)
        {
            blk_num = (u32)simple_strtoul(argv[5], NULL, 10);
        }

        BaseType_t task_ret = FFreeRTOSSdWriteRead(sdio_id, is_emmc, start_blk, blk_num);
        if (pdPASS != task_ret)
        {
            return -2;
        }
    }

    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), sd, SdCmdEntry, test freertos sd rw);