/*
    rpmsg-echo.h
    OpenAMP 通信协议源文件
    by Lawrence Leung
    2024 飞腾风驰队
*/

#ifndef __RPMSG_ECHO_H
#define __RPMSG_ECHO_H

#include <stdio.h>
#include <openamp/open_amp.h>
#include <metal/alloc.h>
#include "platform_info.h"
#include <metal/sleep.h>
#include "rsc_table.h"
#include "fcache.h"
#include "fdebug.h"
#include "FreeRTOS.h"
#include "task.h"

/* 宏定义 */
#define OPENAMP_MAIN_DEBUG_TAG "OPENAMP_SLAVE_MAIN"
#define OPENAMP_MAIN_DEBUG_I(format, ...) FT_DEBUG_PRINT_I(OPENAMP_MAIN_DEBUG_TAG, format, ##__VA_ARGS__)
#define OPENAMP_MAIN_DEBUG_W(format, ...) FT_DEBUG_PRINT_W(OPENAMP_MAIN_DEBUG_TAG, format, ##__VA_ARGS__)
#define OPENAMP_MAIN_DEBUG_E(format, ...) FT_DEBUG_PRINT_E(OPENAMP_MAIN_DEBUG_TAG, format, ##__VA_ARGS__)

#define     SHUTDOWN_MSG                0xEF56A55A
#define     ECHO_DEV_SLAVE_DEBUG_TAG "    SLAVE_01"
#define     ECHO_DEV_SLAVE_DEBUG_I(format, ...) FT_DEBUG_PRINT_I( ECHO_DEV_SLAVE_DEBUG_TAG, format, ##__VA_ARGS__)
#define     ECHO_DEV_SLAVE_DEBUG_W(format, ...) FT_DEBUG_PRINT_W( ECHO_DEV_SLAVE_DEBUG_TAG, format, ##__VA_ARGS__)
#define     ECHO_DEV_SLAVE_DEBUG_E(format, ...) FT_DEBUG_PRINT_E( ECHO_DEV_SLAVE_DEBUG_TAG, format, ##__VA_ARGS__)

int FRpmsgCommunication(void);  //struct rpmsg_device *rdev, void *priv);
int FOpenampInit(void);
BaseType_t OpenAmpTask(void);
void OpenAmpVersionInfo (void);

#endif