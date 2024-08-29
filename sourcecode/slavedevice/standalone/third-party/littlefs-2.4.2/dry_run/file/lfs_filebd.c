/*
 * Block device emulated in a file
 *
 * Copyright (c) 2017, Arm Limited. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include "fdebug.h"
#include "lfs_filebd.h"

#define FFILEDB_DEBUG_TAG "LFS_FILEBD"
#define FFILEDB_ERROR(format, ...)   FT_DEBUG_PRINT_E(FFILEDB_DEBUG_TAG, format, ##__VA_ARGS__)
#define FFILEDB_WARN(format, ...)    FT_DEBUG_PRINT_W(FFILEDB_DEBUG_TAG, format, ##__VA_ARGS__)
#define FFILEDB_INFO(format, ...)    FT_DEBUG_PRINT_I(FFILEDB_DEBUG_TAG, format, ##__VA_ARGS__)
#define FFILEDB_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FFILEDB_DEBUG_TAG, format, ##__VA_ARGS__)

#define FFILEDB_MOUNT_POINT      "0:/"

static FATFS fatfs;
static BYTE work[FF_MAX_SS];
static BYTE buff[FF_MAX_SS];
static boolean fatfs_ok = FALSE;

int lfs_filebd_setup_fatfs(void)
{
    FRESULT res;
    UINT fnum;
    FIL file_handler;
    const char *mount_point = FFILEDB_MOUNT_POINT;

    res = f_mount(&fatfs, mount_point, 1);
    FFILEDB_INFO("mount fatfs at %s, ret = %d", mount_point, res);
    if (res == FR_NO_FILESYSTEM)
    {
        FFILEDB_WARN("no file system, formatting ...");

        res = f_mkfs(mount_point, FM_FAT, 0, work, sizeof(work));
		if (res == FR_OK)
		{
			FFILEDB_INFO("format ok");
			res = f_mount(NULL, mount_point, 1);
			res = f_mount(&fatfs, mount_point, 1);
			FFILEDB_INFO("mount again ret = %d", res);
		}
		else
		{
			FFILEDB_ERROR("format fail");
            return -1;
        }
    }
    else if (res != FR_OK)
    {
        FFILEDB_ERROR("file system mount fail");
        return -2;
    }
    else
	{
		FFILEDB_INFO("file system mount ok");
	}

    fatfs_ok = TRUE;
    return 0;
}

void lfs_filebd_unsetup_fatfs(void)
{
    fatfs_ok = FALSE;
}

int lfs_filebd_createcfg(const struct lfs_config *cfg, const char *path,
                         const struct lfs_filebd_config *bdcfg) 
{
    printf("lfs bd test on file \r\n");
    FFILEDB_DEBUG("lfs_filebd_createcfg(%p {.context=%p, "
                ".read=%p, .prog=%p, .erase=%p, .sync=%p, "
                ".read_size=%"PRIu32", .prog_size=%"PRIu32", "
                ".block_size=%"PRIu32", .block_count=%"PRIu32"}, "
                "\"%s\", "
                "%p {.erase_value=%"PRId32"})",
            (void*)cfg, cfg->context,
            (void*)(uintptr_t)cfg->read, (void*)(uintptr_t)cfg->prog,
            (void*)(uintptr_t)cfg->erase, (void*)(uintptr_t)cfg->sync,
            cfg->read_size, cfg->prog_size, cfg->block_size, cfg->block_count,
            path, (void*)bdcfg, bdcfg->erase_value);
    lfs_filebd_t *bd = cfg->context;
    bd->cfg = bdcfg;
    static FIL file;

    // open file
    memset(&bd->fd, 0, sizeof(bd->fd));
    FRESULT err = f_open(&bd->fd, path, FA_READ | FA_WRITE | FA_OPEN_ALWAYS);
    if (err != FR_OK) {
        FFILEDB_ERROR("lfs_filebd_createcfg -> %d", err);
        return -1;
    }

    FFILEDB_DEBUG("lfs_filebd_createcfg -> %d", 0);
    return 0;
}

int lfs_filebd_create(const struct lfs_config *cfg, const char *path) {
    FFILEDB_DEBUG("lfs_filebd_create(%p {.context=%p, "
                ".read=%p, .prog=%p, .erase=%p, .sync=%p, "
                ".read_size=%"PRIu32", .prog_size=%"PRIu32", "
                ".block_size=%"PRIu32", .block_count=%"PRIu32"}, "
                "\"%s\")",
            (void*)cfg, cfg->context,
            (void*)(uintptr_t)cfg->read, (void*)(uintptr_t)cfg->prog,
            (void*)(uintptr_t)cfg->erase, (void*)(uintptr_t)cfg->sync,
            cfg->read_size, cfg->prog_size, cfg->block_size, cfg->block_count,
            path);
    static const struct lfs_filebd_config defaults = {.erase_value=-1};
    int err = lfs_filebd_createcfg(cfg, path, &defaults);
    FFILEDB_DEBUG("lfs_filebd_create -> %d", err);
    return err;
}

int lfs_filebd_destroy(const struct lfs_config *cfg) {
    FFILEDB_DEBUG("lfs_filebd_destroy(%p)", (void*)cfg);
    lfs_filebd_t *bd = cfg->context;
    FRESULT err = f_close(&bd->fd);
    if (err != FR_OK) {
        FFILEDB_ERROR("lfs_filebd_destroy -> %d", err);
        return -1;
    }
    FFILEDB_DEBUG("lfs_filebd_destroy -> %d", 0);
    return 0;
}

int lfs_filebd_read(const struct lfs_config *cfg, lfs_block_t block,
        lfs_off_t off, void *buffer, lfs_size_t size) {
    FFILEDB_DEBUG("lfs_filebd_read(%p, "
                "0x%"PRIx32", %"PRIu32", %p, %"PRIu32")",
            (void*)cfg, block, off, buffer, size);
    lfs_filebd_t *bd = cfg->context;

    // check if read is valid
    LFS_ASSERT(off  % cfg->read_size == 0);
    LFS_ASSERT(size % cfg->read_size == 0);
    LFS_ASSERT(block < cfg->block_count);

    // zero for reproducibility (in case file is truncated)
    if (bd->cfg->erase_value != -1) {
        memset(buffer, bd->cfg->erase_value, size);
    }

    // read
    FRESULT err = f_lseek(&bd->fd, (FSIZE_t)block*cfg->block_size + (FSIZE_t)off);
    if (err != FR_OK)
    {
        FFILEDB_ERROR("lfs_filebd_read -> %d", err);
        return err;
    }

    UINT rd_bytes = 0;
    err = f_read(&bd->fd, buffer, size, &rd_bytes);
    if ((err != FR_OK) || 
        ((rd_bytes < size * 1) && !f_eof(&bd->fd)))
    {
        FFILEDB_ERROR("lfs_filebd_read -> %d", err);
        return err;
    }

    FFILEDB_DEBUG("lfs_filebd_read -> %d", 0);
    return 0;
}

int lfs_filebd_prog(const struct lfs_config *cfg, lfs_block_t block,
        lfs_off_t off, const void *buffer, lfs_size_t size) {
    FFILEDB_DEBUG("lfs_filebd_prog(%p, 0x%"PRIx32", %"PRIu32", %p, %"PRIu32")",
            (void*)cfg, block, off, buffer, size);
    lfs_filebd_t *bd = cfg->context;
    FRESULT err = FR_OK;
    UINT rd_bytes = 0;
    UINT wr_bytes = 0;

    // check if write is valid
    LFS_ASSERT(off  % cfg->prog_size == 0);
    LFS_ASSERT(size % cfg->prog_size == 0);
    LFS_ASSERT(block < cfg->block_count);

    // check that data was erased? only needed for testing
    if (bd->cfg->erase_value != -1) {
        err = f_lseek(&bd->fd, (FSIZE_t)block*cfg->block_size + (FSIZE_t)off);
        if (err != FR_OK) {
            FFILEDB_ERROR("lfs_filebd_prog -> %d", err);
            return err;
        }

        for (lfs_off_t i = 0; i < size; i++) {
            uint8_t c;
            err = f_read(&bd->fd, &c, 1, &rd_bytes);;
            if ((err != FR_OK) || ((rd_bytes < 1) && !f_eof(&bd->fd)))
            {
                FFILEDB_ERROR("lfs_filebd_prog -> %d", err);
                return err;
            }

            LFS_ASSERT(c == bd->cfg->erase_value);
        }
    }

    // program data
    err = f_lseek(&bd->fd, (FSIZE_t)block*cfg->block_size + (FSIZE_t)off);
    if (err != FR_OK) {
        FFILEDB_ERROR("lfs_filebd_prog -> %d", err);
        return err;
    }

    err = f_write(&bd->fd, buffer, size, &wr_bytes);
    if ((err != FR_OK) || (wr_bytes < size)) 
    {
        FFILEDB_ERROR("lfs_filebd_prog -> %d", err);
        return err;
    }

    FFILEDB_DEBUG("lfs_filebd_prog -> %d", 0);
    return 0;
}

int lfs_filebd_erase(const struct lfs_config *cfg, lfs_block_t block) {
    FFILEDB_DEBUG("lfs_filebd_erase(%p, 0x%"PRIx32")", (void*)cfg, block);
    lfs_filebd_t *bd = cfg->context;
    FRESULT err = FR_OK;
    UINT wr_bytes = 0;

    // check if erase is valid
    LFS_ASSERT(block < cfg->block_count);

    // erase, only needed for testing
    if (bd->cfg->erase_value != -1) {
        err = f_lseek(&bd->fd, (FSIZE_t)block*cfg->block_size);
        if (err != FR_OK) {
            FFILEDB_ERROR("lfs_filebd_erase -> %d", err);
            return err;
        }

        for (lfs_off_t i = 0; i < cfg->block_size; i++) {
            wr_bytes = 0;
            err = f_write(&bd->fd, &(uint8_t){bd->cfg->erase_value}, 1, &wr_bytes);
            if (err != FR_OK) {
                FFILEDB_ERROR("lfs_filebd_erase -> %d", err);
                return err;
            }
        }
    }

    FFILEDB_DEBUG("lfs_filebd_erase -> %d", 0);
    return 0;
}

int lfs_filebd_sync(const struct lfs_config *cfg) {
    FFILEDB_DEBUG("lfs_filebd_sync(%p)", (void*)cfg);
    // file sync
    lfs_filebd_t *bd = cfg->context;
    FRESULT err = f_sync(&bd->fd);
    if (err != FR_OK) {
        FFILEDB_ERROR("lfs_filebd_sync -> %d", 0);
        return err;
    }

    FFILEDB_DEBUG("lfs_filebd_sync -> %d", 0);
    return 0;
}
