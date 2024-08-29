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
 * FilePath: rpmsg-echo.c
 * Date: 2022-03-08 22:00:15
 * LastEditTime: 2022-03-09 10:01:19
 * Description:  This is a sample demonstration application that showcases usage of rpmsg
 *  This application is meant to run on the remote CPU running baremetal code.
 *  This application echoes back data that was sent to it by the master core.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2022/06/20      first release
 */

/***************************** Include Files *********************************/

#include <stdio.h>
#include <openamp/open_amp.h>
#include <metal/alloc.h>
#include "platform_info.h"
#include "rpmsg_service.h"
#include <metal/sleep.h>
#include "rsc_table.h"
#include "shell.h"
#include "fcache.h"
#include "fdebug.h"

/************************** Constant Definitions *****************************/
/***************** Macros (Inline Functions) Definitions *********************/

#define     SHUTDOWN_MSG                0xEF56A55A
#define     ECHO_DEV_SLAVE_DEBUG_TAG "    SLAVE_01"
#define     ECHO_DEV_SLAVE_DEBUG_I(format, ...) FT_DEBUG_PRINT_I( ECHO_DEV_SLAVE_DEBUG_TAG, format, ##__VA_ARGS__)
#define     ECHO_DEV_SLAVE_DEBUG_W(format, ...) FT_DEBUG_PRINT_W( ECHO_DEV_SLAVE_DEBUG_TAG, format, ##__VA_ARGS__)
#define     ECHO_DEV_SLAVE_DEBUG_E(format, ...) FT_DEBUG_PRINT_E( ECHO_DEV_SLAVE_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Variable Definitions *****************************/
static int shutdown_req = 0;

/************************** Function Prototypes ******************************/

/*-----------------------------------------------------------------------------*
 *  RPMSG endpoint callbacks
 *-----------------------------------------------------------------------------*/
static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len, uint32_t src, void *priv)
{
    (void)priv;
    (void)src;

    /* On reception of a shutdown we signal the application to terminate */
    if ((*(unsigned int *)data) == SHUTDOWN_MSG)
    {
        ECHO_DEV_SLAVE_DEBUG_I("Shutdown message is received.\r\n");
        shutdown_req = 1;
        return RPMSG_SUCCESS;
    }

    /* Send data back to master */
    if (rpmsg_send(ept, data, len) < 0)
    {
        ECHO_DEV_SLAVE_DEBUG_E("rpmsg_send failed.\r\n");
    }
    //自报家门，说出自己的src字段是什么
    ECHO_DEV_SLAVE_DEBUG_I("My Name Is %s, My Source Is %d, Hello Lawrence :)", ept->name, src);


    return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
    (void)ept;
    ECHO_DEV_SLAVE_DEBUG_I("Unexpected remote endpoint destroy.\r\n");
    shutdown_req = 1;
}

/*-----------------------------------------------------------------------------*
 *  Application
 *-----------------------------------------------------------------------------*/
// 修改测试版本, by Lawrence Leung 2024.2.29
static int FRpmsgCommunication(struct rpmsg_device *rdev, void *priv)
{
    int ret = 0;
    struct rpmsg_endpoint lept0, lept1, lept2;  //测试创建3个端点，全静态。
    shutdown_req = 0;
    /* Initialize RPMSG framework */
    ECHO_DEV_SLAVE_DEBUG_I("[LAWRENCE] Try to create rpmsg endpoint [0].\r\n");

    // 实验1，创建1个vdev，3个endpoint，全静态
    //ret = rpmsg_create_ept(&lept, rdev, RPMSG_SERVICE_NAME, 0, RPMSG_ADDR_ANY, rpmsg_endpoint_cb, rpmsg_service_unbind);

    //端点0
    ret = rpmsg_create_ept(&lept0, rdev, "channel0", 0, 1, rpmsg_endpoint_cb, rpmsg_service_unbind);
    if (ret)
    {
        ECHO_DEV_SLAVE_DEBUG_E("Failed to create endpoint. %d \r\n", ret);
        return -1;
    }

    //端点1
    ECHO_DEV_SLAVE_DEBUG_I("[LAWRENCE] Try to create rpmsg endpoint [1].\r\n");
    ret = rpmsg_create_ept(&lept1, rdev, "channel1", 10, RPMSG_ADDR_ANY, rpmsg_endpoint_cb, rpmsg_service_unbind);
    if (ret)
    {
        ECHO_DEV_SLAVE_DEBUG_E("Failed to create endpoint. %d \r\n", ret);
        return -1;
    }

    //端点2
    ECHO_DEV_SLAVE_DEBUG_I("[LAWRENCE] Try to create rpmsg endpoint [2].\r\n");
    ret = rpmsg_create_ept(&lept2, rdev, RPMSG_SERVICE_NAME, 0, RPMSG_ADDR_ANY, rpmsg_endpoint_cb, rpmsg_service_unbind);
    if (ret)
    {
        ECHO_DEV_SLAVE_DEBUG_E("Failed to create endpoint. %d \r\n", ret);
        return -1;
    }

    ECHO_DEV_SLAVE_DEBUG_I("Successfully created rpmsg endpoint.\r\n");
    // static u32 test_data = 1 ; 



    while (1)
    {
        platform_poll(priv);
        /* we got a shutdown request, exit */
        if (shutdown_req)
        {
            break;
        }
    }

    //销毁端点，这里需要有3个。当然这是收到了shutdown_req之后才有的
    //rpmsg_destroy_ept(&lept);
    rpmsg_destroy_ept(&lept0);
    rpmsg_destroy_ept(&lept1);
    rpmsg_destroy_ept(&lept2);


    return ret;
}

/*-----------------------------------------------------------------------------*
 *  Application entry point
 *-----------------------------------------------------------------------------*/

int FOpenampExample(void)
{
    int ret = 0;
    void *platform;
    struct rpmsg_device *rpdev;
    /* Initialize platform */
    ret = platform_init(0, NULL, &platform);
    if (ret)
    {
        ECHO_DEV_SLAVE_DEBUG_E("Failed to initialize platform.\r\n");
        platform_cleanup(platform);
        return -1;
    }

    rpdev = platform_create_rpmsg_vdev(platform, 0, VIRTIO_DEV_SLAVE, NULL, NULL);
    if (!rpdev)
    {
        ECHO_DEV_SLAVE_DEBUG_E("Failed to create rpmsg virtio device.\r\n");
        ret = platform_cleanup(platform);
        return ret;
    }
    
    ret = FRpmsgCommunication(rpdev, platform);   //这行对应的是控制Endpoint的创建与函数绑定操作

    if (ret)
    {
        ECHO_DEV_SLAVE_DEBUG_E("Failed to running echoapp");
        return platform_cleanup(platform);
    }
    ECHO_DEV_SLAVE_DEBUG_I("Stopping application...");
    return ret;
}
