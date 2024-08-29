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
 * FilePath: lfs_testbd_port.h
 * Date: 2022-04-07 08:41:31
 * LastEditTime: 2022-04-07 08:41:31
 * Description:  This file is for little fs test bd port
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/4/7     init commit
 */
#ifndef  LITTLE_FS_DRY_RUN_PORT_H
#define  LITTLE_FS_DRY_RUN_PORT_H


/***************************** Include Files *********************************/
#include "lfs.h"
#include "lfs_port.h"

#ifdef __cplusplus
extern "C"
{
#endif

/************************** Constant Definitions *****************************/
enum
{
    FLFS_DRY_RUN_PORT_OK = 0,
    FLFS_DRY_RUN_PORT_INIT_FAILED,
};
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
int FLfsTestBDInitialize(FLfs *const instance, const char* lfs_testbd_path); 
void FLfsTestBDDeInitialize(FLfs *const instance);
const struct lfs_config *FLfsTestBDGetDefaultConfig(void);

#ifdef __cplusplus
}
#endif

#endif