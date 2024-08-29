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
 * FilePath: fwdt_os.h
 * Date: 2022-02-24 13:42:19
 * LastEditTime: 2022-03-21 16:59:58
 * Description:  This file is for providing function related definitions of wdt driver used in FreeRTOS.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2022/08/09  first commit
 */

#ifndef FWDT_OS_H
#define FWDT_OS_H

#include <FreeRTOS.h>
#include <semphr.h>
#include "ferror_code.h"
#include "fwdt.h"
#include "ftypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* freertos wdt error */
#define FREERTOS_WDT_SEM_ERROR    FT_CODE_ERR(ErrModBsp, ErrBspWdt, 10)

/* freertos wdt interrupt priority */
#define FREERTOS_WDT_IRQ_PRIORITY IRQ_PRIORITY_VALUE_12

#define FREERTOS_WDT_CTRL_GET_TIMEOUT    (1) /* get timeout(in seconds) */
#define FREERTOS_WDT_CTRL_SET_TIMEOUT    (2) /* set timeout(in seconds) */
#define FREERTOS_WDT_CTRL_GET_TIMELEFT   (3) /* get the left time before reboot(in seconds) */
#define FREERTOS_WDT_CTRL_KEEPALIVE      (4) /* refresh watchdog */
#define FREERTOS_WDT_CTRL_START          (5) /* start watchdog */
#define FREERTOS_WDT_CTRL_STOP           (6) /* stop watchdog */

typedef struct
{
    FWdtCtrl wdt_ctrl; /* wdt object */
    u32 timeout_value; /* timeout period of wdt */
    xSemaphoreHandle wdt_semaphore; /*!< wdt semaphore for resource sharing */
} FFreeRTOSWdt;

/* init freertos wdt instance */
FFreeRTOSWdt *FFreeRTOSWdtInit(u32 instance_id);

/* deinit freertos wdt instance */
FError FFreeRTOSWdtDeinit(FFreeRTOSWdt *os_wdt_p);

/* control freertos wdt instance */
FError FFreeRTOSWdtControl(FFreeRTOSWdt *os_wdt_p, int cmd, void *args);

#ifdef __cplusplus
}
#endif

#endif // !
