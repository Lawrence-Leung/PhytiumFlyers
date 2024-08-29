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
 * FilePath: ff_bench.c
 * Date: 2022-07-21 13:21:43
 * LastEditTime: 2022-07-21 13:21:44
 * Description:  This files is for fatfs bench implmentation
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/12/3   init commit
 */

#include <string.h>
#include "fdebug.h"
#include "fassert.h"
#include "fkernel.h"
#include "ferror_code.h"
#include "ff_utils.h"
#include "diskio.h"

#define FF_DEBUG_TAG "FATFS"
#define FF_ERROR(format, ...)   FT_DEBUG_PRINT_E(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_INFO(format, ...)    FT_DEBUG_PRINT_I(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_WARN(format, ...)    FT_DEBUG_PRINT_W(FF_DEBUG_TAG, format, ##__VA_ARGS__)

void ff_systimer_start(void);
QWORD ff_systimer_get_tick(void);
void ff_systimer_stop(void);
void ff_systimer_tick_to_time(QWORD ticks, DWORD *sec, DWORD *msec);

static BYTE get_physical_drive(const TCHAR *mount_point)
{
    BYTE pdrv = 0;

    if (!strcmp(mount_point, FF_RAM_DISK_MOUNT_POINT))
    {
        pdrv = 0;
    }
    else if (!strcmp(mount_point, FF_FSDIO_TF_DISK_MOUNT_POINT))
    {
        pdrv = 1;        
    }
    else if (!strcmp(mount_point, FF_FSDIO_EMMC_DISK_MOUNT_POINT))
    {
        pdrv = 2;           
    }
    else if (!strcmp(mount_point, FF_FSDMMC_TF_DISK_MOUNT_POINT))
    {
        pdrv = 3;        
    }
    else if (!strcmp(mount_point, FF_USB_DISK_MOUNT_POINT))
    {
        pdrv = 4;         
    }
    else if (!strcmp(mount_point, FF_SATA_DISK_MOUNT_POINT))
    {
        pdrv = 5;         
    }
    else if (!strcmp(mount_point, FF_SATA_PCIE_DISK_MOUNT_POINT))
    {
        pdrv = 6;         
    }
    else
    {
        FASSERT_MSG(0, "physical driver not found !!!")
    }

    return pdrv;    
}

/*----------------------------------------------------------------------/
/ Low level disk I/O module function checker                            /
/-----------------------------------------------------------------------/
/ WARNING: The data on the target drive will be lost!
*/

static DWORD pn (       /* Pseudo random number generator */
    DWORD pns   /* 0:Initialize, !0:Read */
)
{
    static DWORD lfsr;
    UINT n;


    if (pns) {
        lfsr = pns;
        for (n = 0; n < 32; n++) pn(0);
    }
    if (lfsr & 1) {
        lfsr >>= 1;
        lfsr ^= 0x80200003;
    } else {
        lfsr >>= 1;
    }
    return lfsr;
}

static int test_diskio (
            BYTE pdrv,      /* Physical drive number to be checked (all data on the drive will be lost) */
            UINT ncyc,      /* Number of test cycles */
            DWORD* buff,    /* Pointer to the working buffer */
            UINT sz_buff    /* Size of the working buffer in unit of byte */
)
{
    UINT n, cc, ns;
    DWORD sz_drv, lba, lba2, sz_eblk, pns = 1;
    WORD sz_sect;
    BYTE *pbuff = (BYTE*)buff;
    DSTATUS ds;
    DRESULT dr;


    printf("test_diskio(%u, %u, 0x%08X, 0x%08X)\n", pdrv, ncyc, (uintptr)buff, sz_buff);

    if (sz_buff < FF_MAX_SS + 8) {
        FF_ERROR("Insufficient work area to run the program.");
        return 1;
    }

    for (cc = 1; cc <= ncyc; cc++) {
        printf("**** Test cycle %u of %u start ****\n", cc, ncyc);

        printf(" disk_initalize(%u)", pdrv);
        ds = disk_initialize(pdrv);
        if (ds & STA_NOINIT) {
            FF_ERROR(" - failed.");
            return 2;
        } else {
            printf(" - ok.\n");
        }

        printf("**** Get drive size ****\n");
        printf(" disk_ioctl(%u, GET_SECTOR_COUNT, 0x%08X)", pdrv, (uintptr)&sz_drv);
        sz_drv = 0;
        dr = disk_ioctl(pdrv, GET_SECTOR_COUNT, &sz_drv);
        if (dr == RES_OK) {
            printf(" - ok.\n");
        } else {
            FF_ERROR(" - failed.");
            return 3;
        }
        if (sz_drv < 128) {
            FF_ERROR("Failed: Insufficient drive size to test.");
            return 4;
        }
        printf(" Number of sectors on the drive %u is %lu.\n", pdrv, sz_drv);

#if FF_MAX_SS != FF_MIN_SS
        printf("**** Get sector size ****\n");
        printf(" disk_ioctl(%u, GET_SECTOR_SIZE, 0x%X)", pdrv, (UINT)&sz_sect);
        sz_sect = 0;
        dr = disk_ioctl(pdrv, GET_SECTOR_SIZE, &sz_sect);
        if (dr == RES_OK) {
            printf(" - ok.\n");
        } else {
            FF_ERROR(" - failed.");
            return 5;
        }
        printf(" Size of sector is %u bytes.\n", sz_sect);
#else
        sz_sect = FF_MAX_SS;
#endif

        printf("**** Get block size ****\n");
        printf(" disk_ioctl(%u, GET_BLOCK_SIZE, 0x%X)", pdrv, (uintptr)&sz_eblk);
        sz_eblk = 0;
        dr = disk_ioctl(pdrv, GET_BLOCK_SIZE, &sz_eblk);
        if (dr == RES_OK) {
            printf(" - ok.\n");
        } else {
            FF_ERROR(" - failed.");
        }
        if (dr == RES_OK || sz_eblk >= 2) {
            printf(" Size of the erase block is %lu sectors.\n", sz_eblk);
        } else {
            FF_ERROR(" Size of the erase block is unknown.");
        }

        /* Single sector write test */
        printf("**** Single sector write test ****\n");
        lba = 0;
        for (n = 0, pn(pns); n < sz_sect; n++) pbuff[n] = (BYTE)pn(0);
        printf(" disk_write(%u, 0x%X, %lu, 1)", pdrv, (uintptr)pbuff, lba);
        dr = disk_write(pdrv, pbuff, lba, 1);
        if (dr == RES_OK) {
            printf(" - ok.\n");
        } else {
            FF_ERROR(" - failed.");
            return 6;
        }
        printf(" disk_ioctl(%u, CTRL_SYNC, NULL)", pdrv);
        dr = disk_ioctl(pdrv, CTRL_SYNC, 0);
        if (dr == RES_OK) {
            printf(" - ok.\n");
        } else {
            FF_ERROR(" - failed.");
            return 7;
        }
        memset(pbuff, 0, sz_sect);
        printf(" disk_read(%u, 0x%X, %lu, 1)", pdrv, (uintptr)pbuff, lba);
        dr = disk_read(pdrv, pbuff, lba, 1);
        if (dr == RES_OK) {
            printf(" - ok.\n");
        } else {
            FF_ERROR(" - failed.");
            return 8;
        }
        for (n = 0, pn(pns); n < sz_sect && pbuff[n] == (BYTE)pn(0); n++) ;
        if (n == sz_sect) {
            printf(" Read data matched.\n");
        } else {
            FF_ERROR(" Read data differs from the data written.");
            return 10;
        }
        pns++;

        printf("**** Multiple sector write test ****\n");
        lba = 10; ns = sz_buff / sz_sect;
        if (ns > 4) ns = 3;
        if (ns > 1) {
            for (n = 0, pn(pns); n < (UINT)(sz_sect * ns); n++) pbuff[n] = (BYTE)pn(0);
            printf(" disk_write(%u, 0x%X, %lu, %u)", pdrv, (uintptr)pbuff, lba, ns);
            dr = disk_write(pdrv, pbuff, lba, ns);
            if (dr == RES_OK) {
                printf(" - ok.\n");
            } else {
                FF_ERROR(" - failed.");
                return 11;
            }
            printf(" disk_ioctl(%u, CTRL_SYNC, NULL)", pdrv);
            dr = disk_ioctl(pdrv, CTRL_SYNC, 0);
            if (dr == RES_OK) {
                printf(" - ok.\n");
            } else {
                FF_ERROR(" - failed.");
                return 12;
            }
            memset(pbuff, 0, sz_sect * ns);
            printf(" disk_read(%u, 0x%X, %lu, %u)", pdrv, (uintptr)pbuff, lba, ns);
            dr = disk_read(pdrv, pbuff, lba, ns);
            if (dr == RES_OK) {
                printf(" - ok.\n");
            } else {
                FF_ERROR(" - failed.");
                return 13;
            }

            for (n = 0, pn(pns); n < (UINT)(sz_sect * ns) && pbuff[n] == (BYTE)pn(0); n++) ;
            if (n == (UINT)(sz_sect * ns)) {
                printf(" Read data matched.\n");
            } else {
                FF_ERROR(" Read data differs since %d from the data written.", n);
                return 14;
            }
        } else {
            printf(" Test skipped.\n");
        }
        pns++;

        printf("**** Single sector write test (unaligned buffer address) ****\n");
        if (pdrv == 5||pdrv == 6) 
        {
            FF_ERROR("sata do not suopport unaligned address access,the test will stop.");
            return 0;
        }
        lba = 5;
        for (n = 0, pn(pns); n < sz_sect; n++) pbuff[n+3] = (BYTE)pn(0);
        printf(" disk_write(%u, 0x%X, %lu, 1)", pdrv, (uintptr)(pbuff+3), lba);
        dr = disk_write(pdrv, pbuff+3, lba, 1);
        if (dr == RES_OK) {
            printf(" - ok.\n");
        } else {
            FF_ERROR(" - failed.");
            return 15;
        }
        printf(" disk_ioctl(%u, CTRL_SYNC, NULL)", pdrv);
        dr = disk_ioctl(pdrv, CTRL_SYNC, 0);
        if (dr == RES_OK) {
            printf(" - ok.\n");
        } else {
            FF_ERROR(" - failed.");
            return 16;
        }
        memset(pbuff+5, 0, sz_sect);
        printf(" disk_read(%u, 0x%X, %lu, 1)", pdrv, (uintptr)(pbuff+5), lba);
        dr = disk_read(pdrv, pbuff+5, lba, 1);
        if (dr == RES_OK) {
            printf(" - ok.\n");
        } else {
            FF_ERROR(" - failed.");
            return 17;
        }
        for (n = 0, pn(pns); n < sz_sect && pbuff[n+5] == (BYTE)pn(0); n++) ;
        if (n == sz_sect) {
            printf(" Read data matched.\n");
        } else {
            FF_ERROR(" Read data differs from the data written.");
            return 18;
        }
        pns++;

        printf("**** 4GB barrier test ****\n");
        if (sz_drv >= 128 + 0x80000000 / (sz_sect / 2)) {
            lba = 6; lba2 = lba + 0x80000000 / (sz_sect / 2);
            for (n = 0, pn(pns); n < (UINT)(sz_sect * 2); n++) pbuff[n] = (BYTE)pn(0);
            printf(" disk_write(%u, 0x%X, %lu, 1)", pdrv, (uintptr)pbuff, lba);
            dr = disk_write(pdrv, pbuff, lba, 1);
            if (dr == RES_OK) {
                printf(" - ok.\n");
            } else {
                FF_ERROR(" - failed.");
                return 19;
            }
            printf(" disk_write(%u, 0x%X, %lu, 1)", pdrv, (uintptr)(pbuff+sz_sect), lba2);
            dr = disk_write(pdrv, pbuff+sz_sect, lba2, 1);
            if (dr == RES_OK) {
                printf(" - ok.\n");
            } else {
                FF_ERROR(" - failed.");
                return 20;
            }
            printf(" disk_ioctl(%u, CTRL_SYNC, NULL)", pdrv);
            dr = disk_ioctl(pdrv, CTRL_SYNC, 0);
            if (dr == RES_OK) {
            printf(" - ok.\n");
            } else {
                FF_ERROR(" - failed.");
                return 21;
            }
            memset(pbuff, 0, sz_sect * 2);
            printf(" disk_read(%u, 0x%X, %lu, 1)", pdrv, (uintptr)pbuff, lba);
            dr = disk_read(pdrv, pbuff, lba, 1);
            if (dr == RES_OK) {
                printf(" - ok.\n");
            } else {
                FF_ERROR(" - failed.");
                return 22;
            }
            printf(" disk_read(%u, 0x%X, %lu, 1)", pdrv, (uintptr)(pbuff+sz_sect), lba2);
            dr = disk_read(pdrv, pbuff+sz_sect, lba2, 1);
            if (dr == RES_OK) {
                printf(" - ok.\n");
            } else {
                FF_ERROR(" - failed.");
                return 23;
            }
            for (n = 0, pn(pns); pbuff[n] == (BYTE)pn(0) && n < (UINT)(sz_sect * 2); n++) ;
            if (n == (UINT)(sz_sect * 2)) {
                printf(" Read data matched.\n");
            } else {
                FF_ERROR(" Read data differs from the data written.");
                return 24;
            }
        } else {
            printf(" Test skipped.\n");
        }
        pns++;

        printf("**** Test cycle %u of %u completed ****\n\n", cc, ncyc);
    }

    return 0;
}

int ff_cycle_test (const TCHAR *mount_point, UINT ncyc)
{
    int rc;
    DWORD buff[FF_MAX_SS];  /* Working buffer (4 sector in size) */
    BYTE pdrv = 0;  /* Physical drive number */

    pdrv = get_physical_drive(mount_point);

    /* Check function/compatibility of the physical drive #0 */
    rc = test_diskio(pdrv, ncyc, buff, sizeof buff);

    if (rc) {
        FF_ERROR("Sorry the function/compatibility test failed. (rc=%d)\nFatFs will not work with this disk driver.", rc);
    } else {
        printf("Congratulations! The disk driver works well.\n");
    }

    return rc;
}

static int ff_raw_speed_test (
            BYTE pdrv,      /* Physical drive number */
            DWORD lba,      /* Start LBA for read/write test */
            DWORD len,      /* Number of bytes to read/write (must be multiple of sz_buff) */
            void* buff,     /* Read/write buffer */
            UINT sz_buff    /* Size of read/write buffer (must be multiple of FF_MAX_SS) */
)
{
    WORD ss;
    DWORD ofs;
    QWORD tmr;
    DWORD sec = 0U;
    DWORD msec = 0U;

#if FF_MIN_SS != FF_MAX_SS
    if (disk_ioctl(pdrv, GET_SECTOR_SIZE, &ss) != RES_OK) {
        printf("\ndisk_ioctl() failed.\n");
        return -1;
    }
#else
    ss = FF_MAX_SS;
#endif

    ff_systimer_start();

    printf("Starting raw write test at sector %lu in %u bytes of data chunks...", lba, sz_buff);
    tmr = ff_systimer_get_tick();
    for (ofs = 0; ofs < len / ss; ofs += sz_buff / ss) {
        if (disk_write(pdrv, buff, lba + ofs, sz_buff / ss) != RES_OK) {
            FF_ERROR("\ndisk_write() failed.");
            return -1;
        }
    }
    if (disk_ioctl(pdrv, CTRL_SYNC, 0) != RES_OK) {
        FF_ERROR("\ndisk_ioctl() failed.");
        return -2;
    }

    tmr = ff_systimer_get_tick() - tmr;
    ff_systimer_tick_to_time(tmr, &sec, &msec);
    printf("\n%lu bytes written and it took %lu timer ticks, total time: %d.%03dS.\n", 
            len, tmr, sec, msec);

    printf("Starting raw read test at sector %lu in %u bytes of data chunks...", lba, sz_buff);
    tmr = ff_systimer_get_tick();
    for (ofs = 0; ofs < len / ss; ofs += sz_buff / ss) {
        if (disk_read(pdrv, buff, lba + ofs, sz_buff / ss) != RES_OK) {
            FF_ERROR("\ndisk_read() failed.");
            return -3;
        }
    }
    tmr = ff_systimer_get_tick() - tmr;

    ff_systimer_tick_to_time(tmr, &sec, &msec);
    printf("\n%lu bytes read and it took %lu timer ticks, total time: %d.%03dS.\n", 
            len, tmr, sec, msec);

    ff_systimer_stop();

    printf("Test completed.\n");
    return 0;
}

int ff_speed_bench(const TCHAR *mount_point, DWORD sectors)
{
    BYTE pdrv = 0;  /* Physical drive number */
    DWORD lba = 0;  /* Start LBA for read/write test */
    DWORD len = sectors * FF_MAX_SS;       /* Number of bytes to read/write (must be multiple of sz_buff) */
    static BYTE buff[FF_MAX_SS] = {0};     /* Read/write buffer */
    UINT sz_buff = FF_MAX_SS;              /* Size of read/write buffer (must be multiple of FF_MAX_SS) */

    pdrv = get_physical_drive(mount_point);

    return ff_raw_speed_test(pdrv, lba, len, buff, sz_buff);
}