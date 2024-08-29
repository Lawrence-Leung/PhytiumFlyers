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
 * FilePath: lfs_port.h
 * Date: 2022-04-06 16:00:32
 * LastEditTime: 2022-04-06 16:00:32
 * Description:  This file is for little fs general port
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/4/7     init commit
 */
#ifndef  LITTLE_FS_PORT_H
#define  LITTLE_FS_PORT_H


/***************************** Include Files *********************************/
#include "ftypes.h"
#include "lfs.h"

#ifdef __cplusplus
extern "C"
{
#endif

/************************** Constant Definitions *****************************/
enum
{
    FLFS_PORT_OK = 0,

    FLFS_PORT_ALREADY_INITED,
    FLFS_PORT_MEMP_ALREADY_INIT,
    FLFS_PORT_MEMP_INIT_FAILED,
    FLFS_PORT_FILEBD_INIT_FAILED,
    FLFS_PORT_FSPIM_INIT_FAILED,
    FLFS_PORT_IO_ERR,
};

typedef enum
{
    FLFS_PORT_TO_FSPIM = 0,
    FLFS_PORT_TO_DRY_RUN_IN_RAM,
    FLFS_PORT_TO_DRY_RUN_IN_FILE
} FLfsPortType;

/**************************** Type Definitions *******************************/
typedef struct lfs lfs_t;
typedef struct
{
    lfs_t lfs;
    u32   lfs_ready;
} FLfs;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
int FLfsInitialize(FLfs *const instance, FLfsPortType type);
void FLfsDeInitialize(FLfs *const instance);
const struct lfs_config *FLfsGetDefaultConfig(void);

#ifdef __cplusplus
}
#endif

#endif