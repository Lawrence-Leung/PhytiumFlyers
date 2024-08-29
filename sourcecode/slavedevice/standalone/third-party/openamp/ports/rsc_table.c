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
 * FilePath: rsc_table.c
 * Date: 2022-02-23 11:24:12
 * LastEditTime: 2022-02-23 11:43:59
 * Description:  This file populates resource table for BM remote
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0	 huanghe	2022/03/06   first release
 */


/***************************** Include Files *********************************/

#include <openamp/open_amp.h>
#include "rsc_table.h"
#include "stdio.h"
#include "sdkconfig.h"

/***************** Macros (Inline Functions) Definitions *********************/

/* Place resource table in special ELF section */
#define __section_t(S)          __attribute__((__section__(#S)))
#define __resource              __section_t(.resource_table)

#define RPMSG_IPU_C0_FEATURES        1

/* VirtIO rpmsg device id */
#define VIRTIO_ID_RPMSG_             7

/* notifyid is a unique rproc-wide notify index for this vdev */
#define VDEV_NOTIFYID				0
/* Remote supports Name Service announcement */
#define VIRTIO_RPMSG_F_NS           0

#define NUM_VRINGS                  0x02
#define VRING_ALIGN                 0x1000
#define RING_TX						CONFIG_VRING_TX_ADDR
#define RING_RX						CONFIG_VRING_RX_ADDR
#define VRING_SIZE 					CONFIG_VRING_SIZE

#define NUM_TABLE_ENTRIES           1

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

struct remote_resource_table __resource resources = {
	/* Version */
	1,

	/* NUmber of table entries */
	NUM_TABLE_ENTRIES,
	/* reserved fields */
	{0, 0,},

	/* Offsets of rsc entries */
	{
	 offsetof(struct remote_resource_table, rpmsg_vdev),
	 },

	/* Virtio device entry */
	{
	 RSC_VDEV, VIRTIO_ID_RPMSG_, VDEV_NOTIFYID, RPMSG_IPU_C0_FEATURES, 0, 0, 0,
	 NUM_VRINGS, {0, 0},
	 },

	/* Vring rsc entry - part of vdev rsc entry */
	{RING_TX, VRING_ALIGN, VRING_SIZE, 1, 0},
	{RING_RX, VRING_ALIGN, VRING_SIZE, 2, 0},
};

void *get_resource_table (int rsc_id, int *len)
{
	(void) rsc_id;
	*len = sizeof(resources);
	return &resources;
}




/************************** Function Prototypes ******************************/

void resource_table_dump(void)
{
	printf("version is %d \r\n",resources.version);
	printf("num is %d \r\n",resources.num);
	printf("offset is %d \r\n",resources.offset[0]);
	printf("rpmsg_vdev.type is %d \r\n",resources.rpmsg_vdev.type);
	printf("rpmsg_vdev.id is %d \r\n",resources.rpmsg_vdev.id);
	printf("rpmsg_vdev.notifyid is %d \r\n",resources.rpmsg_vdev.notifyid);
	printf("rpmsg_vdev.dfeatures is %d \r\n",resources.rpmsg_vdev.dfeatures);
	printf("rpmsg_vdev.gfeatures is %d \r\n",resources.rpmsg_vdev.gfeatures);
	printf("rpmsg_vdev.config_len is %d \r\n",resources.rpmsg_vdev.config_len);
	printf("rpmsg_vdev.status is %d \r\n",resources.rpmsg_vdev.status);
	printf("rpmsg_vdev.num_of_vrings is %d \r\n",resources.rpmsg_vdev.num_of_vrings);
	printf("rpmsg_vdev.vring[0].da is %x \r\n",resources.rpmsg_vdev.vring[0].da);
	printf("rpmsg_vdev.vring[0].align is %d \r\n",resources.rpmsg_vdev.vring[0].align);
	printf("rpmsg_vdev.vring[0].num is %d \r\n",resources.rpmsg_vdev.vring[0].num);
	printf("rpmsg_vdev.vring[0].notifyid is %d \r\n",resources.rpmsg_vdev.vring[0].notifyid);
	printf("rpmsg_vdev.vring[0].reserved is %d \r\n",resources.rpmsg_vdev.vring[0].reserved);
	printf("rpmsg_vdev.vring[1].da is %x \r\n",resources.rpmsg_vdev.vring[1].da);
	printf("rpmsg_vdev.vring[1].align is %d \r\n",resources.rpmsg_vdev.vring[1].align);
	printf("rpmsg_vdev.vring[1].num is %d \r\n",resources.rpmsg_vdev.vring[1].num);
	printf("rpmsg_vdev.vring[1].notifyid is %d \r\n",resources.rpmsg_vdev.vring[1].notifyid);
	printf("rpmsg_vdev.vring[1].reserved is %d \r\n",resources.rpmsg_vdev.vring[1].reserved);
}
