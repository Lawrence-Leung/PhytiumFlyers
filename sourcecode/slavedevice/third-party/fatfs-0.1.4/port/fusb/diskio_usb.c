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
#include "fparameters.h"
#include "fassert.h"
#include "fcache.h"
#include "finterrupt.h"
#include "fcpu_info.h"
#include "diskio.h"
#include "ffconf.h"
#include "ff.h"
#include "fmemory_pool.h"
#include "sdkconfig.h"

#include "usbh_core.h"
#include "usbh_msc.h"

#define FF_DEBUG_TAG "DISKIO-USB"
#define FF_ERROR(format, ...)   FT_DEBUG_PRINT_E(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_INFO(format, ...)    FT_DEBUG_PRINT_I(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_WARN(format, ...)    FT_DEBUG_PRINT_W(FF_DEBUG_TAG, format, ##__VA_ARGS__)

#define FUSB_MEMP_TOTAL_SIZE     SZ_1M
#define FUSB_FATFS_ID            FUSB3_ID_0

static FMemp memp;
static u8 memp_buf[FUSB_MEMP_TOTAL_SIZE];
FASSERT_STATIC(FUSB_FATFS_ID < FUSB3_NUM);

static const u32 usb_irq_num[FUSB3_NUM] =
{
    [FUSB3_ID_0] = FUSB3_0_IRQ_NUM,
    [FUSB3_ID_1] = FUSB3_1_IRQ_NUM
};

static const uintptr xhci_base_addr[FUSB3_NUM] =
{
    [FUSB3_ID_0] = FUSB3_0_BASE_ADDR + FUSB3_XHCI_OFFSET,
    [FUSB3_ID_1] = FUSB3_1_BASE_ADDR + FUSB3_XHCI_OFFSET
};

typedef struct
{
    DWORD id;
    boolean init_ok;
    BYTE pdrv;
    const TCHAR *disk_name;
    struct usbh_bus usb;
} ff_usb_disk;

static ff_usb_disk usb_disk =
{
    .id = FUSB_FATFS_ID,
    .pdrv = FF_DRV_NOT_USED,
    .init_ok = FALSE,
    .disk_name = "/usb0/sda" 
};

/*****************************************************************************/
extern void USBH_IRQHandler(void *);

static void UsbHcInterrruptHandler(s32 vector, void *param)
{
    USBH_IRQHandler(param);
}

static void UsbHcSetupInterrupt(u32 id)
{
    u32 cpu_id;
    u32 irq_num = usb_irq_num[id];
    u32 irq_priority = 13U;

    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(irq_num, cpu_id);

    InterruptSetPriority(irq_num, irq_priority);

    /* register intr callback */
    InterruptInstall(irq_num,
                     UsbHcInterrruptHandler,
                     &(usb_disk.usb),
                     NULL);

    /* enable irq */
    InterruptUmask(irq_num);
}

void UsbHcSetupMemp(void)
{
    if (FT_COMPONENT_IS_READY != memp.is_ready)
    {
        USB_ASSERT(FT_SUCCESS == FMempInit(&memp, &memp_buf[0], &memp_buf[0] + FUSB_MEMP_TOTAL_SIZE));
    }
}

/* implement cherryusb weak functions */
void usb_hc_low_level_init(uint32_t id)
{
    UsbHcSetupMemp();
    UsbHcSetupInterrupt(id);
}

unsigned long usb_hc_get_register_base(uint32_t id)
{
    return xhci_base_addr[id];
}

void *usb_hc_malloc(size_t size)
{
    return usb_hc_malloc_align(sizeof(void *), size);
}

void *usb_hc_malloc_align(size_t align, size_t size)
{
    void *result = FMempMallocAlign(&memp, size, align);

    if (result)
    {
        memset(result, 0U, size);
    }

    return result;
}

void usb_hc_free(void *ptr)
{
    if (NULL != ptr)
    {
        FMempFree(&memp, ptr);
    }
}

void usb_assert(const char *filename, int linenum)
{
    FAssert(filename, linenum, 0xff);
}

void usb_hc_dcache_invalidate(void *addr, unsigned long len)
{
    FCacheDCacheInvalidateRange((uintptr)addr, len);
}
/*****************************************/

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

static DSTATUS usb_disk_status(
    BYTE pdrv       /* Physical drive nmuber to identify the drive */
)
{
    DSTATUS status = STA_NOINIT;
    ff_usb_disk *disk = &usb_disk;

    if (disk->init_ok)
    {
        status &= ~STA_NOINIT;
    }

    return status;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

static DSTATUS usb_disk_initialize(
    BYTE pdrv               /* Physical drive nmuber to identify the drive */
)
{
    DSTATUS status = STA_NOINIT;
    ff_usb_disk *disk = &usb_disk;
    int retries = 10000;

    if (FF_DRV_NOT_USED == disk->pdrv)
    {
        return STA_NOINIT;
    }

    if (FALSE == disk->init_ok)
    {
        memset(&disk->usb, 0, sizeof(disk->usb));
        (void)usbh_initialize(disk->id, &disk->usb); /* start a task to emurate usb hub and attached usb disk */
        while (TRUE)
        {
            if (NULL != usbh_find_class_instance(disk->disk_name))
            {
                break;
            }

            if (retries-- < 0)
            {
                FF_ERROR("Init cherryusb host failed or usb disk device not found !!!");
                return STA_NOINIT;
            }

            vTaskDelay(10); /* may need to wait while for usb disk emuration */
        }

        disk->init_ok = TRUE;
    }

    status &= ~STA_NOINIT;
    return status;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

static DRESULT usb_disk_read(
    BYTE pdrv,      /* Physical drive nmuber to identify the drive */
    BYTE *buff,     /* Data buffer to store read data */
    DWORD sector,   /* Start sector in LBA */
    UINT count      /* Number of sectors to read */
)
{
    DRESULT status = RES_OK;
    ff_usb_disk *disk = &usb_disk;
    struct usbh_msc *msc_class = (struct usbh_msc *)usbh_find_class_instance(disk->disk_name);

    if (msc_class)
    {
        if (0 > usbh_msc_scsi_read10(msc_class, sector, buff, count))
        {
            FF_ERROR("Read usb sector [%d-%d] failed: 0x%x.", sector, sector + count);
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

static DRESULT usb_disk_write(
    BYTE pdrv,          /* Physical drive nmuber to identify the drive */
    const BYTE *buff,   /* Data to be written */
    DWORD sector,       /* Start sector in LBA */
    UINT count          /* Number of sectors to write */
)
{
    DRESULT status = RES_OK;
    ff_usb_disk *disk = &usb_disk;
    struct usbh_msc *msc_class = (struct usbh_msc *)usbh_find_class_instance(disk->disk_name);

    if (msc_class)
    {
        if (0 > usbh_msc_scsi_write10(msc_class, sector, buff, count))
        {
            FF_ERROR("Write usb sector [%d-%d] failed: 0x%x.", sector, sector + count);
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

DRESULT usb_disk_ioctl(
    BYTE pdrv,      /* Physical drive nmuber (0..) */
    BYTE cmd,       /* Control code */
    void *buff      /* Buffer to send/receive control data */
)
{
    DRESULT res;
    ff_usb_disk *disk = &usb_disk;
    struct usbh_msc *msc_class = (struct usbh_msc *)usbh_find_class_instance(disk->disk_name);

    res = RES_PARERR;
    if (NULL == msc_class)
    {
        return res;
    }

    switch (cmd)
    {
        case CTRL_SYNC:         /* Nothing to do */
            res = RES_OK;
            break;

        case GET_SECTOR_COUNT:  /* Get number of sectors on the drive */
            *(DWORD *)buff = msc_class->blocknum;
            res = RES_OK;
            break;

        case GET_SECTOR_SIZE:   /* Get size of sector for generic read/write */
            *(WORD *)buff = msc_class->blocksize;
            res = RES_OK;
            break;

        case GET_BLOCK_SIZE:
            *(DWORD *)buff = 1; /* This is not flash storage that can be erase by command, return 1 */
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

    disk->id = FUSB_FATFS_ID;
    disk->init_ok = FALSE;
    disk->pdrv = pdrv; /* assign volume for usb disk */
    ff_diskio_register(pdrv, &usb_disk_drv);

    FF_INFO("Create usb disk as driver-%d.", disk->pdrv);
}