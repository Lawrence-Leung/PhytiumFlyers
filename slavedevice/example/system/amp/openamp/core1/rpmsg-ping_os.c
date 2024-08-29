/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
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
 * FilePath: rpmsg-ping_os.c
 * Date: 2022-02-25 09:12:07
 * LastEditTime: 2022-02-25 09:12:19
 * Description: This file is for  a sample demonstration application that showcases usage of rpmsg.
 *              This application is meant to run on the remote CPU running baremetal code.
 *              This application echoes back data that was sent to it by the master core.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 huanghe    2022/03/25  first commit
 */

/* This is a sample demonstration application that showcases usage of rpmsg 
This application is meant to run on the remote CPU running baremetal code. 
This application echoes back data that was sent to it by the master core. */

/***************************** Include Files *********************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openamp/open_amp.h>
#include <metal/alloc.h>
#include "ftypes.h"
#include "platform_info.h"
#include "rpmsg-echo.h"
#include "FreeRTOS.h"
#include "task.h"
#include "fdebug.h"


/***************** Macros (Inline Functions) Definitions *********************/

#define LPRINTF(format, ...) printf(format, ##__VA_ARGS__)
#define LPERROR(format, ...) LPRINTF("ERROR: " format, ##__VA_ARGS__)

#define OPENAMP_MASTER_DEBUG_TAG "OPENAMP_MASTER"
#define OPENAMP_MASTER_ERROR(format, ...) FT_DEBUG_PRINT_E(OPENAMP_MASTER_DEBUG_TAG, format, ##__VA_ARGS__)
#define OPENAMP_MASTER_WARN(format, ...)  FT_DEBUG_PRINT_W(OPENAMP_MASTER_DEBUG_TAG, format, ##__VA_ARGS__)
#define OPENAMP_MASTER_INFO(format, ...)  FT_DEBUG_PRINT_I(OPENAMP_MASTER_DEBUG_TAG, format, ##__VA_ARGS__)
#define OPENAMP_MASTER_DEBUG(format, ...) FT_DEBUG_PRINT_D(OPENAMP_MASTER_DEBUG_TAG, format, ##__VA_ARGS__)


/**************************** Type Definitions *******************************/

struct _payload {
	unsigned long num;
	unsigned long size;
	unsigned char data[];
};

static int err_cnt;
static char flg_cnt;

#define PAYLOAD_MIN_SIZE    1


/************************** Variable Definitions *****************************/

/* Globals */
static struct rpmsg_endpoint lept;
static struct _payload *i_payload;
static int rnum = 0;
static int err_cnt = 0;
static int ept_deleted = 0;

/************************** Function Prototypes ******************************/

/*-----------------------------------------------------------------------------*
 *  RPMSG endpoint callbacks
 *-----------------------------------------------------------------------------*/
static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
			     uint32_t src, void *priv)
{
	int i;
	struct _payload *r_payload = (struct _payload *)data;

	(void)ept;
	(void)src;
	(void)priv;

	if (r_payload->size == 0) {
		LPERROR(" Invalid size of package is received.\r\n");
		err_cnt++;
		return RPMSG_SUCCESS;
	}
	/* Validate data buffer integrity. */
	for (i = 0; i < (int)r_payload->size; i++) {
		if (r_payload->data[i] != flg_cnt) {
			LPRINTF("Data corruption at index %d\r\n", i);
			LPRINTF("Want data is %d\r\n", flg_cnt);
			LPRINTF("Get data is %d\r\n", r_payload->data[i]);
			err_cnt++;
			break;
		}
	}
	rnum = r_payload->num + 1;
	return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
	(void)ept;
	rpmsg_destroy_ept(&lept);
	LPRINTF("echo test: service is destroyed\r\n");
	ept_deleted = 1;
}

static void rpmsg_name_service_bind_cb(struct rpmsg_device *rdev,
				       const char *name, uint32_t dest)
{
	LPRINTF("new endpoint notification is received.\r\n");
	if (strcmp(name, RPMSG_SERVICE_NAME))
		LPERROR("Unexpected name service %s.\r\n", name);
	else
		(void)rpmsg_create_ept(&lept, rdev, RPMSG_SERVICE_NAME,
				       RPMSG_ADDR_ANY, dest,
				       rpmsg_endpoint_cb,
				       rpmsg_service_unbind);

}

/*-----------------------------------------------------------------------------*
 *  Application
 *-----------------------------------------------------------------------------*/
int app(struct rpmsg_device *rdev, void *priv)
{
	int ret;
	int i;
	int size, max_size, num_payloads;
	int expect_rnum = 0;



	max_size = rpmsg_virtio_get_buffer_size(rdev);
	if (max_size < 0) {
		LPERROR("No avaiable buffer size.\r\n");
		return -1;
	}
	max_size -= sizeof(struct _payload);
	num_payloads = max_size - PAYLOAD_MIN_SIZE + 1;
	i_payload =
	    (struct _payload *)metal_allocate_memory(2 * sizeof(unsigned long) +
				      max_size);

	if (!i_payload) {
		LPERROR("memory allocation failed.\r\n");
		return -1;
	}
	
	OPENAMP_MASTER_INFO("step7 : user to init endpoint ");
	/* Create RPMsg endpoint */
	ret = rpmsg_create_ept(&lept, rdev, RPMSG_SERVICE_NAME,
			       RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
			       rpmsg_endpoint_cb, rpmsg_service_unbind);

	if (ret) {
		LPERROR("Failed to create RPMsg endpoint.\r\n");
		metal_free_memory(i_payload);
		return ret;
	}
	OPENAMP_MASTER_INFO("step8 : user to wait vq code ");
	while (!is_rpmsg_ept_ready(&lept))
		platform_poll(priv);

	for (i = 0, size = PAYLOAD_MIN_SIZE; i < num_payloads; i++, size++) 
	{
		i_payload->num  = i;
		i_payload->size = size;
		flg_cnt++;
		/* Mark the data buffer. */
		memset(&(i_payload->data[0]), flg_cnt, size);

		ret = rpmsg_send(&lept, i_payload,
				 (2 * sizeof(unsigned long)) + size);

		if (ret < 0) {
			LPERROR("Failed to send data...\r\n");
			break;
		}

		expect_rnum++;
		do {
			platform_poll(priv);
		} while ((rnum < expect_rnum) && !err_cnt && !ept_deleted);

	}

	LPRINTF("**********************************\r\n");
	LPRINTF(" Test Results: Error count = %d \r\n", err_cnt);
	LPRINTF("**********************************\r\n");
	OPENAMP_MASTER_INFO("step9 : start to exit ");
	/* Destroy the RPMsg endpoint */
	rpmsg_destroy_ept(&lept);

	metal_free_memory(i_payload);
	return 0;
}


static void rpmsg_ping(void *args)
{
	void *platform;
	struct rpmsg_device *rpdev;
	int ret = 0;
	/* Initialize platform */
	OPENAMP_MASTER_INFO("start application");
	ret = platform_init(0, NULL, &platform);
	if (ret) {
		LPERROR("Failed to initialize platform.\r\n");
	}
	else
	{
		OPENAMP_MASTER_INFO("step3: start to init virtio device ");
		rpdev = platform_create_rpmsg_vdev(platform, 0,
						VIRTIO_DEV_MASTER,
						NULL,
						rpmsg_name_service_bind_cb);
		if (!rpdev) {
			LPERROR("Failed to create platform_create_rpmsg_vdev\r\n");
		}
		else
		{
			app(rpdev, platform);
			platform_release_rpmsg_vdev(rpdev, platform);
		}
	}

	OPENAMP_MASTER_INFO("Stopping application...\r\n");
	platform_cleanup(platform);

	vTaskDelete(NULL);
}




int rpmsg_ping_task(void)
{
    BaseType_t ret; 
    
    ret = xTaskCreate((TaskFunction_t )rpmsg_ping, /* 任务入口函数 */
                        (const char* )"rpmsg_ping",/* 任务名字 */
                        (uint16_t )4096, /* 任务栈大小 */
                        (void* )NULL,/* 任务入口函数参数 */
                        (UBaseType_t )1, /* 任务的优先级 */
                        NULL); /* 任务控制块指针 */

    if(ret != pdPASS)
    {
        LPERROR("Failed to create a rpmsg_echo task ");
        return -1;
    }

    return 0;
}

