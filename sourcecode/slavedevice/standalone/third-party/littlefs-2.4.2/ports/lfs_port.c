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
 * FilePath: lfs_port.c
 * Date: 2022-04-06 16:00:38
 * LastEditTime: 2022-04-06 16:00:38
 * Description:  This file is for little fs general port
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/4/7     init commit
 */

/***************************** Include Files *********************************/
#include "fkernel.h"
#include "fassert.h"
#include "fmemory_pool.h"
#include "fdebug.h"

#include "sdkconfig.h"

#include "lfs_port.h"
#ifdef CONFIG_LITTLE_FS_ON_FSPIM_SFUD
#include "fspim_lfs_port.h"
#endif

#ifdef CONFIG_LITTLE_FS_DRY_RUN
#include "lfs_testbd_port.h"
#include "lfs_filebd.h"
#endif
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FLFS_DEBUG_TAG "LFS-PORT"
#define FLFS_ERROR(format, ...)   FT_DEBUG_PRINT_E(FLFS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FLFS_WARN(format, ...)    FT_DEBUG_PRINT_W(FLFS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FLFS_INFO(format, ...)    FT_DEBUG_PRINT_I(FLFS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FLFS_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FLFS_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
static FMemp memp;
static boolean memp_is_ready = FALSE;
static u8 memp_buf[2 * SZ_1M] = {0};

/*****************************************************************************/
static int FLfsInitMemp(void)
{
    if (TRUE == memp_is_ready)
    {
        FLFS_ERROR("The memory pool has been initialized !!!");
        return FLFS_PORT_MEMP_ALREADY_INIT;
    }

    FError err = FMempInit(&memp, &memp_buf[0], &memp_buf[0] + sizeof(memp_buf));
    if (FMEMP_SUCCESS != err)
    {
        FLFS_ERROR("Memory pool initialization failure: 0x%x \r\n", err);
        return FLFS_PORT_MEMP_INIT_FAILED;
    }

    memp_is_ready = TRUE;
    FLFS_INFO("Memory pool initialized successfully !!!");
    return FLFS_PORT_OK;
}

static void FLfsDeInitMemp(void)
{
    if (FALSE == memp_is_ready)
    {
        FLFS_WARN("The memory pool has not been initialized !!!");
        return;
    }

    FMempRemove(&memp);
    memp_is_ready = FALSE;
    return;
}

void *FLfsMalloc(size_t sz)
{
    void *ptr = NULL;
    if (FALSE == memp_is_ready)
    {
        FLFS_ERROR("The memory pool has not been initialized !!!");
        return ptr;
    }

    ptr = FMempMalloc(&memp, sz);
    return ptr;
}

void FLfsFree(void *ptr)
{
    if (FALSE == memp_is_ready)
    {
        FLFS_ERROR("The memory pool has not been initialized !!!");
        return;
    }

    FMempFree(&memp, ptr);
    return;
}

int FLfsInitialize(FLfs *const instance, FLfsPortType type)
{
    FASSERT(instance);

    if (FT_COMPONENT_IS_READY == instance->lfs_ready)
    {
        FLFS_ERROR("Little-fs has been started");
        return FLFS_PORT_ALREADY_INITED;
    }

    int ret = FLfsInitMemp();
    if (FLFS_PORT_OK != ret)
        return ret;

#ifdef CONFIG_LITTLE_FS_DRY_RUN
    FASSERT_MSG(((FLFS_PORT_TO_DRY_RUN_IN_RAM == type) || (FLFS_PORT_TO_DRY_RUN_IN_FILE == type)), 
                "not dry run !!");
    ret = lfs_filebd_setup_fatfs();
    if (0 != ret)
    {
        FLFS_ERROR("Fatfs failed to initialize: %d", ret);
        ret = FLFS_PORT_FILEBD_INIT_FAILED;
        goto err_exit;
    }

    const char *test_path = (FLFS_PORT_TO_DRY_RUN_IN_FILE == type) ? "test_path.txt" : NULL;
    ret = FLfsTestBDInitialize(instance, test_path);
    if (0 != ret)
    {
        FLFS_ERROR("Failed to initialize tested: %d", ret);
        ret = FLFS_PORT_FILEBD_INIT_FAILED;
        goto err_exit;
    }
#endif

#ifdef CONFIG_LITTLE_FS_ON_FSPIM_SFUD
    FASSERT_MSG((FLFS_PORT_TO_FSPIM == type), "not spim !!");
    ret = FLfsSpimInitialize(instance);
    if (FLFS_FSPIM_PORT_OK != ret)
    {
        FLFS_ERROR("fspim sfud init failed: %d", ret);
        ret = FLFS_PORT_FSPIM_INIT_FAILED;
        goto err_exit;
    }
#endif

    instance->lfs_ready = FT_COMPONENT_IS_READY;

err_exit:
    if (FLFS_PORT_OK != ret)
    {
        FLfsDeInitMemp();
    }
    return ret;
}

void FLfsDeInitialize(FLfs *const instance)
{
    if (FT_COMPONENT_IS_READY != instance->lfs_ready)
    {
        FLFS_ERROR("Little-fs has not started yet");
        return;
    }

#ifdef CONFIG_LITTLE_FS_DRY_RUN_ON_FILE
    lfs_filebd_unsetup_fatfs();
    FLfsTestBDDeInitialize(instance);
#endif

#ifdef CONFIG_LITTLE_FS_ON_FSPIM_SFUD
    FLfsSpimDeInitialize(instance);
#endif

    FLfsDeInitMemp();
    instance->lfs_ready = 0;

    return;
}

const struct lfs_config *FLfsGetDefaultConfig(void)
{
#ifdef CONFIG_LITTLE_FS_ON_FSPIM_SFUD
    return FLfsSpimGetDefaultConfig();
#endif

#ifdef CONFIG_LITTLE_FS_DRY_RUN
    return FLfsTestBDGetDefaultConfig();
#endif
}