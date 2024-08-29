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
 * FilePath: fl3cache.c
 * Date: 2022-03-08 21:56:42
 * LastEditTime: 2022-03-15 11:10:40
 * Description:  This file is for l3 cache-related operations
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2022/03/15       first release
 */


#include "fl3cache.h"
#include "sdkconfig.h"



/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
/* FLUSH L3 CASHE */
#ifdef CONFIG_USE_L3CACHE
    #define HNF_BASE (unsigned long)(0x3A200000)
    #define HNF_COUNT 0x8
    #define HNF_PSTATE_REQ (HNF_BASE + 0x10)
    #define HNF_PSTATE_STAT (HNF_BASE + 0x18)
    #define HNF_PSTATE_OFF 0x0
    #define HNF_PSTATE_SFONLY 0x1
    #define HNF_PSTATE_HALF 0x2
    #define HNF_PSTATE_FULL 0x3
    #define HNF_STRIDE 0x10000
#endif

/************************** Function Prototypes ******************************/

void FCacheL3CacheDisable(void)
{
#ifdef CONFIG_USE_L3CACHE
    int i, pstate;


    for (i = 0; i < 8; i++)
    {
        FtOut32(0x3A200010 + i * 0x10000, 1);
    }

    for (i = 0; i < 8; i++)
    {
        do
        {
            pstate = FtIn32(0x3A200018 + i * 0x10000);
        }
        while ((pstate & 0xf) != (0x1 << 2));
    }
#endif
}


void FCacheL3CacheFlush(void)
{
#ifdef CONFIG_USE_L3CACHE
    int i, pstate;

    for (i = 0; i < HNF_COUNT; i++)
    {
        FtOut64(HNF_PSTATE_REQ + i * HNF_STRIDE, HNF_PSTATE_SFONLY);
    }
    for (i = 0; i < HNF_COUNT; i++)
    {
        do
        {
            pstate = FtIn64(HNF_PSTATE_STAT + i * HNF_STRIDE);
        }
        while ((pstate & 0xf) != (HNF_PSTATE_SFONLY << 2));
    }

    for (i = 0; i < HNF_COUNT; i++)
    {
        FtOut64(HNF_PSTATE_REQ + i * HNF_STRIDE, HNF_PSTATE_FULL);
    }

#endif
    return ;
}


void FCacheL3CacheInvalidate(void)
{
#ifdef CONFIG_USE_L3CACHE
    int i, pstate;

    for (i = 0; i < HNF_COUNT; i++)
    {
        FtOut64(HNF_PSTATE_REQ + i * HNF_STRIDE, HNF_PSTATE_SFONLY);
    }

    for (i = 0; i < HNF_COUNT; i++)
    {
        do
        {
            pstate = FtIn64(HNF_PSTATE_STAT + i * HNF_STRIDE);
        }
        while ((pstate & 0xf) != (HNF_PSTATE_SFONLY << 2));
    }

    for (i = 0; i < HNF_COUNT; i++)
    {
        FtOut64(HNF_PSTATE_REQ + i * HNF_STRIDE, HNF_PSTATE_FULL);
    }
#endif
    return ;
}


void FCacheL3CacheEnable(void)
{
    return ;
}