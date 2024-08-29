
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
 * FilePath: sdmmc_system.c
 * Date: 2022-02-10 14:53:44
 * LastEditTime: 2022-02-25 11:47:44
 * Description:  This files is for sdmmc baremetal port
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/11/15  first release
 */
#include <string.h>

#include <FreeRTOS.h>
#include "fmemory_pool.h"

#include "sdmmc_system.h"

#define SDMMC_MEM_BUF_SIZE      SZ_1M
#define SDMMC_ALIGNMENT         64U
#define SDMMC_256_ALIGNMENT     256U

static u8 sdmmc_buf[SDMMC_MEM_BUF_SIZE] = {0};
static FMemp sdmmc_mem_pool;

void sdmmc_sys_init(void)
{
    memset(&sdmmc_mem_pool, 0, sizeof(sdmmc_mem_pool));

    FError result = FMempInit(&sdmmc_mem_pool, sdmmc_buf, sdmmc_buf + SDMMC_MEM_BUF_SIZE); /* init memory pool */
    SDMMC_ASSERT(result == FMEMP_SUCCESS);    
}

void sdmmc_sys_deinit(void)
{
    FASSERT(FT_COMPONENT_IS_READY == sdmmc_mem_pool.is_ready);
    FMempDeinit(&sdmmc_mem_pool);
}

void sdmmc_sys_delay_ms(uint32_t msecs)
{
    vTaskDelay(pdMS_TO_TICKS(msecs));
}

void *sdmmc_sys_heap_caps_malloc(size_t size, uint32_t caps)
{
    void *ptr = NULL;
    FASSERT(FT_COMPONENT_IS_READY == sdmmc_mem_pool.is_ready);

    if (caps & MALLOC_8_ALIGN)
    {
        ptr = FMempMallocAlign(&sdmmc_mem_pool, size, 8); /* allocate alligned memory for DMA access */
    }
    else if (caps & MALLOC_32_ALIGN)
    {
        ptr = FMempMallocAlign(&sdmmc_mem_pool, size, 32);
    }
    else if (caps & MALLOC_64_ALIGN)
    {
        ptr = FMempMallocAlign(&sdmmc_mem_pool, size, 64);
    }
    else if (caps & MALLOC_256_ALIGN)
    {
        ptr = FMempMallocAlign(&sdmmc_mem_pool, size, 256);
    }
    else if (caps & MALLOC_512_ALIGN)
    {
        ptr = FMempMallocAlign(&sdmmc_mem_pool, size, 512);
    }   
    else
    {
        ptr = FMempCalloc(&sdmmc_mem_pool, 1, size);
    }

    return ptr;
}

void sdmmc_sys_free(void *ptr)
{
    FASSERT(FT_COMPONENT_IS_READY == sdmmc_mem_pool.is_ready);
    FMempFree(&sdmmc_mem_pool, ptr);
}