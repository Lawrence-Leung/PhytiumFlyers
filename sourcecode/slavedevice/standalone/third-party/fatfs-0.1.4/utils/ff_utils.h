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
 * FilePath: ff_utils.h
 * Date: 2022-07-21 13:21:43
 * LastEditTime: 2022-07-21 13:21:44
 * Description:  This file is for fatfs test utility definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/12/3   init commit
 */

#ifndef  FF_UTILS_H
#define  FF_UTILS_H

#ifdef __cplusplus
extern "C"
{
#endif

/* file system */
#include "ff.h"

typedef struct
{
    FATFS fs;
    BYTE  work[FF_MAX_SS]; /* working buffer used for format process */
    DWORD is_ready;
} ff_fatfs;

FRESULT ff_setup(ff_fatfs *fatfs, const TCHAR *mount_point, const MKFS_PARM *opt, UINT force_format);
void ff_dump_info(const TCHAR *mount_point);
FRESULT ff_append_test (const TCHAR* path);
FRESULT ff_read_test (const TCHAR* path);
FRESULT ff_delete_test (const TCHAR* path);
FRESULT ff_list_test (TCHAR* path);

FRESULT ff_basic_test (const TCHAR* mount_point, const TCHAR* file_name);
int ff_cycle_test (const TCHAR *mount_point, UINT ncyc);
int ff_speed_bench(const TCHAR *mount_point, DWORD sectors);
FRESULT ff_big_file_test(const TCHAR* path, UINT sz_mb);

#ifdef __cplusplus
}
#endif

#endif