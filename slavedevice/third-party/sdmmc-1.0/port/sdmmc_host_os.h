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
 * FilePath: fsdio_port.h
 * Date: 2022-02-10 14:53:44
 * LastEditTime: 2022-02-25 11:46:22
 * Description:  This files is for sdmmc function definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/12/7   init commit
 */
#ifndef SDMMC_HOST_OS_H
#define SDMMC_HOST_OS_H

#pragma once

#include "sdmmc_host.h"
#include "sdmmc_cmd.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define SDMMC_LOCKED_CALL_NON_RET_NON_ARG(func, card) \
    do { \
        SDMMC_ASSERT(card); \
        if (SDMMC_OK == sdmmc_host_lock((sdmmc_host_t *)&(card->host))) {\
            func(card);\
        } \
        sdmmc_host_unlock((sdmmc_host_t *)&(card->host));\
    } while(0);

#define SDMMC_LOCKED_CALL_NON_ARG(func, card) \
    do { \
        SDMMC_ASSERT(card); \
        sdmmc_err_t err = SDMMC_OK; \
        if (SDMMC_OK == sdmmc_host_lock(&(card->host))) {\
            err = func(card);\
        } \
        sdmmc_host_unlock(&(card->host));\
        return err; \
    } while(0);

#define SDMMC_LOCKED_CALL_NON_RET(func, card, ...) \
    do { \
        SDMMC_ASSERT(card); \
        if (SDMMC_OK == sdmmc_host_lock(&(card->host))) {\
            func(card, ##__VA_ARGS__);\
        } \
        sdmmc_host_unlock(&(card->host));\
    } while(0);

#define SDMMC_LOCKED_CALL(func, card, ...) \
    do { \
        SDMMC_ASSERT(card); \
        sdmmc_err_t err = SDMMC_OK; \
        if (SDMMC_OK == sdmmc_host_lock(&(card->host))) {\
            err = func(card, ##__VA_ARGS__);\
        } \
        sdmmc_host_unlock(&(card->host));\
        return err; \
    } while(0);

/* thread-safe API warpper */
static inline void sdmmc_os_card_print_info(const sdmmc_card_t *card)
{
    SDMMC_LOCKED_CALL_NON_RET_NON_ARG(sdmmc_card_print_info, card);
}

static inline sdmmc_err_t sdmmc_os_get_status(sdmmc_card_t *card)
{
    SDMMC_LOCKED_CALL_NON_ARG(sdmmc_get_status, card);
}

static inline sdmmc_err_t sdmmc_os_write_sectors(sdmmc_card_t *card, const void *src,
                                   size_t start_sector, size_t sector_count)
{
    SDMMC_LOCKED_CALL(sdmmc_write_sectors, card, src, start_sector, sector_count);
}

static inline sdmmc_err_t sdmmc_os_read_sectors(sdmmc_card_t *card, void *dst,
                                  size_t start_sector, size_t sector_count)
{
    SDMMC_LOCKED_CALL(sdmmc_read_sectors, card, dst, start_sector, sector_count);
}

static inline sdmmc_err_t sdmmc_os_erase_sectors(sdmmc_card_t *card, size_t start_sector,
                                   size_t sector_count, sdmmc_erase_arg_t arg)
{
    SDMMC_LOCKED_CALL(sdmmc_erase_sectors, card, start_sector, sector_count, arg);
}

static inline sdmmc_err_t sdmmc_os_can_discard(sdmmc_card_t *card)
{
    SDMMC_LOCKED_CALL_NON_ARG(sdmmc_can_discard, card);
}

static inline sdmmc_err_t sdmmc_os_can_trim(sdmmc_card_t *card)
{
    SDMMC_LOCKED_CALL_NON_ARG(sdmmc_can_trim, card);
}

static inline sdmmc_err_t sdmmc_os_mmc_can_sanitize(sdmmc_card_t *card)
{
    SDMMC_LOCKED_CALL_NON_ARG(sdmmc_mmc_can_sanitize, card);
}

static inline sdmmc_err_t sdmmc_os_mmc_sanitize(sdmmc_card_t *card, uint32_t timeout_ms)
{
    SDMMC_LOCKED_CALL(sdmmc_mmc_sanitize, card, timeout_ms);
}

static inline sdmmc_err_t sdmmc_os_full_erase(sdmmc_card_t *card)
{
    SDMMC_LOCKED_CALL_NON_ARG(sdmmc_full_erase, card);
}

static inline sdmmc_err_t sdmmc_os_io_read_byte(sdmmc_card_t *card, uint32_t function,
                                  uint32_t reg, uint8_t *out_byte)
{
    SDMMC_LOCKED_CALL(sdmmc_io_read_byte, card, function, reg, out_byte);
}

static inline sdmmc_err_t sdmmc_os_io_write_byte(sdmmc_card_t *card, uint32_t function,
                                   uint32_t reg, uint8_t in_byte, uint8_t *out_byte)
{
    SDMMC_LOCKED_CALL(sdmmc_io_write_byte, card, function, reg, in_byte, out_byte);
}

static inline sdmmc_err_t sdmmc_os_io_read_bytes(sdmmc_card_t *card, uint32_t function,
                                   uint32_t addr, void *dst, size_t size)
{
    SDMMC_LOCKED_CALL(sdmmc_io_read_bytes, card, function, addr, dst, size);
}

static inline sdmmc_err_t sdmmc_os_io_write_bytes(sdmmc_card_t *card, uint32_t function,
                                    uint32_t addr, const void *src, size_t size)
{
    SDMMC_LOCKED_CALL(sdmmc_io_write_bytes, card, function, addr, src, size);
}

static inline sdmmc_err_t sdmmc_os_io_read_blocks(sdmmc_card_t *card, uint32_t function,
                                    uint32_t addr, void *dst, size_t size)
{
    SDMMC_LOCKED_CALL(sdmmc_io_read_blocks, card, function, addr, dst, size);
}

static inline sdmmc_err_t sdmmc_os_io_write_blocks(sdmmc_card_t *card, uint32_t function,
                                     uint32_t addr, const void *src, size_t size)
{
    SDMMC_LOCKED_CALL(sdmmc_io_write_blocks, card, function, addr, src, size);
}

static inline sdmmc_err_t sdmmc_os_io_enable_int(sdmmc_card_t *card)
{
    SDMMC_LOCKED_CALL_NON_ARG(sdmmc_io_enable_int, card);
}

static inline sdmmc_err_t sdmmc_os_io_wait_int(sdmmc_card_t *card, tick_type_t timeout_ticks)
{
    SDMMC_LOCKED_CALL(sdmmc_io_wait_int, card, timeout_ticks);
}

static inline sdmmc_err_t sdmmc_os_io_get_cis_data(sdmmc_card_t *card, uint8_t *out_buffer, size_t buffer_size, size_t *inout_cis_size)
{
    SDMMC_LOCKED_CALL(sdmmc_io_get_cis_data, card, out_buffer, buffer_size, inout_cis_size);
}

#ifdef __cplusplus
}
#endif

#endif