/*
 * @Copyright : (C) 2022 Phytium Information Technology, Inc. 
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
 * @FilePath: fmem_image_store.c
 * @Date: 2023-05-19 10:54:34
 * @LastEditTime: 2023-05-19 10:54:35
 * @Description:  This file is for memory based image store operation
 * 
 * @Modify History: 
 *  Ver   Who           Date        Changes
 * ----- ------         --------    --------------------------------------
 * 1.0  liushengming    2023/05/19  init
 */

#include <metal/atomic.h>
#include <metal/assert.h>
#include <metal/device.h>
#include <metal/io.h>
#include <metal/sys.h>
#include <openamp/rpmsg_virtio.h>
#include <openamp/remoteproc_loader.h>
#include <errno.h>
#include "platform_info.h"
#include "fdebug.h"

#define     MEM_IMAGE_STORE_DEBUG "    MEM_IMAGE_STORE"
#define     MEM_IMAGE_STORE_DEBUG_I(format, ...) FT_DEBUG_PRINT_I( MEM_IMAGE_STORE_DEBUG, format, ##__VA_ARGS__)
#define     MEM_IMAGE_STORE_DEBUG_W(format, ...) FT_DEBUG_PRINT_W( MEM_IMAGE_STORE_DEBUG, format, ##__VA_ARGS__)
#define     MEM_IMAGE_STORE_DEBUG_E(format, ...) FT_DEBUG_PRINT_E( MEM_IMAGE_STORE_DEBUG, format, ##__VA_ARGS__)


struct mem_file {
	const void *base;
};

int mem_image_open(void *store, const char *path, const void **image_data)
{
	struct mem_file *image = store;
	const void *fw_base = image->base;

	(void)(path);
	if (image_data == NULL) 
    {
		MEM_IMAGE_STORE_DEBUG_E("%s: input image_data is NULL\r\n", __func__);
		return -EINVAL;
	}
	*image_data = fw_base;
	/* return an abitrary length, as the whole firmware is in memory */
	return 0x100;
}

void mem_image_close(void *store)
{
	/* The image is in memory, does nothing */
	(void)store;
}

int mem_image_load(void *store, size_t offset, size_t size,
		   const void **data, metal_phys_addr_t pa,
		   struct metal_io_region *io,
		   char is_blocking)
{
	struct mem_file *image = store;
	const void *fw_base = image->base;

	(void)is_blocking;

	MEM_IMAGE_STORE_DEBUG_I("%s: offset=0x%x, size=0x%x\n\r", __func__, offset, size);
	if (pa == METAL_BAD_PHYS) 
    {
		if (data == NULL) 
        {
			MEM_IMAGE_STORE_DEBUG_E("%s: data is NULL while pa is ANY\r\n", __func__);
			return -EINVAL;
		}
		*data = (const void *)((const char *)fw_base + offset);
	} 
    else 
    {
		void *va;

		if (io == NULL) 
        {
			MEM_IMAGE_STORE_DEBUG_E("%s, io is NULL while pa is not ANY\r\n", __func__);
			return -EINVAL;
		}
		va = metal_io_phys_to_virt(io, pa);
		if (va == NULL) 
        {
			MEM_IMAGE_STORE_DEBUG_E("%s: no va is found\r\n", __func__);
			return -EINVAL;
		}
		memcpy(va, (const void *)((const char *)fw_base + offset), size);
	}

	return (int)size;
}

struct image_store_ops mem_image_store_ops = {
	.open = mem_image_open,
	.close = mem_image_close,
	.load = mem_image_load,
	.features = SUPPORT_SEEK,
};

