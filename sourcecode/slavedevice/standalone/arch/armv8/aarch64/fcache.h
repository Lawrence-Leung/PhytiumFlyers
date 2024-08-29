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
 * FilePath: fcache.h
 * Date: 2022-02-10 14:53:41
 * LastEditTime: 2022-02-17 17:32:40
 * Description:  This file is for the arm cache functionality.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe     2021/7/3     first release
 */


#ifndef FCACHE_H
#define FCACHE_H

/***************************** Include Files *********************************/
#include "ftypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

/************************** Function Prototypes ******************************/
void FCacheDCacheEnable(void);
void FCacheDCacheDisable(void);
void FCacheDCacheInvalidate(void);
void FCacheDCacheInvalidateLine(intptr adr);
void FCacheDCacheInvalidateRange(intptr adr, intptr len);
void FCacheDCacheFlush(void);
void FCacheDCacheFlushLine(intptr adr);
void FCacheDCacheFlushRange(intptr adr, intptr len);

void FCacheICacheEnable(void);
void FCacheICacheDisable(void);
void FCacheICacheInvalidate(void);

#ifdef __cplusplus
}
#endif

#endif