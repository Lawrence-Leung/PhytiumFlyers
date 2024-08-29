
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
 * FilePath: diskio.h
 * Date: 2022-07-21 13:21:43
 * LastEditTime: 2022-07-21 13:21:44
 * Description:  This files is for fatfs port glue definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/12/3   init commit
 */

/*-----------------------------------------------------------------------/
/  Low level disk interface modlue include file   (C)ChaN, 2019          /
/-----------------------------------------------------------------------*/

#ifndef _DISKIO_DEFINED
#define _DISKIO_DEFINED

#include <assert.h>
#include "sdkconfig.h"
#include "ftypes.h"
#include "ff.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int UINT;
typedef unsigned char BYTE;
typedef uint32_t DWORD;

/* Status of Disk Functions */
typedef BYTE	DSTATUS;

#define FF_DRV_NOT_USED   0xFF
#define FF_VOL_FOUND      0
#define FF_VOL_NOT_FOUND  -1

/* Results of Disk Functions */
typedef enum {
	RES_OK = 0,		/* 0: Successful */
	RES_ERROR,		/* 1: R/W Error */
	RES_WRPRT,		/* 2: Write Protected */
	RES_NOTRDY,		/* 3: Not Ready */
	RES_PARERR		/* 4: Invalid Parameter */
} DRESULT;


/*---------------------------------------*/
/* Prototypes for disk control functions */


DSTATUS disk_initialize (BYTE pdrv);
DSTATUS disk_status (BYTE pdrv);
DRESULT disk_read (BYTE pdrv, BYTE* buff, LBA_t sector, UINT count);
DRESULT disk_write (BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count);
DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void* buff);


/**
 * Structure of pointers to disk IO driver functions.
 *
 * See FatFs documentation for details about these functions
 */
typedef struct {
    DSTATUS (*init) (BYTE pdrv);    /*!< disk initialization function */
    DSTATUS (*status) (BYTE pdrv);  /*!< disk status check function */
    DRESULT (*read) (BYTE pdrv,		/* Physical drive nmuber to identify the drive */
                    BYTE *buff,		/* Data buffer to store read data */
                    LBA_t sector,	/* Start sector in LBA */
                    UINT count		/* Number of sectors to read */);
    DRESULT (*write) (	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
                        const BYTE *buff,	/* Data to be written */
                        LBA_t sector,		/* Start sector in LBA */
                        UINT count			/* Number of sectors to write */);
    DRESULT (*ioctl) (	BYTE pdrv,		/* Physical drive nmuber (0..) */
                        BYTE cmd,		/* Control code */
                        void *buff		/* Buffer to send/receive control data */);
} ff_diskio_driver_t;

/**
 * Register or unregister diskio driver for given drive number.
 *
 * When FATFS library calls one of disk_xxx functions for driver number pdrv,
 * corresponding function in discio_port for given pdrv will be called.
 *
 * @param pdrv          drive number
 * @param discio_impl   pointer to ff_diskio_impl_t structure with diskio functions
 *                      or NULL to unregister and free previously registered drive
 */
void ff_diskio_register(BYTE pdrv, const ff_diskio_driver_t* diskio_drv);

#define ff_diskio_unregister(pdrv_) ff_diskio_register(pdrv_, NULL)

/**
 * Get next available drive number
 *
 * @param   out_pdrv            pointer to the byte to set if successful
 *
 * @return  FF_VOL_FOUND        on success
 *          FF_VOL_NOT_FOUND    if all drivers are attached
 */
int ff_diskio_get_available_volume(const TCHAR *mount_point, BYTE* out_pdrv);

#ifdef CONFIG_FATFS_RAM_DISK

/**
 * Register ram disk
 *
 * @param   BYTE pdrv           drive number
 */
void ff_diskio_register_ram(BYTE pdrv);

#endif

#ifdef CONFIG_FATFS_SDMMC_FSDIO_TF

/**
 * Register tf disk
 *
 * @param   BYTE pdrv           drive number
 */
void ff_diskio_register_fsdio_tf(BYTE pdrv);

#endif

#ifdef CONFIG_FATFS_SDMMC_FSDIO_EMMC

/**
 * Register eMMC card
 *
 * @param   BYTE pdrv           drive number
 */
void ff_diskio_register_fsdio_emmc(BYTE pdrv);

#endif

#ifdef CONFIG_FATFS_SDMMC_FSDMMC_TF

/**
 * Register Tf card
 *
 * @param   BYTE pdrv           drive number
 */
void ff_diskio_register_fsdmmc_tf(BYTE pdrv);
#endif

#ifdef CONFIG_FATFS_USB

/**
 * Register usb disk
 *
 * @param   BYTE pdrv           drive number
 */
void ff_diskio_register_usb(BYTE pdrv);
#endif

#ifdef CONFIG_FATFS_FSATA

/**
 * Register sata disk
 *
 * @param   BYTE pdrv           drive number
 */
void ff_diskio_register_sata(BYTE pdrv);
#endif

#ifdef CONFIG_FATFS_FSATA_PCIE

/**
 * Register sata pcie disk
 *
 * @param   BYTE pdrv           drive number
 */
void ff_diskio_register_sata_pcie(BYTE pdrv);
#endif

/* Disk Status Bits (DSTATUS) */

#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_PROTECT		0x04	/* Write protected */


/* Command code for disk_ioctrl fucntion */

/* Generic command (Used by FatFs) */
#define CTRL_SYNC			0	/* Complete pending write process (needed at FF_FS_READONLY == 0) */
#define GET_SECTOR_COUNT	1	/* Get media size (needed at FF_USE_MKFS == 1) */
#define GET_SECTOR_SIZE		2	/* Get sector size (needed at FF_MAX_SS != FF_MIN_SS) */
#define GET_BLOCK_SIZE		3	/* Get erase block size (needed at FF_USE_MKFS == 1) */
#define CTRL_TRIM			4	/* Inform device that the data on the block of sectors is no longer used (needed at FF_USE_TRIM == 1) */

/* Generic command (Not used by FatFs) */
#define CTRL_POWER			5	/* Get/Set power status */
#define CTRL_LOCK			6	/* Lock/Unlock media removal */
#define CTRL_EJECT			7	/* Eject media */
#define CTRL_FORMAT			8	/* Create physical format on the media */

/* MMC/SDC specific ioctl command */
#define MMC_GET_TYPE		10	/* Get card type */
#define MMC_GET_CSD			11	/* Get CSD */
#define MMC_GET_CID			12	/* Get CID */
#define MMC_GET_OCR			13	/* Get OCR */
#define MMC_GET_SDSTAT		14	/* Get SD status */
#define ISDIO_READ			55	/* Read data form SD iSDIO register */
#define ISDIO_WRITE			56	/* Write data to SD iSDIO register */
#define ISDIO_MRITE			57	/* Masked write data to SD iSDIO register */

/* ATA/CF specific ioctl command */
#define ATA_GET_REV			20	/* Get F/W revision */
#define ATA_GET_MODEL		21	/* Get model name */
#define ATA_GET_SN			22	/* Get serial number */

#ifdef __cplusplus
}
#endif

#endif
