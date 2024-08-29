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
 * FilePath: ffsystem.c
 * Date: 2022-07-21 13:21:43
 * LastEditTime: 2022-07-21 13:21:44
 * Description:  This files is for os releated function implmentation
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/12/3   init commit
 */

/*------------------------------------------------------------------------*/
/* Sample Code of OS Dependent Functions for FatFs                        */
/* (C)ChaN, 2018                                                          */
/*------------------------------------------------------------------------*/


#include <string.h>
#include <stdlib.h>
#include "fkernel.h"
#include "ff.h"
#include "diskio.h"
#include "sdkconfig.h"
#include "fgeneric_timer.h"
#ifdef CONFIG_FATFS_ALLOC_PREFER_MEMP
#include "fmemory_pool.h"
#endif
#include "fparameters.h"
#ifdef CONFIG_FATFS_ALLOC_PREFER_MEMP
#define FATFS_MEMP_TOTAL_SIZE  (CONFIG_FATFS_MEMP_SIZE * SZ_1M)
static FMemp ff_memp;
static u8 memp_buf[FATFS_MEMP_TOTAL_SIZE] __attribute((aligned(64)));

void ff_mempinit(void)
{
    /* create memory pool to support dynamic memory allocation */
	assert (FT_SUCCESS == FMempInit(&ff_memp, &memp_buf[0], &memp_buf[0] + FATFS_MEMP_TOTAL_SIZE));    
}
#endif

/*------------------------------------------------------------------------*/
/* Allocate a memory block                                                */
/*------------------------------------------------------------------------*/

void* ff_memalloc (	/* Returns pointer to the allocated memory block (null if not enough core) */
	UINT msize		/* Number of bytes to allocate */
)
{
    void *result = NULL;

#ifdef CONFIG_FATFS_ALLOC_PREFER_MEMP
	result = FMempMalloc(&ff_memp, msize);
#else
	result = malloc(msize);	/* Allocate a new memory block with POSIX API */
#endif
    if (NULL != result){
        memset(result, 0, msize);
    }

    return result;
}

void* ff_memalign(	/* Returns pointer to the allocated memory block (null if not enough core) */
	UINT msize,		/* Number of bytes to allocate */
    UINT align      /* Alignment */
)
{
    void *result = NULL;

#ifdef CONFIG_FATFS_ALLOC_PREFER_MEMP
	result = FMempMallocAlign(&ff_memp, msize, align);
#else
    #error "allocate aligned memory is not supported !!!"
	result = malloc(msize);	/* Allocate a new memory block with POSIX API */
#endif

    if (NULL != result){
        memset(result, 0, msize);
    }

    return result;
}

/*------------------------------------------------------------------------*/
/* Free a memory block                                                    */
/*------------------------------------------------------------------------*/

void ff_memfree (
	void* mblock	/* Pointer to the memory block to free (nothing to do if null) */
)
{
#ifdef CONFIG_FATFS_ALLOC_PREFER_MEMP
	FMempFree(&ff_memp, mblock);
#else
	free(mblock);	/* Free the memory block with POSIX API */
#endif
}


#if FF_FS_REENTRANT	/* Mutal exclusion */
#error "No re-entrant implementation for NOOS"
#endif

DWORD get_fattime(void)
{
    return    ((DWORD)(2022) << 25)
            | ((DWORD)(11) << 21)
            | ((DWORD)2 << 16)
            | (WORD)(9 << 11)
            | (WORD)(3 << 5)
            | (WORD)(31 >> 1);
}

void ff_systimer_start(void)
{
    GenericTimerStart(GENERIC_TIMER_ID0);
}

QWORD ff_systimer_get_tick(void)
{
    return (QWORD)GenericTimerRead(GENERIC_TIMER_ID0);
}

void ff_systimer_tick_to_time(QWORD ticks, DWORD *sec, DWORD *msec)
{
    if (sec)
    {
        *sec = (DWORD)(ticks / (u32)GenericTimerFrequecy());
    }

    if (msec)
    {
        *msec = (DWORD)(ticks % (DWORD) GenericTimerFrequecy() / 
                (((DWORD)GenericTimerFrequecy() * 1 + 999) / 1000));
    }
}

void ff_systimer_stop(void)
{
    GenericTimerStop(GENERIC_TIMER_ID0);
}