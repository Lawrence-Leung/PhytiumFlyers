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
 * FilePath: cmd_ddma.c
 * Date: 2022-07-14 14:06:43
 * LastEditTime: 2022-07-14 14:06:43
 * Description:  This file is for ddma command interface
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 zhugengyu    2022/08/26   first commit
 */
/***************************** Include Files *********************************/
#include <string.h>
#include <stdio.h>
#include "strto.h"
#include "sdkconfig.h"

#include "FreeRTOS.h"

#include "../src/shell.h"
#include "ddma_spi_loopback.h"
/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
#if defined(CONFIG_TARGET_E2000D) || defined(CONFIG_TARGET_E2000Q)
#define USED_SPI_ID FSPI2_ID
#else
#define USED_SPI_ID FSPI0_ID
#endif
/************************** Function Prototypes ******************************/

/*****************************************************************************/
static int DdmaCmdEntry(int argc, char *argv[])
{
    int ret = 0;
    u32 bytes = 32;
    u32 spi_id = USED_SPI_ID;

    if (!strcmp(argv[1], "spi-loopback"))
    {
        if (argc >= 3)
        {
            spi_id = (u32)simple_strtoul(argv[2], NULL, 10);
        }

        if (argc >= 4)
        {
            bytes = (u32)simple_strtoul(argv[3], NULL, 10);
        }

        ret = FFreeRTOSRunDDMASpiLoopback(spi_id, bytes);
    }

    return ret;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), ddma, DdmaCmdEntry, test freertos ddma driver);