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
 * FilePath: platform_info.h
 * Date: 2022-02-23 11:24:12
 * LastEditTime: 2022-02-23 11:43:53
 * Description:  This file define platform specific data and implements APIs to set
 *       platform specific information for OpenAMP. 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0	 huanghe	2022/03/06   first release
 */


#ifndef PLATFORM_INFO_H_
#define PLATFORM_INFO_H_

/***************************** Include Files *********************************/
#include <openamp/remoteproc.h>
#include <openamp/virtio.h>
#include <openamp/rpmsg.h>
#include "sdkconfig.h"



/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/


#if defined __cplusplus
extern "C" {
#endif

/***************** Macros (Inline Functions) Definitions *********************/

/* Cortex R5 memory attributes */
#define DEVICE_SHARED		0x00000001U /* device, shareable */
#define DEVICE_NONSHARED	0x00000010U /* device, non shareable */
#define NORM_NSHARED_NCACHE	0x00000008U /* Non cacheable  non shareable */
#define NORM_SHARED_NCACHE	0x0000000CU /* Non cacheable shareable */
#define	PRIV_RW_USER_RW		(0x00000003U<<8U) /* Full Access */


#define SHARED_MEM_PA  		CONFIG_VRING_TX_ADDR
#define POLL_BASE_ADDR      CONFIG_POLL_BASE_ADDR

#define SHARED_MEM_SIZE 	0x100000UL	/* 必须要和kernel 进行约定 */
#define SHARED_BUF_OFFSET 	0x8000UL


#ifndef CONFIG_USE_OPENAMP_IPI
#define  POLL_STOP 0x1U
#endif

/**************************** Type Definitions *******************************/

struct remoteproc_priv {
	const char *kick_dev_name;
	const char *kick_dev_bus_name;
	struct metal_device *kick_dev;
	struct metal_io_region *kick_io;
#ifdef CONFIG_USE_OPENAMP_IPI
	unsigned int ipi_chn_mask; /**< IPI channel mask */
	atomic_int ipi_nokick;/*  */
#endif /* !RPMSG_NO_IPI */
	unsigned int cpu_mask;
};

/************************** Function Prototypes ******************************/

/**
 * platform_init - initialize the platform
 *
 * It will initialize the platform.
 *
 * @argc: number of arguments
 * @argv: array of the input arguements
 * @platform: pointer to store the platform data pointer
 *
 * return 0 for success or negative value for failure
 */
int platform_init(int argc, char *argv[], void **platform);

/**
 * platform_create_rpmsg_vdev - create rpmsg vdev
 *
 * It will create rpmsg virtio device, and returns the rpmsg virtio
 * device pointer.
 *
 * @platform: pointer to the private data
 * @vdev_index: index of the virtio device, there can more than one vdev
 *              on the platform.
 * @role: virtio master or virtio slave of the vdev
 * @rst_cb: virtio device reset callback
 * @ns_bind_cb: rpmsg name service bind callback
 *
 * return pointer to the rpmsg virtio device
 */
struct rpmsg_device *
platform_create_rpmsg_vdev(void *platform, unsigned int vdev_index,
			   unsigned int role,
			   void (*rst_cb)(struct virtio_device *vdev),
			   rpmsg_ns_bind_cb ns_bind_cb);

/**
 * platform_poll - platform poll function
 *
 * @platform: pointer to the platform
 *
 * return negative value for errors, otherwise 0.
 */
int platform_poll(void *platform);

/**
 * platform_release_rpmsg_vdev - release rpmsg virtio device
 *
 * @rpdev: pointer to the rpmsg device
 */
void platform_release_rpmsg_vdev(struct rpmsg_device *rpdev, void *platform);

/**
 * platform_cleanup - clean up the platform resource
 *
 * @platform: pointer to the platform
 */
int platform_cleanup(void *platform);


#if defined __cplusplus
}
#endif


#endif
