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
 * FilePath: diskio.c
 * Date: 2022-07-21 13:21:43
 * LastEditTime: 2022-07-21 13:21:44
 * Description:  This files is for fatfs port glue implmentation
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/12/3   init commit
 */

/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>

#include "fdebug.h"
#include "fassert.h"

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

#define FF_DEBUG_TAG "FATFS"
#define FF_ERROR(format, ...)   FT_DEBUG_PRINT_E(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_INFO(format, ...)    FT_DEBUG_PRINT_I(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_WARN(format, ...)    FT_DEBUG_PRINT_W(FF_DEBUG_TAG, format, ##__VA_ARGS__)


static const ff_diskio_driver_t *diskio_drv_list[FF_VOLUMES] = {NULL};

#if FF_MULTI_PARTITION		/* Multiple partition configuration */
PARTITION VolToPart[] = {
    {0, 0},    /* Logical drive 0 ==> Physical drive 0, auto detection */
    {1, 0},    /* Logical drive 1 ==> Physical drive 1, auto detection */
#if FF_VOLUMES > 2
    {2, 0},     /* Logical drive 2 ==> Physical drive 2, auto detection */
#endif
#if FF_VOLUMES > 3
    {3, 0},     /* Logical drive 3 ==> Physical drive 3, auto detection */
#endif
#if FF_VOLUMES > 4
    {4, 0},     /* Logical drive 4 ==> Physical drive 4, auto detection */
#endif
#if FF_VOLUMES > 5
    {5, 0},     /* Logical drive 5 ==> Physical drive 5, auto detection */
#endif
#if FF_VOLUMES > 6
    {6, 0},     /* Logical drive 6 ==> Physical drive 6, auto detection */
#endif
#if FF_VOLUMES > 7
    {7, 0},     /* Logical drive 7 ==> Physical drive 7, auto detection */
#endif
#if FF_VOLUMES > 8
    {8, 0},     /* Logical drive 8 ==> Physical drive 8, auto detection */
#endif
#if FF_VOLUMES > 9
    {9, 0},     /* Logical drive 9 ==> Physical drive 9, auto detection */
#endif
};
#endif /* FATFS declared to support at most 10 volumes */

int ff_diskio_get_available_volume(const TCHAR *mount_point, BYTE* out_pdrv)
{
    int ret = FF_VOL_NOT_FOUND;

    if (!strcmp(mount_point, FF_RAM_DISK_MOUNT_POINT))
    {
        *out_pdrv = 0;
        ret = FF_VOL_FOUND;
    }
    else if (!strcmp(mount_point, FF_FSDIO_TF_DISK_MOUNT_POINT))
    {
        *out_pdrv = 1;
        ret = FF_VOL_FOUND;
    }
    else if (!strcmp(mount_point, FF_FSDIO_EMMC_DISK_MOUNT_POINT))
    {
#if FF_VOLUMES > 2
        *out_pdrv = 2;
        ret = FF_VOL_FOUND;
#endif
    }
    else if (!strcmp(mount_point, FF_FSDMMC_TF_DISK_MOUNT_POINT))
    {
#if FF_VOLUMES > 3
        *out_pdrv = 3;
        ret = FF_VOL_FOUND;
#endif        
    }
    else if (!strcmp(mount_point, FF_USB_DISK_MOUNT_POINT))
    {
#if FF_VOLUMES > 4
        *out_pdrv = 4;
        ret = FF_VOL_FOUND;
#endif        
    }
    else if (!strcmp(mount_point, FF_SATA_DISK_MOUNT_POINT))
    {
#if FF_VOLUMES > 5
        *out_pdrv = 5;
        ret = FF_VOL_FOUND;
#endif        
    }
else if (!strcmp(mount_point, FF_SATA_PCIE_DISK_MOUNT_POINT))
    {
#if FF_VOLUMES > 6
        *out_pdrv = 6;
        ret = FF_VOL_FOUND;
#endif        
    }

    return ret;	
}

void ff_diskio_register(BYTE pdrv, const ff_diskio_driver_t* diskio_drv)
{
    FASSERT_MSG(pdrv < FF_VOLUMES, "pdrv = %d", pdrv);

    if (diskio_drv_list[pdrv]) /* remove exist driver */
	{
        const ff_diskio_driver_t* im = diskio_drv_list[pdrv];
        diskio_drv_list[pdrv] = NULL;
    }

    if (!diskio_drv) 
	{
        return;
    }

    diskio_drv_list[pdrv] = diskio_drv;

    FF_DEBUG("pdrv = %d", pdrv);
    for (BYTE i = 0; i < FF_VOLUMES; i++)
    {
        FF_DEBUG("drv[%d] @%p", i, diskio_drv_list[i]);
    }

	return;
}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	FASSERT_MSG(diskio_drv_list[pdrv] != NULL, "pdrv = %d", pdrv);
	return diskio_drv_list[pdrv]->status(pdrv); /* init disk */
}


/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	FASSERT_MSG(diskio_drv_list[pdrv] != NULL, "pdrv = %d", pdrv);
    return diskio_drv_list[pdrv]->init(pdrv); /* init disk */	
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	FASSERT_MSG(diskio_drv_list[pdrv] != NULL, "pdrv = %d", pdrv);
	return diskio_drv_list[pdrv]->read(pdrv, buff, sector, count);
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	FASSERT_MSG(diskio_drv_list[pdrv] != NULL, "pdrv = %d", pdrv);
	return diskio_drv_list[pdrv]->write(pdrv, buff, sector, count);
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	FASSERT_MSG(diskio_drv_list[pdrv] != NULL, "pdrv = %d", pdrv);
	return diskio_drv_list[pdrv]->ioctl(pdrv, cmd, buff);
}

