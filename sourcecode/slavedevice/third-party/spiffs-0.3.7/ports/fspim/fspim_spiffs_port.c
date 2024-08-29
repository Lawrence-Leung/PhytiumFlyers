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
 * FilePath: fspim_spiffs_port.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:24:47
 * Description:  This files is for providing spiffs func based on spi.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  liqiaozhong   2022/4/15    first commit, support Spiffs
 */

/***************************** Include Files *********************************/
#include <string.h>

#include "fkernel.h"
#include "fdebug.h"
#include "fassert.h"

#include "sfud.h"
#include "fspim_spiffs_port.h"

/************************** Constant Definitions *****************************/
#define FSPIFFS_FLASH_START_ADDR        0x0
#define FSPIFFS_FLASH_SIZE              SZ_1M
/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static boolean is_sfud_ready = FALSE;
static const sfud_flash *flash_instance = NULL;
#if defined(CONFIG_TARGET_E2000D)||defined(CONFIG_TARGET_E2000Q)
static const fsize_t flash_id = SFUD_FSPIM2_INDEX;
#elif defined(CONFIG_TARGET_PHYTIUMPI)
static const fsize_t flash_id = SFUD_FSPIM0_INDEX;
#endif

/***************** Macros (Inline Functions) Definitions *********************/
#define FSPIM_DEBUG_TAG "SPIM-SPIFFS"
#define FSPIM_ERROR(format, ...)   FT_DEBUG_PRINT_E(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_WARN(format, ...)    FT_DEBUG_PRINT_W(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_INFO(format, ...)    FT_DEBUG_PRINT_I(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/
static s32_t FSpiffsRead(struct spiffs_t *fs, u32_t addr, u32_t size, u8_t *buf)
{
    FASSERT_MSG(fs, "NULL fs instance");
    FASSERT_MSG(buf, "NULL buffer");
    if ((FALSE == is_sfud_ready) || (NULL == flash_instance))
    {
        FSPIM_ERROR("sfud not ready");
        return FSPIFFS_SPIM_PORT_SFUD_NOT_READY;
    }

    sfud_err result = sfud_read(flash_instance,
                                (u32)addr,
                                (fsize_t)size,
                                (u8 *)buf);
    if (SFUD_SUCCESS != result)
    {
        FSPIM_ERROR("Read failed: %d", result);
        return FSPIFFS_SPIM_PORT_SFUD_IO_ERROR;
    }

    FSPIM_DEBUG("sfud read success");
    return FSPIFFS_SPIM_PORT_OK;
}

static s32_t FSpiffsWrite(struct spiffs_t *fs, u32_t addr, u32_t size, u8_t *buf)
{
    FASSERT_MSG(fs, "NULL fs instance");
    FASSERT_MSG(buf, "NULL buffer");
    if ((FALSE == is_sfud_ready) || (NULL == flash_instance))
    {
        FSPIM_ERROR("sfud not ready");
        return FSPIFFS_SPIM_PORT_SFUD_NOT_READY;
    }

    sfud_err result = sfud_write(flash_instance,
                                 (u32)addr,
                                 (fsize_t)size,
                                 (const u8 *)buf);
    if (SFUD_SUCCESS != result)
    {
        FSPIM_ERROR("Write failed: %d", result);
        return FSPIFFS_SPIM_PORT_SFUD_IO_ERROR;
    }

    FSPIM_DEBUG("sfud write success");
    return FSPIFFS_SPIM_PORT_OK;
}

static s32_t FSpiffsErase(struct spiffs_t *fs, u32_t addr, u32_t size)
{
    FASSERT_MSG(fs, "NULL fs instance");
    if ((FALSE == is_sfud_ready) || (NULL == flash_instance))
    {
        FSPIM_ERROR("sfud not ready");
        return FSPIFFS_SPIM_PORT_SFUD_NOT_READY;
    }

    sfud_err result = sfud_erase(flash_instance,
                                 (u32)addr,
                                 (fsize_t)(size));
    if (SFUD_SUCCESS != result)
    {
        FSPIM_ERROR("Erase failed: %d", result);
        return FSPIFFS_SPIM_PORT_SFUD_IO_ERROR;
    }

    FSPIM_DEBUG("sfud erase success");
    return FSPIFFS_SPIM_PORT_OK;
}

int FSpiffsSpimInitialize(FSpiffs *const instance)
{
    FASSERT(instance);
    if (FT_COMPONENT_IS_READY == instance->fs_ready)
    {
        FSPIM_ERROR("little-fs already inited and mounted");
        return FSPIFFS_SPIM_PORT_ALREADY_INITED;
    }

    if ((TRUE == is_sfud_ready) || (NULL != flash_instance))
    {
        FSPIM_ERROR("sfud already inited");
        return FSPIFFS_SPIM_PORT_ALREADY_INITED;
    }

    int sfud_ret = sfud_init();
    if (SFUD_SUCCESS != sfud_ret)
    {
        FSPIM_ERROR("sfud init failed: %d", sfud_ret);
        return FSPIFFS_SPIM_PORT_SFUD_INIT_FAILED;
    }

    flash_instance = sfud_get_device(flash_id);
    if (NULL == flash_instance)
    {
        FSPIM_ERROR("Get sfud flash failed");
        return FSPIFFS_SPIM_PORT_SFUD_INIT_FAILED;
    }

    if ((flash_instance->chip.capacity < (instance->fs_addr + instance->fs_size)) ||
        (FSPIFFS_LOG_BLOCK_SIZE % flash_instance->chip.erase_gran))
    {
        FSPIM_ERROR("Flash not support !!! capacity %d < space %d, erase_gran %d %% %d != 0",
                    flash_instance->chip.capacity,
                    (instance->fs_addr + instance->fs_size),
                    FSPIFFS_LOG_BLOCK_SIZE,
                    flash_instance->chip.erase_gran);
        return FSPIFFS_SPIM_PORT_SFUD_INIT_FAILED;
    }

    if (flash_instance->chip.capacity < SZ_1M)
    {
        printf("%d KB %s is current selected device.\r\n",
               flash_instance->chip.capacity / SZ_1K,
               flash_instance->name);
    }
    else
    {
        printf("%d MB %s is current selected device.\r\n",
               flash_instance->chip.capacity / SZ_1M,
               flash_instance->name);
    }

    is_sfud_ready = TRUE;
    /* instance->fs_ready will be set after mount filesystem */;
    return FSPIFFS_SPIM_PORT_OK;
}

void FSpiffsSpimDeInitialize(FSpiffs *const instance)
{
    memset(instance, 0, sizeof(FSpiffs));
    is_sfud_ready = FALSE;
    return;
}

const spiffs_config *FSpiffsSpimGetDefaultConfig(void)
{
    static const spiffs_config cfg =
    {
        .phys_addr = FSPIFFS_FLASH_START_ADDR, /* start spiffs at start of spi flash */
        .phys_size = FSPIFFS_FLASH_SIZE, /* flash_capcity in use */
        .phys_erase_block = FSPIFFS_LOG_BLOCK_SIZE, /* according to datasheet */
        .log_block_size = FSPIFFS_LOG_BLOCK_SIZE, /* let us not complicate things */
        .log_page_size = FSPIFFS_LOG_PAGE_SIZE, /* as we said */
        .hal_read_f = FSpiffsRead,
        .hal_write_f = FSpiffsWrite,
        .hal_erase_f = FSpiffsErase
    };

    return (const spiffs_config *)&cfg;
}