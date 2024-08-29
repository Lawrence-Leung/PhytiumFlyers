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
 * FilePath: fl3cache.h
 * Date: 2022-03-08 21:56:42
 * LastEditTime: 2022-03-15 11:14:45
 * Description:  This file is for l3 cache-related operations
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2021/10/21       first release
 */


#ifndef FL3CACHE_H
#define FL3CACHE_H

#include "fparameters.h"
#include "fio.h"

#ifdef __cplusplus
extern "C"
{
#endif

/************************** Function Prototypes ******************************/


void FCacheL3CacheEnable(void);
void FCacheL3CacheDisable(void);
void FCacheL3CacheInvalidate(void);
void FCacheL3CacheFlush(void);

#ifdef __cplusplus
}
#endif

#endif