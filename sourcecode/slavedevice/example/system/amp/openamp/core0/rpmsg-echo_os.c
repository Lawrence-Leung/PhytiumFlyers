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
 * FilePath: rpmsg-echo_os.c
 * Date: 2022-02-25 09:12:07
 * LastEditTime: 2022-02-25 09:12:19
 * Description:  This file is for  a sample demonstration application that showcases usage of rpmsg.
 *               This application is meant to run on the remote CPU running baremetal code.
 *               This application echoes back data that was sent to it by the master core.
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 huanghe    2022/03/25  first commit    
 */


/***************************** Include Files *********************************/

#include <stdio.h>
#include <openamp/open_amp.h>
#include <openamp/version.h>
#include <metal/alloc.h>
#include <metal/version.h>
#include "platform_info.h"
#include "rpmsg-echo.h"
#include <metal/sleep.h>
#include "rsc_table.h"
#include "FreeRTOS.h"
#include "task.h"
#include "shell.h"
#include "finterrupt.h"
#include "fpsci.h"
#include "fdebug.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define OPENAMP_SLAVE_DEBUG_TAG "OPENAMP_SLAVE"
#define OPENAMP_SLAVE_ERROR(format, ...) FT_DEBUG_PRINT_E(OPENAMP_SLAVE_DEBUG_TAG, format, ##__VA_ARGS__)
#define OPENAMP_SLAVE_WARN(format, ...)  FT_DEBUG_PRINT_W(OPENAMP_SLAVE_DEBUG_TAG, format, ##__VA_ARGS__)
#define OPENAMP_SLAVE_INFO(format, ...)  FT_DEBUG_PRINT_I(OPENAMP_SLAVE_DEBUG_TAG, format, ##__VA_ARGS__)
#define OPENAMP_SLAVE_DEBUG(format, ...) FT_DEBUG_PRINT_D(OPENAMP_SLAVE_DEBUG_TAG, format, ##__VA_ARGS__)

#ifdef CONFIG_DEBUG_CODE
#define OPENAMP_MASTER_ADDRESS  0xe0100000
#endif

#define SHUTDOWN_MSG				0xEF56A55A

#define LPRINTF(format, ...)		printf(format, ##__VA_ARGS__)
#define LPERROR(format, ...)		LPRINTF("ERROR: " format, ##__VA_ARGS__)

static struct rpmsg_endpoint lept;	//
static int shutdown_req = 0;		//描述关机请求的变量

/************************** Function Prototypes ******************************/

/*-----------------------------------------------------------------------------*
 *  RPMSG endpoint callbacks
 *-----------------------------------------------------------------------------*/
static int rpmsg_endpoint_cb
	(struct rpmsg_endpoint *ept, 	//终点结构体指针
	 void *data, 					//数据指针
	 size_t len, 					//长度
	 uint32_t src, 					//此处无用
	 void *priv)					//此处无用
{
	(void)priv;		//不用
	(void)src;		//不用
	/* On reception of a shutdown we signal the application to terminate */

	//当收到关机信息时
	if ((*(unsigned int *)data) == SHUTDOWN_MSG) {
		LPRINTF("shutdown message is received.\r\n");
		shutdown_req = 1;	//由主核传来关机请求
		return RPMSG_SUCCESS;	//RPMSG_SUCCESS == 0
	}

	/* Send data back to master */
	//将数据传回给主机，采用函数 rpmsg_send()。位于rpmgs.h中。
	if (rpmsg_send(ept, data, len) < 0)
		LPERROR("rpmsg_send failed\r\n");

	return RPMSG_SUCCESS;
}

//发生未预料的远程终点损毁时，执行该函数。
static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
	(void)ept;
	LPRINTF("unexpected Remote endpoint destroy\r\n");
	shutdown_req = 1;	//将shutdown_req置为1，表示远程节点已经被关闭。
}

/*-----------------------------------------------------------------------------*
 *  Application
 *-----------------------------------------------------------------------------*/
int app
(struct rpmsg_device *rdev, 	//RPMsg设备结构体指针
 void *priv)					//私有数据
{
	int ret;	//返回值

	/* Initialize RPMSG framework */
	//初始化RPMsg框架，主要是创建RPMsg终点结构体变量
	OPENAMP_SLAVE_INFO("step7 : user to init endpoint ");

	//1. 创建RPMsg端点结构体变量
	ret = rpmsg_create_ept(
		&lept, 	//endpoint，端点
		rdev, 	//rpmsg_device，RPMsg设备
		RPMSG_SERVICE_NAME, //服务名称
		0, //源地址
		RPMSG_ADDR_ANY, //目的地址
		rpmsg_endpoint_cb, //需要被注册的端点回调函数
		rpmsg_service_unbind);	//需要被注册的解绑(unbind)回调函数
	if (ret) {
		LPERROR("Failed to create endpoint. %d \r\n", ret);
		return -1;
	}

	OPENAMP_SLAVE_INFO("step8 : user to wait vq code ");
	//2. 程序正式运行
	while (1)	//死循环
	{
		platform_poll(priv);	//从硬件平台中获取私有数据
        /* we got a shutdown request, exit */
        if (shutdown_req)	//如果出现了关机请求
		{
			OPENAMP_SLAVE_INFO("step9 : start to exit ");
			break;		//退出死循环
		}
	}
	//3. 死循环外，销毁端点
	rpmsg_destroy_ept(&lept);	//销毁端点

	return 0;
}



/*-----------------------------------------------------------------------------*
 *  Application entry point
 *-----------------------------------------------------------------------------*/
int rpmsg_echo(int argc, char *argv[])	//程序名，估计是自定的：一个用于echo的程序
{
	void *platform;
	struct rpmsg_device *rpdev;	//RPMsg设备
	int ret;	//返回值

	LPRINTF("openamp lib version: %s (", openamp_version());	//OpenAMP版本号
	LPRINTF("Major: %d, ", openamp_version_major());
	LPRINTF("Minor: %d, ", openamp_version_minor());
	LPRINTF("Patch: %d)\r\n", openamp_version_patch());

	LPRINTF("libmetal lib version: %s (", metal_ver());	//libmetal版本号
	LPRINTF("Major: %d, ", metal_ver_major());
	LPRINTF("Minor: %d, ", metal_ver_minor());
	LPRINTF("Patch: %d)\r\n", metal_ver_patch());

	/* Initialize platform */
	//初始化平台(platform)
	OPENAMP_SLAVE_INFO("start application");
	ret = platform_init(argc, argv, &platform);	//初始化函数
	if (ret) {	//如果初始化失败
		LPERROR("Failed to initialize platform.\r\n");
		ret = -1;
	} else {	//如果初始化成功
		#ifdef CONFIG_DEBUG_CODE
		LPERROR("CONFIG_TARGET_CPU_ID is %x \r\n",CONFIG_TARGET_CPU_ID);
		FPsciCpuMaskOn(1<<CONFIG_TARGET_CPU_ID,(uintptr_t)OPENAMP_MASTER_ADDRESS) ;
		#endif
		OPENAMP_SLAVE_INFO("step3: start to init virtio device ");

		rpdev = platform_create_rpmsg_vdev(platform, 0, VIRTIO_DEV_SLAVE, NULL, NULL);	//创建RPMsg的VirtIO设备

		if (!rpdev) {
			LPERROR("Failed to create rpmsg virtio device.\r\n");
			ret = -1;
		} else {

			app(rpdev, platform);
			platform_release_rpmsg_vdev(rpdev, platform);	//释放掉RPMsg的VirtIO设备
			ret = 0;
		}
	}

	OPENAMP_SLAVE_INFO("Stopping application...\r\n");
	platform_cleanup(platform);	//清理平台

    return ret;
}
 

void RpmsgEchoTask( void * args )	//FreeRTOS任务：RPMsg设备创建、初始化与销毁
{
    rpmsg_echo(0, NULL);	//调用这个入口函数，实现RPMsg设备的整个生命周期
    vTaskDelete(NULL);
}

int rpmsg_echo_task(int argc, char *argv[])	//放在嵌入式main()里面的函数，用于Task创建
{
    BaseType_t ret; 

    ret = xTaskCreate((TaskFunction_t )RpmsgEchoTask, /* 任务入口函数 */
                        (const char* )"RpmsgEchoTask",/* 任务名字 */
                        (uint16_t )(4096*2), /* 任务栈大小 */
                        (void* )NULL,/* 任务入口函数参数 */
                        (UBaseType_t )4, /* 任务的优先级 */
                        NULL); /* 任务控制块指针 */
    
    if(ret != pdPASS)
    {
        LPERROR("Failed to create a rpmsg_echo task ");
        return -1;
    }

    return 0;
}


SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), rpmsg_echo_task, rpmsg_echo_task, rpmsg_echo_task);	//该函数被Letter Shell所支持