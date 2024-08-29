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
 * FilePath: lfs_filebd.h
 * Date: 2022-04-06 15:53:35
 * LastEditTime: 2022-04-06 16:17:02
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */
/*
 * Block device emulated in a file
 *
 * Copyright (c) 2017, Arm Limited. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef LFS_FILEBD_H
#define LFS_FILEBD_H

#include "ff.h"

#include "lfs.h"
#include "lfs_util.h"

#ifdef __cplusplus
extern "C"
{
#endif

// filebd config (optional)
struct lfs_filebd_config {
    // 8-bit erase value to use for simulating erases. -1 does not simulate
    // erases, which can speed up testing by avoiding all the extra block-device
    // operations to store the erase value.
    int32_t erase_value;
};

// filebd state
typedef struct lfs_filebd {
    FIL fd;
    const struct lfs_filebd_config *cfg;
} lfs_filebd_t;

// Setup fatfs to support filebd
int lfs_filebd_setup_fatfs(void);
void lfs_filebd_unsetup_fatfs(void);

// Create a file block device using the geometry in lfs_config
int lfs_filebd_create(const struct lfs_config *cfg, const char *path);
int lfs_filebd_createcfg(const struct lfs_config *cfg, const char *path,
        const struct lfs_filebd_config *bdcfg);

// Clean up memory associated with block device
int lfs_filebd_destroy(const struct lfs_config *cfg);

// Read a block
int lfs_filebd_read(const struct lfs_config *cfg, lfs_block_t block,
        lfs_off_t off, void *buffer, lfs_size_t size);

// Program a block
//
// The block must have previously been erased.
int lfs_filebd_prog(const struct lfs_config *cfg, lfs_block_t block,
        lfs_off_t off, const void *buffer, lfs_size_t size);

// Erase a block
//
// A block must be erased before being programmed. The
// state of an erased block is undefined.
int lfs_filebd_erase(const struct lfs_config *cfg, lfs_block_t block);

// Sync the block device
int lfs_filebd_sync(const struct lfs_config *cfg);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
