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
 * FilePath: lfs_testbd_port.c
 * Date: 2022-04-07 08:41:39
 * LastEditTime: 2022-04-07 08:41:39
 * Description:  This file is for little fs testbd port
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/4/7     init commit
 */

/***************************** Include Files *********************************/
#include "fkernel.h"
#include "fassert.h"
#include "fdebug.h"

#include "sdkconfig.h"

#include "sfud.h"
#include "lfs_testbd.h"
#include "lfs_testbd_port.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FLFS_DEBUG_TAG "LFS-TESTBD-PORT"
#define FLFS_ERROR(format, ...)   FT_DEBUG_PRINT_E(FLFS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FLFS_WARN(format, ...)    FT_DEBUG_PRINT_W(FLFS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FLFS_INFO(format, ...)    FT_DEBUG_PRINT_I(FLFS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FLFS_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FLFS_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
const struct lfs_testbd_config bdcfg = 
{
    .erase_value        = 255,
    .erase_cycles       = 1000000,  /* 假设一个块擦写1000000次后会变成坏块 */
    .badblock_behavior  = LFS_TESTBD_BADBLOCK_PROGERROR,
    .power_cycles       = 1000000,  /* 假设1000000次后出现一个掉电事件 */
};

/*****************************************************************************/
const struct lfs_config *FLfsTestBDGetDefaultConfig(void)
{
    static lfs_testbd_t bd;
    static const struct lfs_config config = 
    {
        .context        = &bd,
        .read           = lfs_testbd_read,
        .prog           = lfs_testbd_prog,
        .erase          = lfs_testbd_erase,
        .sync           = lfs_testbd_sync,
        .read_size      = 16,
        .prog_size      = 16,
        .block_size     = 512,
        .block_count    = 256,
        .block_cycles   = 16,
        .cache_size     = 16,
        .lookahead_size = 16,
    };

    return &config;    
}

int FLfsTestBDInitialize(FLfs *const instance, const char* lfs_testbd_path)
{
    FASSERT(instance);

    const struct lfs_config *config = FLfsTestBDGetDefaultConfig();
    int ret = FLFS_DRY_RUN_PORT_OK;
    int err = lfs_testbd_createcfg(config, lfs_testbd_path, &bdcfg);
    
    if (0 != err)
    {
        FLFS_ERROR("Failed to create tested: %d", err);
        ret = FLFS_DRY_RUN_PORT_INIT_FAILED;
    }

    return ret;
}

void FLfsTestBDDeInitialize(FLfs *const instance)
{
    FASSERT(instance);

    const struct lfs_config *config = FLfsTestBDGetDefaultConfig();
    int err = lfs_testbd_destroy(config);
    if (0 != err)
    {
        FLFS_ERROR("Failed to destroy tested: %d", err);
    }

    return;
}