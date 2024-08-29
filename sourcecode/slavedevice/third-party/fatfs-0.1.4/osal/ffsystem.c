/*------------------------------------------------------------------------*/
/* Sample Code of OS Dependent Functions for FatFs                        */
/* (C)ChaN, 2018                                                          */
/*------------------------------------------------------------------------*/

#include <string.h>
#include <stdlib.h>
#include "ff.h"
#include "fassert.h"
#include "sdkconfig.h"
#ifdef CONFIG_FATFS_ALLOC_PREFER_MEMP
#include "fmemory_pool.h"
#endif

#ifdef CONFIG_FATFS_ALLOC_PREFER_MEMP
#define FATFS_MEMP_TOTAL_SIZE  (CONFIG_FATFS_MEMP_SIZE * SZ_1M)
static FMemp ff_memp;
static u8 memp_buf[FATFS_MEMP_TOTAL_SIZE] __attribute((aligned(64)));

void ff_mempinit(void)
{
    /* create memory pool to support dynamic memory allocation */
	FASSERT (FT_SUCCESS == FMempInit(&ff_memp, &memp_buf[0], &memp_buf[0] + FATFS_MEMP_TOTAL_SIZE));    
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

/*------------------------------------------------------------------------*/
/* Create a Synchronization Object                                        */
/*------------------------------------------------------------------------*/
/* This function is called in f_mount() function to create a new
/  synchronization object for the volume, such as semaphore and mutex.
/  When a 0 is returned, the f_mount() function fails with FR_INT_ERR.
*/

int ff_cre_syncobj (	/* 1:Function succeeded, 0:Could not create the sync object */
	BYTE vol,			/* Corresponding volume (logical drive number) */
	FF_SYNC_t* sobj		/* Pointer to return the created sync object */
)
{
	/* FreeRTOS */
	*sobj = xSemaphoreCreateMutex();
	return (int)(*sobj != NULL);
}


/*------------------------------------------------------------------------*/
/* Delete a Synchronization Object                                        */
/*------------------------------------------------------------------------*/
/* This function is called in f_mount() function to delete a synchronization
/  object that created with ff_cre_syncobj() function. When a 0 is returned,
/  the f_mount() function fails with FR_INT_ERR.
*/

int ff_del_syncobj (	/* 1:Function succeeded, 0:Could not delete due to an error */
	FF_SYNC_t sobj		/* Sync object tied to the logical drive to be deleted */
)
{
	/* FreeRTOS */
  	vSemaphoreDelete(sobj);
	return 1;
}


/*------------------------------------------------------------------------*/
/* Request Grant to Access the Volume                                     */
/*------------------------------------------------------------------------*/
/* This function is called on entering file functions to lock the volume.
/  When a 0 is returned, the file function fails with FR_TIMEOUT.
*/

int ff_req_grant (	/* 1:Got a grant to access the volume, 0:Could not get a grant */
	FF_SYNC_t sobj	/* Sync object to wait */
)
{
	/* FreeRTOS */
	return (int)(xSemaphoreTake(sobj, FF_FS_TIMEOUT) == pdTRUE);
}


/*------------------------------------------------------------------------*/
/* Release Grant to Access the Volume                                     */
/*------------------------------------------------------------------------*/
/* This function is called on leaving file functions to unlock the volume.
*/

void ff_rel_grant (
	FF_SYNC_t sobj	/* Sync object to be signaled */
)
{
	/* FreeRTOS */
	xSemaphoreGive(sobj);
}

#endif

DWORD get_fattime(void) /* TODO: impl file time with RTC */
{
    return    ((DWORD)(2022 - 1980) << 25) /* bit31:25, Year origin from the 1980 (0..127, e.g. 37 for 2017) */
            | ((DWORD)(11) << 21) /* bit24:21, Month (1..12) */
            | ((DWORD)2 << 16) /* bit20:16, Day of the month (1..31) */
            | (WORD)(9 << 11) /* bit15:11, Hour (0..23) */
            | (WORD)(3 << 5) /* bit10:5, Minute (0..59) */
            | (WORD)(15 >> 1); /* bit4:0, Second / 2 (0..29, e.g. 25 for 50)  */
}


void ff_systimer_start(void)
{
	/* no need to start systimer */
}

QWORD ff_systimer_get_tick(void)
{
	return (QWORD)xTaskGetTickCount();
}

void ff_systimer_tick_to_time(QWORD ticks, DWORD *sec, DWORD *msec)
{
	
    if (sec)
    {
        *sec = (DWORD)(ticks / (u32)configTICK_RATE_HZ);
    }

    if (msec)
    {
        *msec = (DWORD)(ticks % (DWORD)configTICK_RATE_HZ / 
                (((DWORD)configTICK_RATE_HZ * 1 + 999) / 1000));
    }

}

void ff_systimer_stop(void)
{
	/* no need to stop systimer */
}