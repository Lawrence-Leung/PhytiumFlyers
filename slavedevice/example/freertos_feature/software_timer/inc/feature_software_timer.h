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
 * FilePath: feature_software_timer.h
 * Date: 2022-06-17 10:42:40
 * LastEditTime: 2022-06-17 10:42:40
 * Description:  This file is for task function define
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2022/08/09  first commit
 */


#ifndef FEATURE_SOFTWARE_TIMER_H
#define FEATURE_SOFTWARE_TIMER_H

#ifdef __cplusplus
extern "C"
{
#endif

/* software timer create and start */
void CreateTimerTasks(void);
void DeleteTimerTasks(void);

void CreateTimerResetTasks(void);
void DeleteTimerResetTasks(void);

#ifdef __cplusplus
}
#endif

#endif // !