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
 * FilePath: phytium_rproc.c
 * Date: 2022-03-08 22:26:17
 * LastEditTime: 2022-03-09 13:16:04
 * Description:  This file define phytium platform specific remoteproc
 *       implementation.
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
#include "platform_info.h"
#include "sdkconfig.h"
#include "fdebug.h"
#include "finterrupt.h"
#include "stdio.h"
#include "fmmu.h"
#include "ftypes.h"
#include "fcpu_info.h"
#include "fpsci.h"
#include "fcache.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define     PHYTIUM_RPROC_MAIN_DEBUG_TAG "    PHYTIUM_RPROC_MAIN"
#define     PHYTIUM_RPROC_MAIN_DEBUG_I(format, ...) FT_DEBUG_PRINT_I( PHYTIUM_RPROC_MAIN_DEBUG_TAG, format, ##__VA_ARGS__)
#define     PHYTIUM_RPROC_MAIN_DEBUG_W(format, ...) FT_DEBUG_PRINT_W( PHYTIUM_RPROC_MAIN_DEBUG_TAG, format, ##__VA_ARGS__)
#define     PHYTIUM_RPROC_MAIN_DEBUG_E(format, ...) FT_DEBUG_PRINT_E( PHYTIUM_RPROC_MAIN_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/


#ifdef CONFIG_MEM_NORMAL
#define DEFAULT_MEM_ATTRIBUTE (MT_NORMAL|MT_P_RW_U_RW)
#endif

#ifdef CONFIG_MEM_WRITE_THROUGH
#define DEFAULT_MEM_ATTRIBUTE (MT_NORMAL_WT|MT_P_RW_U_RW)
#endif

#ifdef CONFIG_MEM_NO_CACHE
#define DEFAULT_MEM_ATTRIBUTE (MT_NORMAL_NC|MT_P_RW_U_RW)
#endif

#ifdef  CONFIG_USE_OPENAMP_IPI

static void PhytiumIrqhandler(s32 vector, void *param)
{
    struct remoteproc *rproc = param;
	struct remoteproc_priv *prproc;
	u32 cpu_id;
    (void)vector;

    if(!rproc) {
		PHYTIUM_RPROC_MAIN_DEBUG_E("rproc is empty.") ;
        return ;
    }

	prproc = rproc->priv;
	atomic_flag_clear(&prproc->ipi_nokick);
}

#endif

static struct remoteproc *
PhytiumProcInit(struct remoteproc *rproc, struct remoteproc_ops *ops,
			void *arg)
{
    struct remoteproc_priv *prproc = arg;
	struct metal_device *kick_dev;
	unsigned int irq_vect;
	int ret;

    if (!rproc || !prproc || !ops)
		return NULL;
	ret = metal_device_open(prproc->kick_dev_bus_name,
				prproc->kick_dev_name,
				&kick_dev);
	if (ret) {
		PHYTIUM_RPROC_MAIN_DEBUG_E("Failed to open polling device: %d.", ret);
		return NULL;
	}
    rproc->priv = prproc;
	prproc->kick_dev = kick_dev;
	prproc->kick_io = metal_device_io_region(kick_dev, 0);
    if (!prproc->kick_io)
        goto err1;

#ifdef CONFIG_USE_OPENAMP_IPI
	u32 cpu_id;
	atomic_store(&prproc->ipi_nokick, 1);
	GetCpuId(&cpu_id);
	/* Register interrupt handler and enable interrupt */
	irq_vect = (uintptr_t)kick_dev->irq_info;
	PHYTIUM_RPROC_MAIN_DEBUG_I("current %d",cpu_id) ;
	InterruptSetPriority(irq_vect, IRQ_PRIORITY_VALUE_1) ;
    InterruptInstall(irq_vect,PhytiumIrqhandler,rproc,"phytium_rproc") ;
	PHYTIUM_RPROC_MAIN_DEBUG_I("irq_vect is %d",irq_vect) ;
    InterruptUmask(irq_vect) ;
#else
	(void)irq_vect;
	metal_io_write32(prproc->kick_io, 0, !POLL_STOP);
#endif /* !CONFIG_USE_OPENAMP_IPI */

	rproc->ops = ops;

	return rproc;
err1:
	metal_device_close(kick_dev);
	return NULL;
}


static void PhytiumProcRemove(struct remoteproc *rproc)
{
    struct remoteproc_priv *prproc;
	struct metal_device *dev;

    if (!rproc)
	{
		return;
	}
    
	prproc = rproc->priv;
#ifdef CONFIG_USE_OPENAMP_IPI
	dev = prproc->kick_dev;
	if (dev) {
		PHYTIUM_RPROC_MAIN_DEBUG_E("Start to remove.") ;
		InterruptMask((uintptr_t)dev->irq_info);
	}
#else /* RPMSG_NO_IPI */
	(void)dev;
#endif /* !RPMSG_NO_IPI */
	metal_device_close(prproc->kick_dev);

}

static void *
PhytiumProcMmap(struct remoteproc *rproc, metal_phys_addr_t *pa,
			metal_phys_addr_t *da, size_t size,
			unsigned int attribute, struct metal_io_region **io)
{
    struct remoteproc_mem *mem;
	metal_phys_addr_t lpa, lda;
	struct metal_io_region *tmpio;

    lpa = *pa;
	lda = *da;

	if (lpa == METAL_BAD_PHYS && lda == METAL_BAD_PHYS)
		return NULL;
	if (lpa == METAL_BAD_PHYS)
		lpa = lda;
	if (lda == METAL_BAD_PHYS)
		lda = lpa;

	if (!attribute)
		attribute = DEFAULT_MEM_ATTRIBUTE;

	mem = metal_allocate_memory(sizeof(*mem));
	if (!mem)
		return NULL;
	tmpio = metal_allocate_memory(sizeof(*tmpio));
	if (!tmpio) {
		metal_free_memory(mem);
		return NULL;
	}
	/*
	mem->pa = lpa;
	mem->da = lda;
	mem->io = tmpio;
	mem->size = size; 
	*/
	remoteproc_init_mem(mem, NULL, lpa, lda, size, tmpio);
	/* 
	va is the same as pa in this platform,
	tmpio->virt = lpa;
	tmpio->physmap = &mem->pa;
	tmpio->size = size;
	tmpio->page_shift = -1;
	tmpio->mem_flags = attribute;
	tmpio->ops = ops ? *ops : nops; const struct metal_io_ops nops = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
	*/
	metal_io_init(tmpio, (void *)lpa, &mem->pa, size, -1, attribute, NULL);
	/*
	 Add remoteproc memory
	 &rproc->mems, add &mem->node
	*/
	remoteproc_add_mem(rproc, mem);
	*pa = lpa;
	*da = lda;
	if (io)
		*io = tmpio;
	return metal_io_phys_to_virt(tmpio, mem->pa);

}

static int PhytiumProcNotify(struct remoteproc *rproc, uint32_t id)
{
	struct remoteproc_priv *prproc;

	(void)id;
	if (!rproc)
		return -1;
	prproc = rproc->priv;

#ifndef CONFIG_USE_OPENAMP_IPI
	metal_io_write32(prproc->kick_io, 0, POLL_STOP);
#else
	u32 cpu_id;
	GetCpuId(&cpu_id);
	InterruptCoreInterSend((uintptr)(prproc->kick_dev->irq_info), prproc->ipi_chn_mask);
#endif /* RPMSG_NO_IPI */

	return 0;
}

static int Phytium_rproc_start(struct remoteproc *rproc)
{
	struct remoteproc_priv *priv;
	FError ret;

	priv = rproc->priv;
	FCacheDCacheFlush();
	ret = FPsciCpuMaskOn(priv->cpu_mask, (uintptr_t)rproc->bootaddr);
	if (ret != 0) {
		PHYTIUM_RPROC_MAIN_DEBUG_E("Failed to start core mask 0x%x, ret=0x%x\n\r", priv->cpu_mask, ret);
		return -1;
	} 

	return 0;
}

static int PhytiumProcStop(struct remoteproc *rproc)
{
	/* It is lacking a stop operation in the libPM */
	(void)rproc;
	return 0;
}

static int PhytiumProcShutdown(struct remoteproc *rproc)
{
	struct remoteproc_priv *priv;
	int ret;
	struct remoteproc_mem *mem;
	struct metal_list *node;

	priv = rproc->priv;
	/* Delete all the registered remoteproc memories */
	metal_list_for_each(&rproc->mems, node) {
		struct metal_list *tmpnode;
		metal_phys_addr_t pa, pa_end;

		mem = metal_container_of(node, struct remoteproc_mem, node);
		tmpnode = node;
		node = tmpnode->prev;
		metal_list_del(tmpnode);
		metal_free_memory(mem->io);
		metal_free_memory(mem);
	}
/*
	if (rproc->state == RPROC_RUNNING)
	{
		ret = XPm_ForcePowerDown(priv->cpu_id, REQUEST_ACK_NO);
		if (ret != XST_SUCCESS)
		return -1;
	}
*/
	return 0;
}

struct remoteproc_ops phytium_proc_ops = {
	.init = PhytiumProcInit,
	.remove = PhytiumProcRemove,
	.mmap = PhytiumProcMmap,
	.notify = PhytiumProcNotify,
	.start = Phytium_rproc_start,
	.stop = PhytiumProcStop,
	.shutdown = PhytiumProcShutdown,
};

