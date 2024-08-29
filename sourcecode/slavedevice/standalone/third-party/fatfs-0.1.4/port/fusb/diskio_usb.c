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
 * FilePath: diskio_usb.c
 * Date: 2022-07-21 13:21:43
 * LastEditTime: 2022-07-21 13:21:44
 * Description:  This file is for fatfs port to usb disk
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
#include "fparameters.h"
#include "fkernel.h"
#include "fsleep.h"
#include "diskio.h"
#include "ffconf.h"
#include "ff.h"
#include "fusb.h"
#include "fusb_hub.h"
#include "fusb_msc.h"

#define FF_DEBUG_TAG "DISKIO-USB"
#define FF_ERROR(format, ...)   FT_DEBUG_PRINT_E(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_INFO(format, ...)    FT_DEBUG_PRINT_I(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_WARN(format, ...)    FT_DEBUG_PRINT_W(FF_DEBUG_TAG, format, ##__VA_ARGS__)

#define FUSB_DISK_BLOCK_SIZE     512
#define FUSB_MAX_NUM_OF_DEV      10

typedef struct
{
    DWORD id;
    DWORD sector_sz;
    DWORD sector_cnt;
    FUsb usb;
    FUsbDev *usb_disk;
    boolean init_ok;
    BYTE pdrv;
} ff_usb_disk;

static ff_usb_disk usb_disk = 
{
    .pdrv = FF_DRV_NOT_USED,
    .init_ok = FALSE,
    .usb_disk = NULL
};

static int usb_add_hub_support(FUsb *usb)
{
    FError ret = FUSB_SUCCESS;
    FUsbDevIndex index;
    FUsbDevInitHandler handler = FUsbHubInit;

    index.category = FUSB_STANDARD_INTERFACE;
    index.class = FUSB_HUB_DEVICE;
    index.sub_class = 0;
    index.protocol = 0;

    ret = FUsbAssignDevInitFunc(usb, &index, handler);
    return (FUSB_SUCCESS == ret) ? 0 : -1;
}

static int usb_add_mass_storage_support(FUsb *usb)
{
    FError ret = FUSB_SUCCESS;
    FUsbDevIndex index;
    FUsbDevInitHandler handler = FUsbMassStorageInit;

    index.category = FUSB_STANDARD_INTERFACE;
    index.class = FUSB_MASS_STORAGE_DEVICE;
    index.sub_class = 0x6;
    index.protocol = 0x50;

    ret = FUsbAssignDevInitFunc(usb, &index, handler);
    return (FUSB_SUCCESS == ret) ? 0 : -1;    
}

static FUsbDev *usb_get_first_dev_of(FUsb *usb, const FUsbDevClass class)
{
    fsize_t devs_num;
    fsize_t loop;
    FUsbDev *target_dev = NULL;
    FUsbHc *hc = usb->hc;
	if (NULL == hc)
		return target_dev;

    FUsbDev **devs = FUSB_ALLOCATE(usb, sizeof(FUsbDev *) * FUSB_MAX_NUM_OF_DEV, FUSB_DEFAULT_ALIGN);
	if (NULL == devs)
        return target_dev;

    devs_num = FUsbGetAllDevEntries(hc, devs, FUSB_MAX_NUM_OF_DEV);
    FF_INFO("Total %d devices", devs_num);
    for (loop = 0; loop < devs_num; loop++)
    {
        FF_INFO("    dev@%p", devs[loop]);
        FF_INFO("        ep num %d", devs[loop]->num_endp);
        FF_INFO("        dev address 0x%x", devs[loop]->address);
        FF_INFO("        hub attached %d", devs[loop]->hub);
        FF_INFO("        port attached %d", devs[loop]->port);
        FF_INFO("        speed type %d", devs[loop]->speed);
        FF_INFO("        class 0x%x", devs[loop]->class);

        if (class == devs[loop]->class)
        {
			target_dev = devs[loop];
			break;
		}
	}

    FUSB_FREE(usb, devs);
    return target_dev;
}

static void *usb_align(size_t size, size_t align)
{
    return ff_memalign((UINT)size, (UINT)align);
}

static int usb_disk_setup(ff_usb_disk *disk)
{
    FError ret = FUSB_SUCCESS;
    FUsbConfig input_config;

    if (disk->init_ok)
        return 0;

    memset(&disk->usb, 0, sizeof(disk->usb));
    memset(&input_config, 0, sizeof(input_config));

    input_config = *FUsbLookupConfig(disk->id); /* lookup config if it is soc usb */    

    input_config.allocator.malloc_align = usb_align;
    input_config.allocator.free = ff_memfree;

    ret = FUsbCfgInitialize(&disk->usb, &input_config);
    if (FUSB_SUCCESS == ret)
    {
        if (0 != usb_add_hub_support(&disk->usb))
            return -3;

        if (0 != usb_add_mass_storage_support(&disk->usb))
            return -4;

    }    

    fsleep_millisec(100);

    FUsbPoll(&disk->usb);

    /* get first mass storage device, ignore if there are more */
    disk->usb_disk = usb_get_first_dev_of(&disk->usb, FUSB_MASS_STORAGE_DEVICE);

    if (NULL != disk->usb_disk)
    {
        disk->sector_cnt = FUsbMscGetBlkNum(disk->usb_disk);
        disk->sector_sz = FUsbMscGetBlkSize(disk->usb_disk);

        FF_INFO("init usb disk driver @%p ok", disk->usb_disk);
        printf("    capcity(total): %ld MB \r\n", FUsbMscGetCapcityMB(disk->usb_disk));
        printf("    block num(total): %ld \r\n", disk->sector_cnt);
        printf("    block size: %d \r\n", disk->sector_sz);

        disk->init_ok = TRUE;
        printf("Usb stated ok\r\n");
    }
    else
    {
        FF_ERROR("No usb disk found !!!");
        return -5;
    }

    return 0;
}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

static DSTATUS usb_disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS status = STA_NOINIT;
    ff_usb_disk *disk = &usb_disk;

	if (disk->init_ok)
		status &= ~STA_NOINIT;

	return status;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

static DSTATUS usb_disk_initialize(
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
    DSTATUS status = STA_NOINIT;
    ff_usb_disk *disk = &usb_disk;

    if (FF_DRV_NOT_USED == disk->pdrv)
    {
        return STA_NOINIT;
    }

    if (0 == usb_disk_setup(disk))
    {
        status &= ~STA_NOINIT;
    }

	return status;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

static DRESULT usb_disk_read (
                BYTE pdrv,		/* Physical drive nmuber to identify the drive */
                BYTE *buff,		/* Data buffer to store read data */
                DWORD sector,	/* Start sector in LBA */
                UINT count		/* Number of sectors to read */
)
{
    DRESULT status = RES_OK;
    ff_usb_disk *disk = &usb_disk;

    if ((disk->usb_disk) && (sector < disk->sector_cnt))
    {
        if (0 != FUsbMscRwBlk512(disk->usb_disk, sector, count, FUSB_DIR_DATA_IN, buff))
        {
            FF_ERROR("read usb sector [%d-%d] failed: 0x%x", sector, sector + count);
            status = RES_ERROR;
        }
    }
    else
    {
        status = RES_PARERR;
    }

    return status;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

static DRESULT usb_disk_write (
                    BYTE pdrv,			/* Physical drive nmuber to identify the drive */
                    const BYTE *buff,	/* Data to be written */
                    DWORD sector,		/* Start sector in LBA */
                    UINT count			/* Number of sectors to write */
)
{
    DRESULT status = RES_OK;
    ff_usb_disk *disk = &usb_disk;

    if ((disk->usb_disk) && (sector < disk->sector_cnt))
    {
        if (0 != FUsbMscRwBlk512(disk->usb_disk, sector, count, FUSB_DIR_DATA_OUT, (u8 *)buff))
        {
            FF_ERROR("read usb sector [%d-%d] failed: 0x%x", sector, sector + count);
            status = RES_ERROR;
        }
    }
    else
    {
        status = RES_PARERR;
    }

    return status;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT usb_disk_ioctl (
            BYTE pdrv,		/* Physical drive nmuber (0..) */
            BYTE cmd,		/* Control code */
            void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
    ff_usb_disk *disk = &usb_disk;

	res = RES_PARERR;
    if (NULL == disk->usb_disk)
    {
        return res;
    }

    switch (cmd)
    {
        case CTRL_SYNC:			/* Nothing to do */
            res = RES_OK;
        break;

        case GET_SECTOR_COUNT:	/* Get number of sectors on the drive */
            *(DWORD*)buff = FUsbMscGetBlkNum(disk->usb_disk);
            res = RES_OK;
        break;

        case GET_SECTOR_SIZE:	/* Get size of sector for generic read/write */
            *(WORD*)buff = FUsbMscGetBlkSize(disk->usb_disk);
            res = RES_OK;
        break;

        case GET_BLOCK_SIZE:
            *(DWORD*)buff = 1; /* This is not flash storage that can be erase by command, return 1 */
            res = RES_OK;
        break;
    }

    return res;
}

static const ff_diskio_driver_t usb_disk_drv = 
{
    .init = &usb_disk_initialize,
    .status = &usb_disk_status,
    .read = &usb_disk_read,
    .write = &usb_disk_write,
    .ioctl = &usb_disk_ioctl
};

void ff_diskio_register_usb(BYTE pdrv)
{
    ff_usb_disk *disk = &usb_disk;    

    disk->id = FUSB3_ID_0;
    disk->init_ok = FALSE;
    disk->usb_disk = NULL;
    disk->pdrv = pdrv; /* assign volume for usb disk */
    ff_diskio_register(pdrv, &usb_disk_drv);

    FF_INFO("Create usb disk as driver-%d", disk->pdrv);
}