/*
 * Block device emulated in RAM
 *
 * Copyright (c) 2017, Arm Limited. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include "fdebug.h"
#include "lfs_rambd.h"

#define FRAMDB_DEBUG_TAG "LFS_RAMBD"
#define FRAMDB_ERROR(format, ...)   FT_DEBUG_PRINT_E(FRAMDB_DEBUG_TAG, format, ##__VA_ARGS__)
#define FRAMDB_WARN(format, ...)    FT_DEBUG_PRINT_W(FRAMDB_DEBUG_TAG, format, ##__VA_ARGS__)
#define FRAMDB_INFO(format, ...)    FT_DEBUG_PRINT_I(FRAMDB_DEBUG_TAG, format, ##__VA_ARGS__)
#define FRAMDB_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FRAMDB_DEBUG_TAG, format, ##__VA_ARGS__)

int lfs_rambd_createcfg(const struct lfs_config *cfg, const struct lfs_rambd_config *bdcfg) 
{
    printf("lfs bd test on ram \r\n");
    FRAMDB_DEBUG("lfs_rambd_createcfg(%p {.context=%p, "
                ".read=%p, .prog=%p, .erase=%p, .sync=%p, "
                ".read_size=%"PRIu32", .prog_size=%"PRIu32", "
                ".block_size=%"PRIu32", .block_count=%"PRIu32"}, "
                "%p {.erase_value=%"PRId32", .buffer=%p})",
            (void*)cfg, cfg->context,
            (void*)(uintptr_t)cfg->read, (void*)(uintptr_t)cfg->prog,
            (void*)(uintptr_t)cfg->erase, (void*)(uintptr_t)cfg->sync,
            cfg->read_size, cfg->prog_size, cfg->block_size, cfg->block_count,
            (void*)bdcfg, bdcfg->erase_value, bdcfg->buffer);
    lfs_rambd_t *bd = cfg->context;
    bd->cfg = bdcfg;

    // allocate buffer?
    if (bd->cfg->buffer) {
        bd->buffer = bd->cfg->buffer;
    } else {
        bd->buffer = lfs_malloc(cfg->block_size * cfg->block_count);
        if (!bd->buffer) {
            FRAMDB_ERROR("lfs_rambd_createcfg -> %d", LFS_ERR_NOMEM);
            return LFS_ERR_NOMEM;
        }
    }

    // zero for reproducibility?
    if (bd->cfg->erase_value != -1) {
        memset(bd->buffer, bd->cfg->erase_value,
                cfg->block_size * cfg->block_count);
    } else {
        memset(bd->buffer, 0, cfg->block_size * cfg->block_count);
    }

    FRAMDB_DEBUG("lfs_rambd_createcfg -> %d", 0);
    return 0;
}

int lfs_rambd_create(const struct lfs_config *cfg) 
{
    FRAMDB_DEBUG("lfs_rambd_create(%p {.context=%p, "
                ".read=%p, .prog=%p, .erase=%p, .sync=%p, "
                ".read_size=%"PRIu32", .prog_size=%"PRIu32", "
                ".block_size=%"PRIu32", .block_count=%"PRIu32"})",
            (void*)cfg, cfg->context,
            (void*)(uintptr_t)cfg->read, (void*)(uintptr_t)cfg->prog,
            (void*)(uintptr_t)cfg->erase, (void*)(uintptr_t)cfg->sync,
            cfg->read_size, cfg->prog_size, cfg->block_size, cfg->block_count);
    static const struct lfs_rambd_config defaults = {.erase_value=-1};
    int err = lfs_rambd_createcfg(cfg, &defaults);
    FRAMDB_DEBUG("lfs_rambd_create -> %d", err);
    return err;
}

int lfs_rambd_destroy(const struct lfs_config *cfg) 
{
    FRAMDB_DEBUG("lfs_rambd_destroy(%p)", (void*)cfg);
    // clean up memory
    lfs_rambd_t *bd = cfg->context;
    if (!bd->cfg->buffer) {
        lfs_free(bd->buffer);
    }
    FRAMDB_DEBUG("lfs_rambd_destroy -> %d", 0);
    return 0;
}

int lfs_rambd_read(const struct lfs_config *cfg, lfs_block_t block,
                   lfs_off_t off, void *buffer, lfs_size_t size) 
{
    FRAMDB_DEBUG("lfs_rambd_read(%p, "
                "0x%"PRIx32", %"PRIu32", %p, %"PRIu32")",
            (void*)cfg, block, off, buffer, size);
    lfs_rambd_t *bd = cfg->context;

    // check if read is valid
    LFS_ASSERT(off  % cfg->read_size == 0);
    LFS_ASSERT(size % cfg->read_size == 0);
    LFS_ASSERT(block < cfg->block_count);

    // read data
    memcpy(buffer, &bd->buffer[block*cfg->block_size + off], size);

    FRAMDB_DEBUG("lfs_rambd_read -> %d", 0);
    return 0;
}

int lfs_rambd_prog(const struct lfs_config *cfg, lfs_block_t block,
                   lfs_off_t off, const void *buffer, lfs_size_t size) 
{
    FRAMDB_DEBUG("lfs_rambd_prog(%p, "
                "0x%"PRIx32", %"PRIu32", %p, %"PRIu32")",
            (void*)cfg, block, off, buffer, size);
    lfs_rambd_t *bd = cfg->context;

    // check if write is valid
    LFS_ASSERT(off  % cfg->prog_size == 0);
    LFS_ASSERT(size % cfg->prog_size == 0);
    LFS_ASSERT(block < cfg->block_count);

    // check that data was erased? only needed for testing
    if (bd->cfg->erase_value != -1) {
        for (lfs_off_t i = 0; i < size; i++) {
            LFS_ASSERT(bd->buffer[block*cfg->block_size + off + i] ==
                    bd->cfg->erase_value);
        }
    }

    // program data
    memcpy(&bd->buffer[block*cfg->block_size + off], buffer, size);

    FRAMDB_DEBUG("lfs_rambd_prog -> %d", 0);
    return 0;
}

int lfs_rambd_erase(const struct lfs_config *cfg, lfs_block_t block) {
    FRAMDB_DEBUG("lfs_rambd_erase(%p, 0x%"PRIx32")", (void*)cfg, block);
    lfs_rambd_t *bd = cfg->context;

    // check if erase is valid
    LFS_ASSERT(block < cfg->block_count);

    // erase, only needed for testing
    if (bd->cfg->erase_value != -1) {
        memset(&bd->buffer[block*cfg->block_size],
                bd->cfg->erase_value, cfg->block_size);
    }

    FRAMDB_DEBUG("lfs_rambd_erase -> %d", 0);
    return 0;
}

int lfs_rambd_sync(const struct lfs_config *cfg) {
    FRAMDB_DEBUG("lfs_rambd_sync(%p)", (void*)cfg);
    // sync does nothing because we aren't backed by anything real
    (void)cfg;
    FRAMDB_DEBUG("lfs_rambd_sync -> %d", 0);
    return 0;
}
