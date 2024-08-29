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
 * FilePath: spiffs_port.h
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:24:52
 * Description:  This files is for giving tolal spiffs init api.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangxiaodong 2022/8/9   first release
 */

#ifndef SPIFFS_PORT_H
#define SPIFFS_PORT_H

/***************************** Include Files *********************************/
#include "ftypes.h"

#include "spiffs.h"
/************************** Constant Definitions *****************************/

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    FSPIFFS_PORT_TO_FSPIM = 0,
    FSPIFFS_PORT_TO_FQSPI = 1,
} FSpiffsPortType;

enum
{
    FSPIFFS_PORT_OK = 0,
};

/**************************** Type Definitions *******************************/
typedef struct spiffs_t spiffs;
typedef struct
{
    spiffs fs;
    u32    fs_ready;
    u32    fs_addr;
    u32    fs_size;
} FSpiffs;
/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int FSpiffsInitialize(FSpiffs *const instance, FSpiffsPortType type);
void FSpiffsDeInitialize(FSpiffs *const instance);
const spiffs_config *FSpiffsGetDefaultConfig(void);

#ifdef __cplusplus
}
#endif

#endif
