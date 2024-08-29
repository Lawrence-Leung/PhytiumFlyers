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
 * FilePath: diskio_ram.c
 * Date: 2022-07-21 13:21:43
 * LastEditTime: 2022-07-21 13:21:44
 * Description:  This file is for fatfs port to ramdisk
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/12/3   init commit
 */

/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include "fdebug.h"
#include "fkernel.h"
#include "diskio.h"
#include "ffconf.h"
#include "ff.h"

#define FF_DEBUG_TAG "DISKIO-RAM"
#define FF_ERROR(format, ...)   FT_DEBUG_PRINT_E(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_INFO(format, ...)    FT_DEBUG_PRINT_I(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_WARN(format, ...)    FT_DEBUG_PRINT_W(FF_DEBUG_TAG, format, ##__VA_ARGS__)

typedef struct
{
    void  *base;
    DWORD sector_sz;
    DWORD sector_cnt;
    BYTE pdrv;
} ff_ram_disk;

static ff_ram_disk ram_disk = 
{
    .base = (void *)(uintptr)CONFIG_FATFS_RAM_DISK_BASE,
    .sector_sz = CONFIG_FATFS_RAM_DISK_SECTOR_SIZE_BYTE,
    .sector_cnt = CONFIG_FATFS_RAM_DISK_SIZE_MB * SZ_1M / 
                                    CONFIG_FATFS_RAM_DISK_SECTOR_SIZE_BYTE,
    .pdrv = FF_DRV_NOT_USED
};

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

static DSTATUS ram_disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
    ff_ram_disk *disk = &ram_disk;

    if (FF_DRV_NOT_USED == disk->pdrv)
    {
        return STA_NOINIT;
    }

	return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

static DSTATUS ram_disk_initialize(
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

static DRESULT ram_disk_read (
                BYTE pdrv,		/* Physical drive nmuber to identify the drive */
                BYTE *buff,		/* Data buffer to store read data */
                DWORD sector,	/* Start sector in LBA */
                UINT count		/* Number of sectors to read */
)
{
    ff_ram_disk *disk = &ram_disk;

    if ((FF_DRV_NOT_USED != disk->pdrv) && (sector < disk->sector_cnt))
    {
        memcpy(buff, disk->base + sector * disk->sector_sz, count * disk->sector_sz);
        return RES_OK;
    }

    return RES_PARERR;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

static DRESULT ram_disk_write (
                    BYTE pdrv,			/* Physical drive nmuber to identify the drive */
                    const BYTE *buff,	/* Data to be written */
                    DWORD sector,		/* Start sector in LBA */
                    UINT count			/* Number of sectors to write */
)
{
    ff_ram_disk *disk = &ram_disk;

    if ((FF_DRV_NOT_USED != disk->pdrv) && (sector < disk->sector_cnt))
    {
        memcpy(disk->base + sector * disk->sector_sz, buff, count * disk->sector_sz);
        return RES_OK;
    }

    return RES_PARERR;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT ram_disk_ioctl (
            BYTE pdrv,		/* Physical drive nmuber (0..) */
            BYTE cmd,		/* Control code */
            void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
    ff_ram_disk *disk = &ram_disk;

	res = RES_PARERR;
    if (FF_DRV_NOT_USED == disk->pdrv)
    {
        return res;
    }

    switch (cmd)
    {
        case CTRL_SYNC:			/* Nothing to do */
            res = RES_OK;
        break;

        case GET_SECTOR_COUNT:	/* Get number of sectors on the drive */
            *(DWORD*)buff = disk->sector_cnt;
            res = RES_OK;
        break;

        case GET_SECTOR_SIZE:	/* Get size of sector for generic read/write */
            *(WORD*)buff = disk->sector_sz;
            res = RES_OK;
        break;

        case GET_BLOCK_SIZE:
            *(DWORD*)buff = 1; /* This is not flash storage that can be erase by command, return 1 */
            res = RES_OK;
        break;
    }

    return res;
}

static const ff_diskio_driver_t ram_disk_drv = 
{
    .init = &ram_disk_initialize,
    .status = &ram_disk_status,
    .read = &ram_disk_read,
    .write = &ram_disk_write,
    .ioctl = &ram_disk_ioctl
};

void ff_diskio_register_ram(BYTE pdrv)
{
    ff_ram_disk *disk = &ram_disk;    

    disk->pdrv = pdrv; /* assign volume for ram disk */
    ff_diskio_register(pdrv, &ram_disk_drv);

    printf("Create ram disk @[0x%p ~ 0x%p] as driver-%d\r\n", 
            disk->base, 
            disk->base + disk->sector_sz * disk->sector_cnt,
            disk->pdrv);
}