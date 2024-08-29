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
 * FilePath: fmemory_pool.h
 * Date: 2021-11-25 15:18:57
 * LastEditTime: 2022-02-17 18:02:16
 * Description:  This files is for memory pool API definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2021/12/2    init
 * 1.1   zhugengyu  2022/2/28    support memory tag for trace
 */

#ifndef FMEMORY_POOL_H
#define FMEMORY_POOL_H

#include <stdio.h>
#include "FreeRTOS.h"
#include "event_groups.h"
#include "semphr.h"

#include "ferror_code.h"

#include "tlsf.h"
#include "fslink_list.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FMEMP_SUCCESS           FT_SUCCESS
#define FMEMP_ERR_INVALID_BUF   FT_MAKE_ERRCODE(ErrorModGeneral, ErrCommMemp, 0)
#define FMEMP_ERR_INIT_TLFS     FT_MAKE_ERRCODE(ErrorModGeneral, ErrCommMemp, 1)
#define FMEMP_ERR_BAD_MALLOC    FT_MAKE_ERRCODE(ErrorModGeneral, ErrCommMemp, 2)
#define FMEMP_ERR_ALREADY_INIT  FT_MAKE_ERRCODE(ErrorModGeneral, ErrCommMemp, 3)

#ifndef FMEMP_TAG_DEBUG
/* #define FMEMP_TAG_DEBUG */
#endif

#ifdef FMEMP_TAG_DEBUG
typedef struct _FMempTag FMempTag;

typedef enum
{
    FMEMP_TAG_ALLOCATED = 0,
    FMEMP_TAG_FREEED,
} FMempTagStatus; /* 内存块TAG状态 */

typedef struct _FMempTag
{
    uintptr mem_addr;
    uintptr id;
    fsize_t mem_size;
#define FMEMP_TAG_MSG_LEN    125
    char msg[FMEMP_TAG_MSG_LEN];
    FMempTagStatus status;
    FMempTag *next;
} FMempTag; /* 内存块TAG数据 */

#endif

typedef struct
{
    pool_t pool_addr;
    FSListNode list;
} FMempPoolList; /* 内存池控制数据 */

typedef struct
{
    FMempPoolList *pools_list;   /* 内存池链表 */
    tlsf_t         tlsf_ptr;     /* tlsf内存池 */
    u32            is_ready;     /* 内存池初始化完成标志 */
    SemaphoreHandle_t locker;    /* 内存池访问互斥锁 */
#ifdef FMEMP_TAG_DEBUG
    FMempTag      *tags_list;    /* 分配内存tag链表 */
#endif
} FMemp; /* 内存池控制数据 */

/* 初始化内存池, 分配内存池的内存空间 */
FError FMempInit(FMemp *memp, void *begin_addr, void *end_addr);

/* 释放所有分配的内存，删除内存池 */
void FMempDeinit(FMemp *memp);

/* 从内存池申请一段空间 */
void *FMempMalloc(FMemp *memp, fsize_t size);

/* 从内存池申请一段数组空间并清零 */
void *FMempCalloc(FMemp *memp, fsize_t count, fsize_t size);

/* 按指定对齐方式申请一段空间 */
void *FMempMallocAlign(FMemp *memp, fsize_t size, fsize_t align);

/* 回收原来申请的空间并重新分配 */
void *FMempRealloc(FMemp *memp, void *ptr, fsize_t size);

/* 释放一段从内存池申请的空间 */
void FMempFree(FMemp *memp, void *ptr);

/* 跟踪当前内存池的使用情况 */
void FMemProbe(FMemp *memp, u32 *total, u32 *used, u32 *max_used);

/* 打印当前分配的内存块信息 */
void FMemListAll(FMemp *memp);

/* 支持带TAG的内存分配，用于跟踪动态内存使用 */
#ifdef FMEMP_TAG_DEBUG
void *FMempMallocTag(FMemp *memp, fsize_t nbytes, const char *file, unsigned long line, const char *msg);
void *FMempMallocAlignTag(FMemp *memp, fsize_t size, fsize_t align, const char *file, unsigned long line, const char *msg);
void *FMempCallocTag(FMemp *memp, fsize_t count, fsize_t size, const char *file, unsigned long line, const char *msg);
void *FMempReallocTag(FMemp *memp, void *ptr, fsize_t size, const char *file, unsigned long line, const char *msg);
void FMempFreeTag(FMemp *memp, void *ptr);

#endif

#ifdef FMEMP_TAG_DEBUG
#define FMEMP_MALLOC(memp, size) FMempMallocTag((memp), (size), __FILE__, __LINE__, "")
#define FMEMP_MALLOC_ALIGN(memp, size, align)  FMempMallocAlignTag((memp), (size), (align), __FILE__, __LINE__, "")
#define FMEMP_CALLOC(memp, count, size) FMempCallocTag((memp), (count), (size),  __FILE__, __LINE__, "")
#define FMEMP_REALLOC(memp, ptr, nbytes) FMempReallocTag((memp), (ptr), (nbytes),  __FILE__, __LINE__, "")
#define FMEMP_FREE(memp, ptr)   FMempFreeTag((memp), (ptr))
#else
#define FMEMP_MALLOC(memp, size) FMempMalloc((memp), (size))
#define FMEMP_MALLOC_ALIGN(memp, size, align) FMempMallocAlign((memp), (size), (align))
#define FMEMP_CALLOC(memp, count, size) FMempCalloc((memp), (count), (size))
#define FMEMP_REALLOC(memp, ptr, nbytes) FMempRealloc((memp), (ptr), (nbytes))
#define FMEMP_FREE(memp, ptr)   FMempFree((memp), (ptr))

#endif

#ifdef __cplusplus
}
#endif

#endif // !
