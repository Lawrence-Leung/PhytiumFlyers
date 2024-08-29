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
 * FilePath: platform_info.c
 * Date: 2022-02-23 11:24:12
 * LastEditTime: 2022-02-23 11:43:44
 * Description:  This file define platform specific data and implements APIs to set
 *       platform specific information for OpenAMP.
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0	 huanghe	2022/03/06   first release
 */

/***************************** Include Files *********************************/

#include <metal/atomic.h>
#include <metal/assert.h>
#include <metal/device.h>
#include <metal/irq.h>
#include <metal/utilities.h>
#include <openamp/rpmsg_virtio.h>
#include <errno.h>
#include "platform_info.h"
#include "rsc_table.h"
#include "sdkconfig.h"
#include "fmmu.h"
#include "fdebug.h"
#include "stdio.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define     FT_PLAT_INFO_MAIN_DEBUG_TAG "    FT_PLAT_INFO_MAIN"
#define     FT_PLAT_INFO_MAIN_DEBUG_I(format, ...) FT_DEBUG_PRINT_I( FT_PLAT_INFO_MAIN_DEBUG_TAG, format, ##__VA_ARGS__)
#define     FT_PLAT_INFO_MAIN_DEBUG_W(format, ...) FT_DEBUG_PRINT_W( FT_PLAT_INFO_MAIN_DEBUG_TAG, format, ##__VA_ARGS__)
#define     FT_PLAT_INFO_MAIN_DEBUG_E(format, ...) FT_DEBUG_PRINT_E( FT_PLAT_INFO_MAIN_DEBUG_TAG, format, ##__VA_ARGS__)


#define KICK_DEV_NAME         "poll_dev"
#define KICK_BUS_NAME         "generic"


#ifdef CONFIG_USE_OPENAMP_IPI
#define _rproc_wait() asm volatile("wfi")
#endif /* !RPMSG_NO_IPI */

#ifdef CONFIG_MEM_NORMAL
#define MEM_ATTR_ACESS (MT_NORMAL|MT_P_RW_U_RW)
#endif

#ifdef CONFIG_MEM_WRITE_THROUGH
#define MEM_ATTR_ACESS (MT_NORMAL_WT|MT_P_RW_U_RW)
#endif

#ifdef CONFIG_MEM_NO_CACHE
#define MEM_ATTR_ACESS (MT_NORMAL_NC|MT_P_RW_U_RW)
#endif


/************************** Variable Definitions *****************************/

/* Polling information used by remoteproc operations.
 */
static metal_phys_addr_t poll_phys_addr = POLL_BASE_ADDR;
struct metal_device kick_device = {
	.name = "poll_dev",
	.bus = NULL,
	.num_regions = 1,
	.regions = {
		{
			.virt = (void *)POLL_BASE_ADDR,
			.physmap = &poll_phys_addr,
			.size = 0x1000,
			.page_shift = -1UL,
			.page_mask = -1UL,
			.mem_flags = MEM_ATTR_ACESS,
			.ops = {NULL},
		}
	},
	.node = {NULL},
#ifdef CONFIG_USE_OPENAMP_IPI
	.irq_num = 1,
	.irq_info = (void *)CONFIG_IPI_IRQ_NUM,
#endif /* !RPMSG_NO_IPI */
};

static struct remoteproc_priv rproc_priv = {
	.kick_dev_name = KICK_DEV_NAME,
	.kick_dev_bus_name = KICK_BUS_NAME,
#ifdef CONFIG_USE_OPENAMP_IPI
	.ipi_chn_mask = CONFIG_IPI_CHN_BITMASK,//IPI_CHN_BITMASK,
#endif /* !RPMSG_NO_IPI */
};

#ifdef CONFIG_USE_MASTER_VRING_DEFINE
static metal_phys_addr_t linux_share_buffer;
#endif

static struct remoteproc rproc_inst;
/* notification operation and remote processor managementi operations. */
extern struct remoteproc_ops phytium_proc_ops;
/* RPMsg virtio shared buffer pool */
static struct rpmsg_virtio_shm_pool shpool;

extern struct remote_resource_table resources;


/************************** Function Prototypes ******************************/

static struct remoteproc *platform_create_proc(int proc_index, int rsc_index)
{
	void *rsc_table;
	int rsc_size;
	int ret;
	u32 cpu_id;
	metal_phys_addr_t pa;

	(void)proc_index;

	rsc_table = get_resource_table(rsc_index, &rsc_size);
	/* rsc_size = sizeof(resources); */
	/* resource_table_dump(); */
	/* Register IPI device */
	if (metal_register_generic_device(&kick_device))
		return NULL;
	/* Initialize remoteproc instance */
	/* metal_device_open(KICK_BUS_NAME,KICK_DEV_NAME,rproc_inst->priv->kick_dev) */
	/* rproc_inst->priv->kick_io = metal_device_io_region(rproc_inst->priv->kick_dev, 0); */
	if (!remoteproc_init(&rproc_inst, &phytium_proc_ops, &rproc_priv))
		return NULL;

	/* Mmap shared memories Or shall we constraint that they will be set as carved out in the resource table?. mmap resource table */
	/*
	@da: device address
	@pa: physical address
	*/
	pa = (metal_phys_addr_t)rsc_table;
	/* rproc_inst.mems  rproc_inst.rsc_io*/
	/* 在OpenAMP应用中，通常需要通过remoteproc_mmap()函数将远程处理器中的共享内存映射到本地主机中，以便应用程序进行读写操作。*/
	(void *)remoteproc_mmap(&rproc_inst, &pa, NULL, rsc_size, MEM_ATTR_ACESS, &rproc_inst.rsc_io);
	/* mmap shared memory */
	pa = SHARED_MEM_PA;
	/* rproc_inst.mems */
	(void *)remoteproc_mmap(&rproc_inst, &pa, NULL, SHARED_MEM_SIZE, MEM_ATTR_ACESS, NULL);

	/* linux kernel malloc addr */
#ifdef CONFIG_USE_MASTER_VRING_DEFINE
	pa = resources.rpmsg_vdev.vring[0].da; /* 默认kernel vring[0].da 的首地址是整个sharebuffer 的起始位置 */
	(void *)remoteproc_mmap(&rproc_inst, &pa, NULL, SHARED_MEM_SIZE * 2, MEM_ATTR_ACESS, NULL);
	linux_share_buffer = pa;
#endif
	/* parse resource table to remoteproc */
	/*
	rproc_inst->rsc_table = rsc_table;
	rproc_inst->rsc_len = rsc_size;
	rproc_inst->rsc_io = io;
	*/
	ret = remoteproc_set_rsc_table(&rproc_inst, rsc_table, rsc_size);
	if (ret) {
		FT_PLAT_INFO_MAIN_DEBUG_E("Failed to intialize remoteproc %d \r\n",ret);
		remoteproc_remove(&rproc_inst);
		return NULL;
	}

	FT_PLAT_INFO_MAIN_DEBUG_I("Initialize remoteproc successfully.\r\n");

	return &rproc_inst;
}

extern int init_system(void);
extern void cleanup_system(void);

int platform_init(int argc, char *argv[], void **platform)
{
	unsigned long proc_id = 0;
	unsigned long rsc_id = 0;
	struct remoteproc *rproc;

	if (!platform) {
		FT_PLAT_INFO_MAIN_DEBUG_E("Failed to initialize platform, NULL pointer to store platform data.\r\n");
		return -EINVAL;
	}

	/* Initialize HW system components */
	init_system();

	if (argc >= 2)
		proc_id = strtoul(argv[1], NULL, 0);

	if (argc >= 3)
		rsc_id = strtoul(argv[2], NULL, 0);
	/*  */
	rproc = platform_create_proc(proc_id, rsc_id);
	if (!rproc) {
		FT_PLAT_INFO_MAIN_DEBUG_E("Failed to create remoteproc device.\r\n");
		return -EINVAL;
	}

	*platform = rproc;

	return 0;
}

struct rpmsg_device *platform_create_rpmsg_vdev(void *platform, unsigned int vdev_index, unsigned int role,
												void (*rst_cb)(struct virtio_device *vdev), rpmsg_ns_bind_cb ns_bind_cb)
{
	struct remoteproc *rproc = platform;
	struct rpmsg_virtio_device *rpmsg_vdev;
	struct virtio_device *vdev;
	void *shbuf;
	struct metal_io_region *shbuf_io;
	int ret;

	rpmsg_vdev = metal_allocate_memory(sizeof(*rpmsg_vdev));
	if (!rpmsg_vdev)
		return NULL;

#ifdef CONFIG_USE_MASTER_VRING_DEFINE
	shbuf_io = remoteproc_get_io_with_pa(rproc, linux_share_buffer);
	if (!shbuf_io)
		goto err1;
	FT_PLAT_INFO_MAIN_DEBUG_I("linux_share_buffer is %p \r\n",linux_share_buffer);
	shbuf = metal_io_phys_to_virt(shbuf_io, linux_share_buffer + SHARED_BUF_OFFSET);
#else
	shbuf_io = remoteproc_get_io_with_pa(rproc, SHARED_MEM_PA);
	FT_PLAT_INFO_MAIN_DEBUG_I("shbuf_io is %p \r\n",shbuf_io);
	if (!shbuf_io)
		goto err1;

	shbuf = metal_io_phys_to_virt(shbuf_io, SHARED_MEM_PA + SHARED_BUF_OFFSET);
#endif

	FT_PLAT_INFO_MAIN_DEBUG_I("creating remoteproc virtio\r\n");
	/* TODO: can we have a wrapper for the following two functions? */
	vdev = remoteproc_create_virtio(rproc, vdev_index, role, rst_cb);
	if (!vdev) {
		FT_PLAT_INFO_MAIN_DEBUG_E("failed remoteproc_create_virtio\r\n");
		goto err1;
	}

	FT_PLAT_INFO_MAIN_DEBUG_I("initializing rpmsg shared buffer pool\r\n");
	/* Only RPMsg virtio master needs to initialize the shared buffers pool */
	rpmsg_virtio_init_shm_pool(&shpool, shbuf, (SHARED_MEM_SIZE - SHARED_BUF_OFFSET));

	/* RPMsg virtio slave can set shared buffers pool argument to NULL */
	ret = rpmsg_init_vdev(rpmsg_vdev, vdev, ns_bind_cb, shbuf_io, &shpool);
	if (ret) {
		FT_PLAT_INFO_MAIN_DEBUG_E("failed rpmsg_init_vdev\r\n");
		goto err2;
	}

	FT_PLAT_INFO_MAIN_DEBUG_I("initializing rpmsg vdev\r\n");
	return rpmsg_virtio_get_rpmsg_device(rpmsg_vdev);

err2:
	remoteproc_remove_virtio(rproc, vdev);
err1:
	metal_free_memory(rpmsg_vdev);
	return NULL;
}

int platform_poll(void *priv)
{
	struct remoteproc *rproc = priv;
	struct remoteproc_priv *prproc;
	unsigned int flags;
	int ret;


	prproc = rproc->priv;
	while(1) {
#ifndef CONFIG_USE_OPENAMP_IPI
		if (metal_io_read32(prproc->kick_io, 0) & POLL_STOP) { //RPROC_M2S_SHIFT
			ret = remoteproc_get_notification(rproc, RSC_NOTIFY_ID_ANY);
			if (ret)
				return ret;
			break;
		}
		(void)flags;
#else /* !RPMSG_NO_IPI */
		flags = metal_irq_save_disable();
		if (!(atomic_flag_test_and_set(&prproc->ipi_nokick)))
		{
			metal_irq_restore_enable(flags);
			ret = remoteproc_get_notification(rproc, RSC_NOTIFY_ID_ANY);
			if (ret)
				return ret;
			break;
		}
		_rproc_wait();
		metal_irq_restore_enable(flags);
#endif /* RPMSG_NO_IPI */
	}
	return 0;
}


void platform_release_rpmsg_vdev(struct rpmsg_device *rpdev, void *platform)
{
	struct rpmsg_virtio_device *rpvdev;
	struct remoteproc *rproc;

	rpvdev = metal_container_of(rpdev, struct rpmsg_virtio_device, rdev);
	rproc = platform;

	rpmsg_deinit_vdev(rpvdev);
	remoteproc_remove_virtio(rproc, rpvdev->vdev);
}

int platform_cleanup(void *platform)
{
	int ret = 0;
	struct remoteproc *rproc = platform;

	if (rproc)
		ret = remoteproc_remove(rproc);
	cleanup_system();

	return ret;
}
