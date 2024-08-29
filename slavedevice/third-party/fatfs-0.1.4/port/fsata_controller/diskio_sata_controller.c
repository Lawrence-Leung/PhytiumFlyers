/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include <string.h>
#include "fparameters.h"
#include "fdebug.h"
#include "finterrupt.h"
#include "ff.h"
#include "diskio.h"     /* FatFs lower layer API */
#include "fsata.h"
#include "fsata_hw.h"

#define FSATA_DEBUG_TAG "FSATA-CONTROLLER-DISKIO"
#define FSATA_ERROR(format, ...)   FT_DEBUG_PRINT_E(FSATA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSATA_WARN(format, ...)    FT_DEBUG_PRINT_W(FSATA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSATA_INFO(format, ...)    FT_DEBUG_PRINT_I(FSATA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSATA_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FSATA_DEBUG_TAG, format, ##__VA_ARGS__)

/* 64位需要预留给内存池更大的空间 */
static u8 mem[50000] __attribute__((aligned(1024))) = {0};

#define ADDR_ALIGNMENT 1024

static FSataCtrl sata_device[FSATA_NUM];//最多支持16个ahci控制器，可以自行定义个数

static boolean sata_ok = FALSE;

static u32 host_num = 0;    /* sata host */
static u32 port_num = 0;   /* sata link port */

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/
typedef struct
{
    DWORD id;
    DWORD sector_sz;
    DWORD sector_cnt;
    FSataCtrl sata;
    boolean init_ok;
    BYTE pdrv;
} ff_sata_disk;

static ff_sata_disk sata_disk =
{
    .pdrv = FF_DRV_NOT_USED,
    .init_ok = FALSE,
};

DSTATUS sata_disk_status(
    BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
    DSTATUS status = STA_NOINIT;

    if (FT_COMPONENT_IS_READY == sata_device[host_num].is_ready)
    {
        status &= ~STA_NOINIT;    /* 假设Sata处于插入状态 */
    }

    return status;
}

static int FSataInit(void)
{
    u32 instance_id = 0;
    u32 port = 0;
    u32 port_mem_count = 0;
    u8 link_success = 0;

    const FSataConfig *config_p = NULL;
    FSataCtrl *instance_p;
    FError ret = FSATA_SUCCESS;
    boolean host_valid = FALSE;

    if (sata_ok == TRUE)
    {
        FSATA_WARN("Sata already init.\r\n");
        return 0;
    }

    for (instance_id = 0; instance_id < FSATA_NUM; instance_id++)
    {
        host_valid = FALSE;
        config_p = FSataLookupConfig(instance_id, FSATA_TYPE_CONTROLLER);
        if (config_p != NULL)
        {
            ret = FSataCfgInitialize(&sata_device[instance_id], config_p);
            if (FSATA_SUCCESS != ret)
            {
                FSATA_ERROR("Init sata failed, ret: 0x%x.", ret);
                continue;
            }

            FSATA_DEBUG("Plat ahci host[%d] base_addr = 0x%x.", instance_id, sata_device[instance_id].config.base_addr);
            FSATA_DEBUG("Plat ahci host[%d] irq_num = %d.", instance_id, sata_device[instance_id].config.irq_num);
        }
        else
        {
            continue;
        }

        instance_p = &sata_device[instance_id];

        /* init ahci controller and port */
        ret = FSataAhciInit(instance_p);
        if (FSATA_SUCCESS != ret)
        {
            FSataCfgDeInitialize(instance_p);
            FSATA_ERROR("FSataAhciInit sata failed, ret: 0x%x.", ret);
            continue;
        }

        FSATA_DEBUG("instance_p->n_ports = %d.\n", instance_p->n_ports);

        for (port = 0; port < instance_p->n_ports; port++)
        {
            u32 port_map = instance_p->port_map;
            if (!(port_map & BIT(port)))
            {
                continue;
            }

            /* command list address must be 1K-byte aligned */
            ret = FSataAhciPortStart(instance_p, port,
                                     (uintptr)mem + PALIGN_UP(FSATA_AHCI_PORT_PRIV_DMA_SZ, ADDR_ALIGNMENT) * port_mem_count);
            port_mem_count++;
            if (FSATA_SUCCESS != ret)
            {
                FSATA_ERROR("FSataAhciPortStart %d-%d failed, ret: 0x%x.", instance_id, port, ret);
                continue;
            }

            ret = FSataAhciReadInfo(instance_p, port);
            if (FSATA_SUCCESS != ret)
            {
                FSataCfgDeInitialize(instance_p);
                FSATA_ERROR("FSataAhciReadInfo %d-%d failed, ret: 0x%x.", instance_id, port, ret);
                continue;
            }
            if (FSATA_SUCCESS == ret)
            {
                host_valid = TRUE;
            }
        }

    }

    sata_ok = TRUE;

    /* get host number and port number which link success */
    for (host_num = 0; host_num < FSATA_NUM; host_num++)
    {
        for (port_num = 0; port_num < sata_device[host_num].n_ports; port_num++)
        {
            if (!(sata_device[host_num].link_port_map & BIT(port_num)))
            {
                FSATA_ERROR("host_num %d port_num %d is not link.\n", host_num, port_num);
                continue;
            }
            else
            {
                FSATA_INFO("host_num %d port_num %d is link.\n", host_num, port_num);
                link_success = 1;
                break;
            }
        }
        if (link_success == 1)
        {
            break;
        }
    }
    if (link_success != 1)
    {
        FSATA_ERROR("Sata host port link failed.\n");
        return -1;
    }

    return 0;
}


/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS sata_disk_initialize(
    BYTE pdrv               /* Physical drive nmuber to identify the drive */
)
{
    DSTATUS status = STA_NOINIT;
    if (FSATA_SUCCESS == FSataInit())
    {
        status &= ~STA_NOINIT;
        FSATA_INFO("Init sata driver successfully.");
    }
    else
    {
        FSATA_ERROR("Init sata driver failed.");
    }

    return status;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT sata_disk_read(
    BYTE pdrv,      /* Physical drive nmuber to identify the drive */
    BYTE *buff,     /* Data buffer to store read data */
    DWORD sector,   /* Start sector in LBA */
    UINT count      /* Number of sectors to read */
)
{
    DRESULT status = RES_OK;
    BYTE *io_buf = buff;
    UINT err = FSATA_SUCCESS;

    err = FSataReadWrite(&sata_device[host_num], port_num, sector, count, io_buf, FALSE, FALSE);

    if (FSATA_SUCCESS != err)
    {
        FSATA_ERROR("Read sata controller sector [%d-%d] failed: 0x%x.", sector, sector + count, err);
        status = RES_ERROR;
    }

    return status;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT sata_disk_write(
    BYTE pdrv,          /* Physical drive nmuber to identify the drive */
    const BYTE *buff,   /* Data to be written */
    DWORD sector,       /* Start sector in LBA */
    UINT count          /* Number of sectors to write */
)
{
    DRESULT status = RES_OK;
    const BYTE *io_buf = buff;
    UINT err = FSATA_SUCCESS;

    err = FSataReadWrite(&sata_device[host_num], port_num, sector, count, (u8 *)io_buf, FALSE, TRUE);

    if (FSATA_SUCCESS != err)
    {
        FSATA_ERROR("Write sata controller sector [%d-%d] failed: 0x%x.", sector, sector + count, err);
        status = RES_ERROR;
    }

    return status;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT sata_disk_ioctl(
    BYTE pdrv,      /* Physical drive nmuber (0..) */
    BYTE cmd,       /* Control code */
    void *buff      /* Buffer to send/receive control data */
)
{
    DRESULT res = RES_ERROR;

    switch (cmd)
    {
        /* 确保磁盘驱动器已经完成了写处理，当磁盘I/O有一个写回缓存，
           立即刷新原扇区，只读配置下不适用此命令 */
        case CTRL_SYNC:
            res = RES_OK;
            break;
        /* 所有可用的扇区数目（逻辑寻址即LBA寻址方式） */
        case GET_SECTOR_COUNT:
            *((DWORD *)buff) = sata_device[host_num].port[port_num].dev_info.lba512;
            res = RES_OK;
            break;
        /* 返回磁盘扇区大小, 只用于f_mkfs() */
        case GET_SECTOR_SIZE:
            res = RES_PARERR;
            break;
        /* 每个扇区有多少个字节 */
        case GET_BLOCK_SIZE:
            *((DWORD *)buff) = sata_device[host_num].port[port_num].dev_info.blksz;
            res = RES_OK;
            break;
        case CTRL_TRIM:
            res = RES_PARERR;
            break;
    }

    FSATA_INFO("cmd %d, buff: %p", cmd, *((DWORD *) buff));
    return res;
}

static const ff_diskio_driver_t sata_disk_drv =
{
    .init = &sata_disk_initialize,
    .status = &sata_disk_status,
    .read = &sata_disk_read,
    .write = &sata_disk_write,
    .ioctl = &sata_disk_ioctl
};

void ff_diskio_register_sata(BYTE pdrv)
{
    ff_sata_disk *disk = &sata_disk;

    disk->id = FSATA0_ID;
    disk->init_ok = FALSE;
    disk->pdrv = pdrv; /* assign volume for sata disk */
    ff_diskio_register(pdrv, &sata_disk_drv);

    FSATA_INFO("Create sata disk as driver-%d.", disk->pdrv);
}










