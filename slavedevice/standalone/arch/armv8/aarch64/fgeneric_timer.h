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
 * FilePath: fgeneric_timer.h
 * Date: 2022-02-10 14:53:41
 * LastEditTime: 2022-02-17 17:33:07
 * Description:  This file provides the common helper routines for the generic timer API's 
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  huanghe	2021/11/13		initialization
 *  1.1  zhugengyu 2022/06/05     add tick api
 *  1.2  wangxiaodong 2023/05/29  modify api
 */


#ifndef FGENERIC_TIMER_H
#define FGENERIC_TIMER_H

#include "ftypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Set generic timer CompareValue */
void GenericTimerSetTimerCompareValue(u32 id, u64 timeout);

/* Set generic timer TimerValue */
void GenericTimerSetTimerValue(u32 id, u32 timeout);

/* Unmask generic timer interrupt */
void GenericTimerInterruptEnable(u32 id);

/* Mask generic timer interrupt */
void GenericTimerInterruptDisable(u32 id);

/* Enable generic timer */
void GenericTimerStart(u32 id);

/* Get generic timer physical count value */
u64 GenericTimerRead(u32 id);

/* Get generic timer frequency of the system counter */
u64 GenericTimerFrequecy(void);

/* Disable generic timer */
void GenericTimerStop(u32 id);


#ifdef __cplusplus
}
#endif

#endif // !