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
 * FilePath: ftimer_tacho_os.h
 * Date: 2022-08-23 17:20:58
 * LastEditTime: 2022-08-23 17:20:58
 * Description:  This file is for providing function related definitions of timer tacho driver used in FreeRTOS.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 liushengming 2022/08/24  init
 */

#ifndef FTIMER_TACHO_OS_H
#define FTIMER_TACHO_OS_H
/***************************** Include Files *********************************/
#include <FreeRTOS.h>
#include <semphr.h>
#include <event_groups.h>

#include "fparameters.h"
#include "ftimer_tacho.h"
#include "ftimer_tacho_hw.h"
#include "ftypes.h"
/************************** Constant Definitions *****************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*Error code from standalone tacho driver*/
#define FREERTOS_TIMER_TACHO_SUCCESS                    FTIMER_TACHO_SUCCESS
#define FREERTOS_TIMER_TACHO_INVAILD_PARAM_ERROR        FTIMER_TACHO_ERR_INVAL_PARM
#define FREERTOS_TIMER_TACHO_NOT_READY_ERROR            FTIMER_TACHO_ERR_NOT_READY
#define FREERTOS_TIMER_TACHO_TIMEOUT_ERROR              FTIMER_TACHO_ERR_INIT_FAILED
#define FREERTOS_TIMER_TACHO_NOT_SUPPORT_ERROR          FTIMER_TACHO_ERR_NOT_SUPPORT
#define FREERTOS_TIMER_TACHO_IS_READ_ERROR              FTIMER_TACHO_ERR_IS_READ
#define FREERTOS_TIMER_TACHO_ABORT_ERROR                FTIMER_TACHO_ERR_ABORT
#define FREERTOS_TIMER_TACHO_FAILED_ERROR               FTIMER_TACHO_ERR_FAILED

/*Error code depend on OS standard*/
#define FREERTOS_TIMER_TACHO_TASK_ERROR                 FT_CODE_ERR(ErrModPort, ErrBspTimer, 0x1)
#define FREERTOS_TIMER_TACHO_MESG_ERROR                 FT_CODE_ERR(ErrModPort, ErrBspTimer, 0x2)
#define FREERTOS_TIMER_TACHO_TIME_ERROR                 FT_CODE_ERR(ErrModPort, ErrBspTimer, 0x3)
#define FREERTOS_TIMER_TACHO_MEMY_ERROR                 FT_CODE_ERR(ErrModPort, ErrBspTimer, 0x4)
#define FREERTOS_TIMER_TACHO_SEMA_ERROR                 FT_CODE_ERR(ErrModPort, ErrBspTimer, 0x5)

/************************** Variable Definitions *****************************/
typedef struct
{
    FTimerTachoCtrl ctrl;
    SemaphoreHandle_t locker;
} FFreeRTOSTimerTacho;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
/*timer*/
FFreeRTOSTimerTacho *FFreeRTOSTimerInit(u32 id, boolean timer_mode, u64 times);
FError FFreeRTOSTimerStart(FFreeRTOSTimerTacho *os_timer_p);
FError FFreeRTOSTimerStop(FFreeRTOSTimerTacho *os_timer_p);
void FFreeRTOSTimerDeinit(FFreeRTOSTimerTacho *os_timer_p);
void FFreeRTOSTimerDebug(FFreeRTOSTimerTacho *os_timer_p);
/*tacho*/
FFreeRTOSTimerTacho *FFreeRTOSTachoInit(u32 id, boolean tacho_mode);
FError FFreeRTOSTachoGetRPM(FFreeRTOSTimerTacho *os_timer_p, u32 *rpm);
u32 FFreeRTOSTachoGetCNT(FFreeRTOSTimerTacho *os_timer_p);
void FFreeRTOSTachoDeinit(FFreeRTOSTimerTacho *os_timer_p);

/*****************************************************************************/
#ifdef __cplusplus
}
#endif

#endif