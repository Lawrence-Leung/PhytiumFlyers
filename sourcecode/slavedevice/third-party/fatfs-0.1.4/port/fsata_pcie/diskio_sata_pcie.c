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
#include "fpcie.h"
#include "fpcie_common.h"
#include "fsata.h"
#include "fsata_hw.h"

#define FSATA_DEBUG_TAG "FSATA-PCIE-DISKIO"
#define FSATA_ERROR(format, ...)   FT_DEBUG_PRINT_E(FSATA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSATA_WARN(format, ...)    FT_DEBUG_PRINT_W(FSATA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSATA_INFO(format, ...)    FT_DEBUG_PRINT_I(FSATA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSATA_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FSATA_DEBUG_TAG, format, ##__VA_ARGS__)

/* 64位需要预留给内存池更大的空间 */
static u8 mem[50000] __attribute__((aligned(1024))) = {0};

#define PCI_CLASS_STORAGE_SATA_AHCI 0x010601

#define SATA_HOST_MAX_NUM   PLAT_AHCI_HOST_MAX_COUNT

#define ADDR_ALIGNMENT 1024

static FSataCtrl sata_device[SATA_HOST_MAX_NUM];//最多支持16个ahci控制器，可以自行定义个数
static s32 sata_host_count = 0;

static boolean sata_ok = FALSE;

static s32 host_num = 0;    /* sata host */
static u32 port_num = 0;   /* sata link port */

static FPcie pcie_obj = {0};

typedef struct
{
    DWORD id;
    DWORD sector_sz;
    DWORD sector_cnt;
    FSataCtrl sata;
    boolean init_ok;
    BYTE pdrv;
} ff_sata_pcie_disk;

static ff_sata_pcie_disk sata_pcie_disk =
{
    .pdrv = FF_DRV_NOT_USED,
    .init_ok = FALSE,
};
/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS sata_pcie_disk_status(
    BYTE pdrv       /* Physical drive nmuber to identify the drive */
)
{
    DSTATUS status = STA_NOINIT;

    if (FT_COMPONENT_IS_READY == sata_device[host_num].is_ready)
    {
        status &= ~STA_NOINIT;    /* 假设Sata处于插入状态 */
    }

    return status;
}

static void FSataPcieIrqHandler(void *param)
{
    FSataIrqHandler(0, param);
}

static void PCieIntxInit(FPcie *instance_p)
{
    InterruptSetPriority(FPCI_INTA_IRQ_NUM, 0);

    InterruptInstall(FPCI_INTA_IRQ_NUM, (IrqHandler)FPcieIntxIrqHandler, instance_p, "pcieInta");
    InterruptUmask(FPCI_INTA_IRQ_NUM);
    InterruptSetPriority(FPCI_INTB_IRQ_NUM, 0);
    InterruptInstall(FPCI_INTB_IRQ_NUM, (IrqHandler)FPcieIntxIrqHandler, instance_p, "pcieIntB");
    InterruptUmask(FPCI_INTB_IRQ_NUM);
    InterruptSetPriority(FPCI_INTC_IRQ_NUM, 0);
    InterruptInstall(FPCI_INTC_IRQ_NUM, (IrqHandler)FPcieIntxIrqHandler, instance_p, "pcieIntC");
    InterruptUmask(FPCI_INTC_IRQ_NUM);
    InterruptSetPriority(FPCI_INTD_IRQ_NUM, 0);
    InterruptInstall(FPCI_INTD_IRQ_NUM, (IrqHandler)FPcieIntxIrqHandler, instance_p, "pcieIntD");
    InterruptUmask(FPCI_INTD_IRQ_NUM);
}

static void FPcieInit()
{
    /* 第一步初始化pcie_obj这个实例，初始化mem，io资源成员 */
    FPcieCfgInitialize(&pcie_obj, FPcieLookupConfig(FPCIE0_ID));
    FSATA_DEBUG("\n");
    FSATA_DEBUG("	PCI:\n");
    FSATA_DEBUG("	B:D:F			VID:PID			parent_BDF			class_code\n");
    FPcieScanBus(&pcie_obj, 0, 0xffffffff);
    PCieIntxInit(&pcie_obj);
}

static uintptr_t SataPcieIrqInstall(FSataCtrl *ahci_ctl, u32 bdf)
{
    int ret = FT_SUCCESS;

    FPcieIntxFun intx_fun;
    intx_fun.IntxCallBack = FSataPcieIrqHandler;
    intx_fun.args = ahci_ctl;
    intx_fun.bdf = bdf;

    ret = FPcieIntxRegiterIrqHandler(&pcie_obj, bdf, &intx_fun);
    if (ret != FT_SUCCESS)
    {
        return ret;
    }

    return 0;
}

static int FSataInit(void)
{
    int ret;
    s32 host = 0;
    u32 port = 0;
    u32 bdf;
    u32 class;
    u16 pci_command;
    const u32 class_code = PCI_CLASS_STORAGE_SATA_AHCI;
    uintptr bar_addr = 0;
    u16 vid, did;
    u32 port_mem_count = 0;
    u8 link_success = 0;

    const FSataConfig *config_p = NULL;
    FSataCtrl *instance_p;
    FError status = FSATA_SUCCESS;

    FPcie *pcie = &pcie_obj;

    if (sata_ok == TRUE)
    {
        FSATA_WARN("Sata already init.\r\n");
        return 0;
    }

    for (host = 0; host < SATA_HOST_MAX_NUM; host++)
    {
        instance_p = &sata_device[host];
        memset(instance_p, 0, sizeof(*instance_p));
        config_p = FSataLookupConfig(host, FSATA_TYPE_PCIE);
        status = FSataCfgInitialize(&sata_device[host], config_p);
        if (FSATA_SUCCESS != status)
        {
            FSATA_ERROR("Init sata failed, status: 0x%x.", status);
            continue;
        }
    }

    /* find host from pcie instance */
    for (host = 0; host < pcie->scaned_bdf_count; host++)
    {
        bdf = pcie->scaned_bdf_array[host];
        FPcieEcamReadConfig32bit(pcie->config.ecam, bdf, FPCI_CLASS_REVISION, &class) ;
        class = (class) >> 8 ;

        if (class == class_code)
        {
            FPcieEcamReadConfig16bit(pcie->config.ecam, bdf, FPCIE_VENDOR_REG, &vid) ;
            FPcieEcamReadConfig16bit(pcie->config.ecam, bdf, FPCIE_DEVICE_ID_REG, &did);

            FSATA_DEBUG("AHCI-PCI HOST found !!!, b.d.f = %x.%x.%x\n", FPCIE_BUS(bdf), FPCIE_DEV(bdf), FPCIE_FUNC(bdf));

            FPcieEcamReadConfig32bit(pcie->config.ecam, bdf, FPCIE_BASE_ADDRESS_5, (u32 *)&bar_addr);
            FSATA_DEBUG("FSataPcieIntrInstall BarAddress %p.", bar_addr);

            if (0x0 == bar_addr)
            {
                FSATA_ERROR("Bar address: 0x%lx.", bar_addr);
                return -1;
            }
            FPcieEcamReadConfig16bit(pcie->config.ecam, bdf, FPCIE_COMMAND_REG, &pci_command);
            pci_command |= FPCIE_COMMAND_MASTER;
            FPcieEcamWriteConfig16bit(pcie->config.ecam, bdf, FPCIE_COMMAND_REG, pci_command);

            SataPcieIrqInstall(&sata_device[sata_host_count], bdf);
            sata_device[sata_host_count].config.base_addr = bar_addr;
            FSATA_DEBUG("sata_device[%d].config.base_addr = 0x%x.\n", sata_host_count, sata_device[sata_host_count].config.base_addr);
            sata_host_count++;
            if (sata_host_count >= SATA_HOST_MAX_NUM)
            {
                break;
            }
        }
    }

    FSATA_DEBUG("Scaned %d ahci host.\n", sata_host_count);

    for (host = 0; host < sata_host_count; host++)
    {
        instance_p = &sata_device[host];

        /* Initialization */
        status = FSataAhciInit(instance_p);
        if (FSATA_SUCCESS != status)
        {
            FSataCfgDeInitialize(instance_p);
            FSATA_ERROR("FSataAhciInit sata failed, ret: 0x%x.", status);
            continue;
        }

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
                FSATA_ERROR("FSataAhciPortStart port %d failed, ret: 0x%x.", port, ret);
                continue;
            }

            status = FSataAhciReadInfo(instance_p, port);
            if (FSATA_SUCCESS != status)
            {
                FSataCfgDeInitialize(instance_p);
                FSATA_ERROR("FSataAhciReadInfo failed, ret: 0x%x.", status);
                continue;
            }

        }
    }

    sata_ok = TRUE;

    /* get host number and port number which link success */
    for (host_num = 0; host_num < sata_host_count; host_num++)
    {
        for (port_num = 0; port_num < sata_device[host_num].n_ports; port_num++)
        {
            if (!(sata_device[host_num].link_port_map & BIT(port_num)))
            {
                FSATA_DEBUG("host_num %d port_num %d is not link.\n", host_num, port_num);
                continue;
            }
            else
            {
                FSATA_DEBUG("host_num %d port_num %d is link.\n", host_num, port_num);
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

DSTATUS sata_pcie_disk_initialize(
    BYTE pdrv               /* Physical drive nmuber to identify the drive */
)
{
    DSTATUS status = STA_NOINIT;
    static u8 flag = 0;
    if (flag == 0)
    {
        FPcieInit();
        flag = 1;
    }

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

DRESULT sata_pcie_disk_read(
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
        FSATA_ERROR("Read pcie sata sector [%d-%d] failed: 0x%x.", sector, sector + count, err);
        status = RES_ERROR;
    }

    return status;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT sata_pcie_disk_write(
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
        FSATA_ERROR("Write pcie sata sector [%d-%d] failed: 0x%x.", sector, sector + count, err);
        status = RES_ERROR;
    }

    return status;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT sata_pcie_disk_ioctl(
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
            res = RES_OK; /* 最多使用1000个sector */
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

static const ff_diskio_driver_t sata_pcie_disk_drv =
{
    .init = &sata_pcie_disk_initialize,
    .status = &sata_pcie_disk_status,
    .read = &sata_pcie_disk_read,
    .write = &sata_pcie_disk_write,
    .ioctl = &sata_pcie_disk_ioctl
};

void ff_diskio_register_sata_pcie(BYTE pdrv)
{
    ff_sata_pcie_disk *disk = &sata_pcie_disk;

    disk->id = FSATA0_ID;
    disk->init_ok = FALSE;
    disk->pdrv = pdrv; /* assign volume for sata pcie disk */
    ff_diskio_register(pdrv, &sata_pcie_disk_drv);

    FSATA_INFO("Create sata pcie disk as driver-%d.", disk->pdrv);
}








