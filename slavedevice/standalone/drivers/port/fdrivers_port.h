/*
 * Copyright : (C) 2023 Phytium Information Technology, Inc.
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
 * FilePath: fdrivers_port.h
 * Created Date: 2023-10-16 17:02:35
 * Last Modified: 2023-11-21 17:03:55
 * Description:  This file is for drive layer code decoupling
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 *  1.0     huanghe    2023/10/17    first release
 */

#ifndef FDRIVERS_PORT_H
#define FDRIVERS_PORT_H

#include "ftypes.h"
#include "faarch.h"

#include "fkernel.h"
#include "fdebug.h"
#include "sdkconfig.h"

/***************************** Include Files *********************************/
#ifdef __cplusplus
extern "C"
{
#endif

/* cache */
void FDriverDCacheRangeFlush(uintptr_t adr,size_t len);

void FDriverDCacheRangeInvalidate(uintptr_t adr,size_t len);

void FDriverICacheRangeInvalidate(void);


/* memory barrier */

#define FDRIVER_DSB() DSB()

#define FDRIVER_DMB() DMB()

#define FDRIVER_ISB() ISB()

/* time delay */

void FDriverUdelay(u32 usec);

void FDriverMdelay(u32 msec);

void FDriverSdelay(u32 sec);

#ifndef FT_DEBUG_PRINT_I
#define FT_DEBUG_PRINT_I(TAG, format, ...)
#endif

#ifndef FT_DEBUG_PRINT_I
#define FT_DEBUG_PRINT_E(TAG, format, ...)
#endif

#ifndef FT_DEBUG_PRINT_I
#define FT_DEBUG_PRINT_D(TAG, format, ...)
#endif

#ifndef FT_DEBUG_PRINT_W
#define FT_DEBUG_PRINT_W(TAG, format, ...)
#endif

#ifndef FT_DEBUG_PRINT_V
#define FT_DEBUG_PRINT_V(TAG, format, ...)
#endif

#ifdef __cplusplus
}
#endif


#endif
