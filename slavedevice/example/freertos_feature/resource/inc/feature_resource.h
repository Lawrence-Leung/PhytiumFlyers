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
 * FilePath: feature_resource.h
 * Date: 2022-06-17 10:42:40
 * LastEditTime: 2022-06-17 10:42:40
 * Description:  This file is for task function define
 *
 * Modify History:
 *  Ver   Who  Date   Changes
 * ----- ------  -------- --------------------------------------
 * 1.0 wangxiaodong 2022/08/09  first commit
 */

#ifndef FEATURE_RESOURCE_H
#define FEATURE_RESOURCE_H

#ifdef __cplusplus
extern "C"
{
#endif

/* mutex task */
void CreateResourceTasks(void);
void DeleteResourceTasks(void);

/* gatekeeper task */
void CreateGatekeeperTasks(void);
void DeleteGatekeeperTasks(void);

#ifdef __cplusplus
}
#endif

#endif // !