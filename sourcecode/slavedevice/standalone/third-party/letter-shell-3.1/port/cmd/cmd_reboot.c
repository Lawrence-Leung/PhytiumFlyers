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
 * FilePath: cmd_reboot.c
 * Date: 2022-02-10 14:53:43
 * LastEditTime: 2022-02-25 11:47:19
 * Description:  This file is for reboot command implmentation
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0    huanghe  2021-10-18   init commit
 */

#include "../src/shell.h"
#include "ftypes.h"
#include "fpsci.h"

int RebootCmdEntry()
{
    FPsciSystemReset(FPSCI_SYSTEM_RESET_TYPE_COLD);
    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), reboot, 
                 RebootCmdEntry, reboot board by psci);