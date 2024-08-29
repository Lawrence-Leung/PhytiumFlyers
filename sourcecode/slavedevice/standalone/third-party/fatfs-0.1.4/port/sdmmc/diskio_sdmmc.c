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
 * FilePath: diskio_sdmmc.c
 * Date: 2022-07-21 13:21:43
 * LastEditTime: 2022-07-21 13:21:44
 * Description:  This file is for fatfs port to sdmmc card
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
#include "fparameters.h"
#include "fdebug.h"
#include "fkernel.h"
#include "diskio.h"
#include "ffconf.h"
#include "ff.h"

#if defined(CONFIG_USE_BAREMETAL)
#include "sdmmc_host.h"

#elif defined(CONFIG_USE_FREERTOS)
#include "sdmmc_host_os.h"
#endif


#define FF_DEBUG_TAG "DISKIO-SDIO"
#define FF_ERROR(format, ...)   FT_DEBUG_PRINT_E(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_INFO(format, ...)    FT_DEBUG_PRINT_I(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_WARN(format, ...)    FT_DEBUG_PRINT_W(FF_DEBUG_TAG, format, ##__VA_ARGS__)

typedef struct
{
    DWORD id;
    DWORD start_blk;
    DWORD sector_sz;
    DWORD sector_cnt;
    DWORD pdrv;
    boolean init_ok;
    sdmmc_host_instance_t sd;
} ff_sdmmc_disk;

static ff_sdmmc_disk sdio_tf_disk = 
{
    .pdrv = FF_DRV_NOT_USED
};

static ff_sdmmc_disk sdio_emmc_disk = 
{
    .pdrv = FF_DRV_NOT_USED
};

static ff_sdmmc_disk sdmmc_tf_disk = 
{
    .pdrv = FF_DRV_NOT_USED
};

static ff_sdmmc_disk * get_sdio_disk(BYTE pdrv)
{
    if (sdio_tf_disk.pdrv == pdrv)
    {
        return &sdio_tf_disk;
    }
    else if (sdio_emmc_disk.pdrv == pdrv)
    {
        return &sdio_emmc_disk;
    }
    else if (sdmmc_tf_disk.pdrv == pdrv)
    {
        return &sdmmc_tf_disk;
    }
    else
    {
        return NULL;
    }
}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

static DSTATUS sdio_disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
    ff_sdmmc_disk *disk = get_sdio_disk(pdrv);

    if (NULL == disk)
    {
        return STA_NOINIT;
    }

	return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

static DSTATUS sdio_disk_initialize(
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
    ff_sdmmc_disk *disk = get_sdio_disk(pdrv);

    if (FALSE == disk->init_ok) /* TODO: why will fatfs try to init disk twice */
    {
        if (SDMMC_OK != sdmmc_host_init(&(disk->sd), &(disk->sd.config))) /* init sdio ctrl, start send init cmd */
        {
            FF_ERROR("sdmmc host init failed");
            return RES_ERROR;
        }

        disk->sector_cnt = disk->sd.card.csd.capacity;
        disk->sector_sz = disk->sd.card.csd.sector_size;
        disk->init_ok = TRUE;
        printf("drv-%d init ok, disk capacity %.0fMB, sector size %d\r\n", 
                pdrv,
                ((double)disk->sector_cnt * (double)disk->sector_sz) / SZ_1M , 
                disk->sd.card.csd.sector_size);
    }
    
	return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

static DRESULT sdio_disk_read (
                BYTE pdrv,		/* Physical drive nmuber to identify the drive */
                BYTE *buff,		/* Data buffer to store read data */
                DWORD sector,	/* Start sector in LBA */
                UINT count		/* Number of sectors to read */
)
{
    ff_sdmmc_disk *disk = get_sdio_disk(pdrv);
    DRESULT status = RES_PARERR;

    if ((NULL != disk) && (disk->init_ok) && (sector < disk->sector_cnt))
    {
#if defined(CONFIG_USE_BAREMETAL)
        /* read sectors from card */
        if (SDMMC_OK != sdmmc_read_sectors(&(disk->sd.card), buff, sector, count))
        {
            FF_ERROR("read sdmmc sector [%d-%d] failed", sector, sector + count);
            status = RES_ERROR;
        }

#elif defined(CONFIG_USE_FREERTOS)
        /* read sectors from card */
        if (SDMMC_OK != sdmmc_os_read_sectors(&(disk->sd.card), buff, sector, count))
        {
            FF_ERROR("read sdmmc sector [%d-%d] failed", sector, sector + count);
            status = RES_ERROR;
        }

#endif

        status = RES_OK;
    }

    return status;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

static DRESULT sdio_disk_write (
                    BYTE pdrv,			/* Physical drive nmuber to identify the drive */
                    const BYTE *buff,	/* Data to be written */
                    DWORD sector,		/* Start sector in LBA */
                    UINT count			/* Number of sectors to write */
)
{
    ff_sdmmc_disk *disk = get_sdio_disk(pdrv);
    DRESULT status = RES_PARERR;

    if ((NULL != disk) && (disk->init_ok) && (sector < disk->sector_cnt))
    {
#if defined(CONFIG_USE_BAREMETAL)
        /* write sectors from card */
        if (SDMMC_OK != sdmmc_write_sectors(&(disk->sd.card), buff, sector, count))
        {
            FF_ERROR("write sdmmc sector [%d-%d] failed", sector, sector + count);
            status = RES_ERROR;
        }
#elif defined(CONFIG_USE_FREERTOS)
        /* write sectors from card */
        if (SDMMC_OK != sdmmc_os_write_sectors(&(disk->sd.card), buff, sector, count))
        {
            FF_ERROR("write sdmmc sector [%d-%d] failed", sector, sector + count);
            status = RES_ERROR;
        }
#endif

        status = RES_OK;
    }

    return status;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT sdio_disk_ioctl (
            BYTE pdrv,		/* Physical drive nmuber (0..) */
            BYTE cmd,		/* Control code */
            void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
    ff_sdmmc_disk *disk = get_sdio_disk(pdrv);

	res = RES_PARERR;
    if (NULL == disk)
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
            *(DWORD*)buff = 4 * disk->sector_sz; /* This is not flash storage that can be erase by command, return 1 */
            res = RES_OK;
        break;
    }

    return res;
}

static const ff_diskio_driver_t sdio_disk_drv = 
{
    .init = &sdio_disk_initialize,
    .status = &sdio_disk_status,
    .read = &sdio_disk_read,
    .write = &sdio_disk_write,
    .ioctl = &sdio_disk_ioctl
};

#ifdef CONFIG_FATFS_SDMMC_FSDIO_TF

void ff_diskio_register_fsdio_tf(BYTE pdrv)
{
    ff_sdmmc_disk *disk = &sdio_tf_disk; 
    sdmmc_host_instance_t *sd = &disk->sd;
    sdmmc_host_config_t *sd_config = &(sd->config);

    memset(sd, 0, sizeof(*sd));

    disk->id = FSDIO1_ID;
    disk->pdrv = pdrv; /* assign volume for ram disk */
    disk->init_ok = FALSE;

    sd_config->slot = FSDIO1_ID;
    sd_config->type = SDMMC_HOST_TYPE_FSDIO;
    sd_config->flags = SDMMC_HOST_WORK_MODE_DMA | SDMMC_HOST_WORK_MODE_IRQ | SDMMC_HOST_REMOVABLE_CARD;
    ff_diskio_register(pdrv, &sdio_disk_drv);

    FF_INFO("Create tf disk as driver-%d", disk->pdrv);
}

#endif

#ifdef CONFIG_FATFS_SDMMC_FSDIO_EMMC

void ff_diskio_register_fsdio_emmc(BYTE pdrv)
{
    ff_sdmmc_disk *disk = &sdio_emmc_disk; 
    sdmmc_host_instance_t *emmc = &disk->sd;
    sdmmc_host_config_t *emmc_config = &(emmc->config);

    memset(emmc, 0, sizeof(*emmc));

    disk->id = FSDIO0_ID;
    disk->pdrv = pdrv; /* assign volume for ram disk */
    disk->init_ok = FALSE;

    emmc_config->slot = FSDIO0_ID;
    emmc_config->type = SDMMC_HOST_TYPE_FSDIO;
    emmc_config->flags = SDMMC_HOST_WORK_MODE_DMA | SDMMC_HOST_WORK_MODE_IRQ;
    ff_diskio_register(pdrv, &sdio_disk_drv);

    FF_INFO("Create emmc disk as driver-%d", disk->pdrv);
}

#endif

#ifdef CONFIG_FATFS_SDMMC_FSDMMC_TF

void ff_diskio_register_fsdmmc_tf(BYTE pdrv)
{
    ff_sdmmc_disk *disk = &sdmmc_tf_disk; 
    sdmmc_host_instance_t *tf = &disk->sd;
    sdmmc_host_config_t *tf_config = &(tf->config);

    memset(tf, 0, sizeof(*tf));

    disk->id = FSDMMC0_ID;
    disk->pdrv = pdrv; /* assign volume for ram disk */
    disk->init_ok = FALSE;

    tf_config->slot = FSDMMC0_ID;
    tf_config->type = SDMMC_HOST_TYPE_FSDMMC;
    tf_config->flags = SDMMC_HOST_WORK_MODE_DMA | SDMMC_HOST_WORK_MODE_IRQ | SDMMC_HOST_REMOVABLE_CARD;
    ff_diskio_register(pdrv, &sdio_disk_drv);

    FF_INFO("Create tf card as driver-%d", disk->pdrv);    
}

#endif