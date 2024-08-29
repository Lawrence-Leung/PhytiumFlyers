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
 * FilePath: sdmmc_host.h
 * Date: 2022-02-10 14:53:44
 * LastEditTime: 2022-02-25 11:46:22
 * Description:  This file is for sdmmc function definition
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/12/5   init commit
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

#include "sdmmc_common.h"
#include "sdmmc_system.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    SDMMC_HOST_TYPE_FSDIO = 0,
    SDMMC_HOST_TYPE_FSDMMC,

    SDMMC_HOST_TYPE_NUM
} sdmmc_host_type_t;

typedef enum
{
    SDMMC_SDR_12 = 0,
    SDMMC_SDR_25
} sdmmc_bus_speed_type_t;

typedef struct
{
    int slot;
    sdmmc_host_type_t type;
    sdmmc_bus_speed_type_t bus_speed_mode;                 /* Single Date Rate */
    uint32_t flags;                                /* Features used by this slot */
#define SDMMC_HOST_WORK_MODE_DMA        (0x1 << 0) /* 1: in dma mode, 0: in pio mode */
#define SDMMC_HOST_WORK_MODE_IRQ        (0x1 << 1) /* 1: in irq mode, 0: in poll mode */
#define SDMMC_HOST_REMOVABLE_CARD       (0x1 << 2) /* 1: removable card, tf, 0: non-removable, eMMC */
#define SDMMC_HOST_IO_SUPPORT           (0x1 << 3) /* 1: sdio card, not supported yet */
} sdmmc_host_config_t;

typedef struct
{
    sdmmc_card_t    card; /* instance of card in sdmmc */
    sdmmc_host_t    host; /* instance of host in sdmmc */
    void            *private; /* private data for host */
    sdmmc_host_config_t config; /* current active configs */
    uint32_t        is_ready;
} sdmmc_host_instance_t;

/**
 * @brief Initialize SDMMC host peripheral
 *
 * @note This function is not thread safe
 *
 *
 * @param instance  sdmmc host instance
 * @param config  sdmmc host config
 * @return
 *      - SDMMC_OK on success
 *      - SDMMC_ERR_INVALID_STATE if sdmmc_host_init was already called
 *      - SDMMC_ERR_NO_MEM if memory can not be allocated
 */
sdmmc_err_t sdmmc_host_init(sdmmc_host_instance_t *const instance, const sdmmc_host_config_t* config);

/**
 * @brief DeInitialize SDMMC host peripheral
 *
 * @note This function is not thread safe
 *
 *
 * @param instance  sdmmc host instance
 * @return
 *      - SDMMC_OK on success
 *      - SDMMC_ERR_INVALID_STATE if sdmmc_host_init was already called
 *      - SDMMC_ERR_NO_MEM if memory can not be allocated
 */
sdmmc_err_t sdmmc_host_deinit(sdmmc_host_instance_t *const instance);


/**
 * @brief Lock SDMMC host so that other thread/task cannot touch the host
 *
 * @note Call this function before call API in sdmmc_cmd.h
 *
 *
 * @param instance  sdmmc host instance
 * @return
 *      - SDMMC_OK on success
 */
sdmmc_err_t sdmmc_host_lock(sdmmc_host_t *host);

/**
 * @brief Unlock SDMMC host so that other thread/task can touch the host
 *
 * @note Call this function after call API in sdmmc_cmd.h
 *
 *
 * @param instance  sdmmc host instance 
 */
void sdmmc_host_unlock(sdmmc_host_t *host);

#ifdef __cplusplus
}
#endif