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
 * FilePath: fspim_lfs_port.h
 * Date: 2022-04-06 16:07:32
 * LastEditTime: 2022-04-06 16:07:33
 * Description:  This file is for little fs spim port
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/4/7     init commit
 */
#ifndef  LITTLE_FS_FSPIM_PORT_H
#define  LITTLE_FS_FSPIM_PORT_H


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
    FLFS_FSPIM_PORT_OK = 0,
    FLFS_FSPIM_PORT_INIT_SFUD_FAILED,
    FLFS_FSPIM_PORT_SFUD_NOT_READY,
};

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
int FLfsSpimInitialize(FLfs *const instance);
void FLfsSpimDeInitialize(FLfs *const instance);
const struct lfs_config *FLfsSpimGetDefaultConfig(void);

#ifdef __cplusplus
}
#endif

#endif