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
 * FilePath: fcan_os.h
 * Date: 2022-09-16 11:40:19
 * LastEditTime: 2022-09-21 16:59:58
 * Description:  This file is for providing function related definitions of can driver used in FreeRTOS.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2022/09/23  first commit
 * 1.1 wangxiaodong 2022/11/01  file name adaptation
 * 1.2 zhangyan     2023/2/7    improve functions
 */

#ifndef FCAN_OS_H
#define FCAN_OS_H

#include <FreeRTOS.h>
#include <semphr.h>
#include "ferror_code.h"
#include "fcan.h"
#include "ftypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* freertos can error */
#define FREERTOS_CAN_OK           FT_SUCCESS
#define FREERTOS_CAN_SEM_ERROR    FT_CODE_ERR(ErrModBsp, ErrBspCan, 10)

/* freertos can interrupt priority */
#define FREERTOS_CAN_IRQ_PRIORITY   IRQ_PRIORITY_VALUE_12

/* can control operation */
enum
{
    FREERTOS_CAN_CTRL_ENABLE = 0,   /* enable can */
    FREERTOS_CAN_CTRL_DISABLE = 1,  /* disable can */
    FREERTOS_CAN_CTRL_BAUDRATE_SET, /* set can baudrate */
    FREERTOS_CAN_CTRL_STATUS_GET,   /* get can status */
    FREERTOS_CAN_CTRL_ID_MASK_SET,  /* set can receive id mask */
    FREERTOS_CAN_CTRL_ID_MASK_ENABLE,  /* enable can receive id mask */
    FREERTOS_CAN_CTRL_INTR_SET,     /* set can interrupt handler */
    FREERTOS_CAN_CTRL_INTR_ENABLE,   /* enable can interrupt */
    FREERTOS_CAN_CTRL_FD_ENABLE,    /* set can fd enable */
    FREERTOS_CAN_CTRL_MODE_SET,   /* set can transmit mode */
    FREERTOS_CAN_CTRL_NUM
};

typedef struct
{
    FCanCtrl can_ctrl; /* can object */
    xSemaphoreHandle can_semaphore; /* can semaphore for resource sharing */
} FFreeRTOSCan;

/* init freertos can instance */
FFreeRTOSCan *FFreeRTOSCanInit(u32 instance_id);

/* deinit freertos can instance */
FError FFreeRTOSCanDeinit(FFreeRTOSCan *os_can_p);

/* can config */
FError FFreeRTOSCanControl(FFreeRTOSCan *os_can_p, int cmd, void *arg);

/* can send frame */
FError FFreeRTOSCanSend(FFreeRTOSCan *os_can_p, FCanFrame *frame_p);

/* can receive frame */
FError FFreeRTOSCanRecv(FFreeRTOSCan *os_can_p, FCanFrame *frame_p);

#ifdef __cplusplus
}
#endif

#endif // !
