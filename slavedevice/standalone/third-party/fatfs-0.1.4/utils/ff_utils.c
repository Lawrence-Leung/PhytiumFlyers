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
 * FilePath: ff_utils.c
 * Date: 2022-07-21 13:21:43
 * LastEditTime: 2022-07-21 13:21:44
 * Description:  This file is for fatfs test utility implmentation
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/12/3   init commit
 */

#include <string.h>
#include "fdebug.h"
#include "fkernel.h"
#include "fassert.h"
#include "ferror_code.h"
#include "ff_utils.h"
#include "diskio.h"
#include "sdkconfig.h"

#define FF_DEBUG_TAG "FATFS"
#define FF_ERROR(format, ...)   FT_DEBUG_PRINT_E(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_INFO(format, ...)    FT_DEBUG_PRINT_I(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_WARN(format, ...)    FT_DEBUG_PRINT_W(FF_DEBUG_TAG, format, ##__VA_ARGS__)

void ff_dump_info(const TCHAR *mount_point)
{
    FATFS *fs = NULL;
    DWORD fre_clust, fre_sect, tot_sect;
    FRESULT res;
    TCHAR label[12]; /* volume label is at mote 12 bytes if Unicode not enabled */
    static const TCHAR *fs_type[] = {"", "fat12", "fat16", "fat32", "exfat"};    

    /* Get volume information and free clusters of drive 1 */
    res = f_getfree(mount_point, &fre_clust, &fs);
    if (res) 
        return;

    f_getlabel(mount_point, label, NULL);

    printf("Volume-%d:%s @%s\r\n", fs->pdrv, label, mount_point);
    printf("\tformat: %s\r\n", fs_type[fs->fs_type]);

    /* Get total sectors and free sectors */
    tot_sect = (fs->n_fatent - 2) * fs->csize;
    fre_sect = fre_clust * fs->csize;

    /* Print the free space (assuming 512 bytes/sector) */
    printf("\t%10lu KiB total drive space.\r\n\t%10lu KiB available.\r\n", 
            tot_sect / 2, 
            fre_sect / 2);

}

FRESULT ff_setup(ff_fatfs *fatfs, const TCHAR *mount_point , const MKFS_PARM *opt, UINT force_format)
{
    FASSERT(mount_point);
    FASSERT(fatfs);
    FRESULT res;
    FATFS *fs = &(fatfs->fs);
    BYTE *work = fatfs->work;
    BYTE pvol = 0;
    TCHAR label[12]; /* volume label is at mote 12 bytes if Unicode not enabled */

#ifdef CONFIG_FATFS_ALLOC_PREFER_MEMP
    static UINT memp_init = 0;
    extern void ff_mempinit(void);

    if (0 == memp_init)
    {
        ff_mempinit();
        memp_init = 1;
        FF_INFO("Memory poll init ok.");
    }
#endif

    if (FF_VOL_NOT_FOUND == ff_diskio_get_available_volume(mount_point, &pvol))
    {
        FF_ERROR("No available driver found !!!");
        return FR_INVALID_DRIVE;
    }

    if (!strcmp(mount_point, FF_RAM_DISK_MOUNT_POINT))
    {
#ifdef CONFIG_FATFS_RAM_DISK
        /* register ram disk */
        ff_diskio_register_ram(pvol);
        sprintf(label, "%s", "0:ram");
        FF_INFO("Register RAM disk success.");
#else
        return -1;
#endif
    }
    else if (!strcmp(mount_point, FF_FSDIO_TF_DISK_MOUNT_POINT))
    {
#ifdef CONFIG_FATFS_SDMMC_FSDIO_TF
        /* register tf disk */
        ff_diskio_register_fsdio_tf(pvol);
        sprintf(label, "%s", "1:tf");
        FF_INFO("Register TF card success.");
#else
        return -1;
#endif
    }
    else if (!strcmp(mount_point, FF_FSDIO_EMMC_DISK_MOUNT_POINT))
    {
#ifdef CONFIG_FATFS_SDMMC_FSDIO_EMMC
        /* register emmc disk */
        ff_diskio_register_fsdio_emmc(pvol);
        sprintf(label, "%s", "2:emmc");
        FF_INFO("Register eMMC success.");
#else
        return -1;
#endif
    }
    else if (!strcmp(mount_point, FF_FSDMMC_TF_DISK_MOUNT_POINT))
    {
#ifdef CONFIG_FATFS_SDMMC_FSDMMC_TF
        /* register tf card */
        ff_diskio_register_fsdmmc_tf(pvol);
        sprintf(label, "%s", "3:tf");
        FF_INFO("Register TF card success.");
#else
        return -1;
#endif        
    }
    else if (!strcmp(mount_point, FF_USB_DISK_MOUNT_POINT))
    {
#ifdef CONFIG_FATFS_USB
        /* register tf disk */
        ff_diskio_register_usb(pvol);
        sprintf(label, "%s", "4:usb");
        FF_INFO("Register USB disk success.");
#else
        return -1;
#endif      
    }
else if (!strcmp(mount_point, FF_SATA_DISK_MOUNT_POINT))
    {
#ifdef CONFIG_FATFS_FSATA
        /* register tf disk */
        ff_diskio_register_sata(pvol);
        sprintf(label, "%s", "5:sata");
        FF_INFO("Register sata disk success.");
#else
        return -1;
#endif      
    }
else if (!strcmp(mount_point, FF_SATA_PCIE_DISK_MOUNT_POINT))
    {
#ifdef CONFIG_FATFS_FSATA_PCIE
        /* register tf disk */
        ff_diskio_register_sata_pcie(pvol);
        sprintf(label, "%s", "6:sata pcie");
        FF_INFO("Register sata pcie disk success.");
#else
        return -1;
#endif      
    }
    /* force mounted the volume to check if it is ready to work. */
    printf("About to mount. \r\n");
    res = f_mount(fs, mount_point, 1);
    FF_INFO("Mount fatfs at %s, ret = %d", mount_point, res);
    if ((res == FR_NO_FILESYSTEM) || (force_format))
    {
        printf("No file system, formatting ...\r\n");

        /* make file system if none */
        res = f_mkfs(mount_point, opt, work, FF_MAX_SS * sizeof(BYTE));
        if (res == FR_OK)
        {
			FF_INFO("Format ok.");
            /* unregisters the registered filesystem */
			res = f_mount(NULL, mount_point, 1);
            /* register the filesystem */
			res = f_mount(fs, mount_point, 1);
			FF_INFO("Mount again ret = %d", res);            
        }
		else
		{
			FF_ERROR("Format fail, ret = %d", res);
            return res;
        }
    }
    else if (res != FR_OK)
    {
        FF_ERROR("File system mount fail.");
        return res;
    }
    else
	{
		printf("File system mount ok.\r\n");
	}

    (void)f_setlabel(label);

    memset(label, 0, sizeof(label));    
    f_getlabel(mount_point, label, NULL);
    printf("%s Setup success !!!\r\n", label);
    fatfs->is_ready = FT_COMPONENT_IS_READY;
    return res;
}