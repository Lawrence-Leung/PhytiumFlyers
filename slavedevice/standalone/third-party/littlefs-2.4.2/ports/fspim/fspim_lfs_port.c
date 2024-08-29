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
 * FilePath: fspim_lfs_port.c
 * Date: 2022-04-06 16:07:42
 * LastEditTime: 2022-04-06 16:07:43
 * Description:  This file is for little fs spim port
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
#include "fspim_lfs_port.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FLFS_DEBUG_TAG "LFS-FSPIM-PORT"
#define FLFS_ERROR(format, ...)   FT_DEBUG_PRINT_E(FLFS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FLFS_WARN(format, ...)    FT_DEBUG_PRINT_W(FLFS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FLFS_INFO(format, ...)    FT_DEBUG_PRINT_I(FLFS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FLFS_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FLFS_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/
static int FLfsSpimRead(const struct lfs_config *cfg, lfs_block_t block,
						lfs_off_t off, void *buffer, lfs_size_t size);
static int FLfsSpimWrite(const struct lfs_config *cfg, lfs_block_t block,
						 lfs_off_t off, const void *buffer, lfs_size_t size);
static int FLfsSpimErase(const struct lfs_config *cfg, lfs_block_t block);
static int FLfsSpimSync(const struct lfs_config *cfg);
/************************** Variable Definitions *****************************/
static boolean is_sfud_ready = FALSE;
static const sfud_flash *flash_instance = NULL;
static const fsize_t flash_id = SFUD_FSPIM2_INDEX;

/*****************************************************************************/
int FLfsSpimInitialize(FLfs *const instance)
{
    FASSERT(instance);
    if (FT_COMPONENT_IS_READY == instance->lfs_ready)
    {
        FLFS_ERROR("Little-fs is already initialized");
        return FLFS_PORT_ALREADY_INITED;
    }
    
	if ((TRUE == is_sfud_ready) || (NULL != flash_instance))
	{
        FLFS_ERROR("The Sfud is already initialized");
        return FLFS_PORT_ALREADY_INITED;		
	}

	int sfud_ret = sfud_init();
	if (SFUD_SUCCESS != sfud_ret)
	{
		FLFS_ERROR("Sfud failed to initialize: %d", sfud_ret);
		return FLFS_FSPIM_PORT_INIT_SFUD_FAILED;
	}

	flash_instance = sfud_get_device(flash_id);
	if (NULL == flash_instance)
	{
		FLFS_ERROR("Failed to obtain sfud flash");
		return FLFS_FSPIM_PORT_INIT_SFUD_FAILED;		
	}

	if (flash_instance->chip.capacity < SZ_1M)
    {
        printf("%d KB %s is the currently selected device.\r\n", 
			   flash_instance->chip.capacity / SZ_1K, 
			   flash_instance->name);
    }
    else
    {
        printf("%d MB %s is the currently selected device.\r\n", 
			   flash_instance->chip.capacity / SZ_1M, 
			   flash_instance->name);        
    }

	is_sfud_ready = TRUE;
	return FLFS_FSPIM_PORT_OK;
}

void FLfsSpimDeInitialize(FLfs *const instance)
{
	FASSERT(instance);
	/* sfud do not provide de-init api */
	is_sfud_ready = FALSE;
	flash_instance = NULL;
}

const struct lfs_config *FLfsSpimGetDefaultConfig(void)
{
	static const struct lfs_config config =
	{
		/* block device operations */
		.read = FLfsSpimRead,
		.prog = FLfsSpimWrite,
		.erase = FLfsSpimErase,
		.sync = FLfsSpimSync,

		/* block device configuration */
		.read_size = 16,
		.prog_size = 16,
		.block_size = 4096,
		.block_count = 128,
		.block_cycles = 500, /* wear-leveling, 在一个块上写500次后，会将数据拷贝到其它块继续写 */
		.cache_size = 16,
		.lookahead_size = 16
	};

	return &config;	
}

static int FLfsSpimRead(const struct lfs_config *cfg, lfs_block_t block,
                    	lfs_off_t off, void *buffer, lfs_size_t size)
{
    FASSERT_MSG(off % cfg->read_size == 0, "Invalid flash read offset");
    FASSERT_MSG(size % cfg->read_size == 0, "Invalid flash read size");
    FASSERT_MSG(block < cfg->block_count, "Invalid flash block index");
	if ((FALSE == is_sfud_ready) || (NULL == flash_instance))
	{
		FLFS_ERROR("sfud is not ready");
		return FLFS_FSPIM_PORT_SFUD_NOT_READY;
	}

	sfud_err result = sfud_read(flash_instance, 
								(u32)(block * cfg->block_size + off),
								(fsize_t)size, 
								(u8 *)buffer);
	if (SFUD_SUCCESS != result)
    {
        FLFS_ERROR("read failure: %d", result);
        return FLFS_PORT_IO_ERR;
    }

    FLFS_DEBUG("sfud read successfully");
    return FLFS_PORT_OK;
}

static int FLfsSpimWrite(const struct lfs_config *cfg, lfs_block_t block,
                    	 lfs_off_t off, const void *buffer, lfs_size_t size)
{
    FASSERT_MSG(off % cfg->read_size == 0, "Invalid flash write offset");
    FASSERT_MSG(size % cfg->read_size == 0, "Invalid flash write size");
    FASSERT_MSG(block < cfg->block_count, "Invalid flash block index"); 
	if ((FALSE == is_sfud_ready) || (NULL == flash_instance))
	{
		FLFS_ERROR("Sfud is not ready");
		return FLFS_FSPIM_PORT_SFUD_NOT_READY;
	}
	
	sfud_err result = sfud_write(flash_instance, 
								(u32)(block * cfg->block_size + off), 
								(fsize_t)size, 
								(const u8 *)buffer);
	if (SFUD_SUCCESS != result)
    {
        FLFS_ERROR("Write failure: %d", result);
        return FLFS_PORT_IO_ERR;
    }

    FLFS_DEBUG("sfud was written successfully");
    return FLFS_PORT_OK;
}

static int FLfsSpimErase(const struct lfs_config *cfg, lfs_block_t block)
{
	FASSERT_MSG(block < cfg->block_count, "Invalid flash block index");
	if ((FALSE == is_sfud_ready) || (NULL == flash_instance))
	{
		FLFS_ERROR("sfud is not ready yet");
		return FLFS_FSPIM_PORT_SFUD_NOT_READY;
	}

	sfud_err result = sfud_erase(flash_instance, 
								(u32)(block * cfg->block_size),
								(fsize_t)(cfg->block_size));
	if (SFUD_SUCCESS != result)
    {
        FLFS_ERROR("Erasure failure: %d", result);
        return FLFS_PORT_IO_ERR;
    }

    FLFS_DEBUG("The sfud erasure was successful");
    return FLFS_PORT_OK;
}

static int FLfsSpimSync(const struct lfs_config *cfg)
{
	/* dummy sync, every write is synced */
    return FLFS_PORT_OK;
}