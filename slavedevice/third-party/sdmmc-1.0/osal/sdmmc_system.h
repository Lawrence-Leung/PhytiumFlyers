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
 * FilePath: sdmmc_system.h
 * Date: 2022-02-10 14:53:44
 * LastEditTime: 2022-02-25 11:46:22
 * Description:  This files is for sdmmc baremetal port
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/11/15  first release
 */

#ifndef SDMMC_SYSTEM_H
#define SDMMC_SYSTEM_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "ftypes.h"
#include "fassert.h"
#include "fdebug.h"
#include "fprintf.h"

/* Definitions for portable marco. */
#ifdef BIT
#undef BIT
#endif
#define BIT(x) (1U << (x))

#ifndef MAX
#undef MAX
#endif
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#ifndef MIN
#undef MIN
#endif
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define SDMMC_ASSERT(a)         FASSERT(a)
#define WORD_ALIGNED_ATTR       __attribute__((aligned(4)))
#define SDMMC_PTR_DMA_CAP(a)    !((unsigned long)a % 512) 
#define true                    TRUE 
#define false                   FALSE
#define MALLOC_8_ALIGN          (1U << 0)
#define MALLOC_32_ALIGN         (1U << 1)
#define MALLOC_64_ALIGN         (1U << 2)
#define MALLOC_256_ALIGN        (1U << 3)
#define MALLOC_512_ALIGN        (1U << 4)
#define MALLOC_CAP_DMA          MALLOC_512_ALIGN
#define MALLOC_CAP_DESC         MALLOC_256_ALIGN

#define SDMMC_PRINTF            printf

/* Definitions for portable types. */
typedef int sdmmc_err_t;
typedef int bool;
typedef unsigned long tick_type_t;

#define SDMMC_LOGE( tag, format, ... ) LOG_EARLY_IMPL(tag, format, FT_LOG_ERROR, E, ##__VA_ARGS__)
#define SDMMC_LOGW( tag, format, ... ) LOG_EARLY_IMPL(tag, format, FT_LOG_WARN, W, ##__VA_ARGS__)
#define SDMMC_LOGI( tag, format, ... ) LOG_EARLY_IMPL(tag, format, FT_LOG_INFO, I, ##__VA_ARGS__)
#define SDMMC_LOGD( tag, format, ... ) LOG_EARLY_IMPL(tag, format, FT_LOG_DEBUG, D, ##__VA_ARGS__)
#define SDMMC_LOGV( tag, format, ... ) LOG_EARLY_IMPL(tag, format, FT_LOG_VERBOSE, W, ##__VA_ARGS__)

#define SDMMC_LOG_BUFFER_HEXDUMP( tag, buffer, buff_len, level ) FtDumpHexWord((u32 *)(buffer), (u32)(buff_len))

void sdmmc_sys_init(void);
void sdmmc_sys_deinit(void);
void sdmmc_sys_delay_ms(uint32_t msecs);
void *sdmmc_sys_heap_caps_malloc(size_t size, uint32_t caps);
void sdmmc_sys_free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif