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
 * FilePath: fmemory_pool.c
 * Date: 2021-11-25 15:19:14
 * LastEditTime: 2022-02-17 18:01:43
 * Description:  This files is for memory pool API implmentation
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2021/12/2    init
 * 1.1   zhugengyu  2022/2/28    support memory tag for trace
 */

#include <string.h>
#include <stdio.h>
#include "ftypes.h"
#include "fassert.h"
#include "fkernel.h"
#include "fdebug.h"

#include "tlsf.h"

#include "fmemory_pool.h"

#define FMEMP_DEBUG_TAG "FMEMP"
#define FMEMP_ERROR(format, ...)   FT_DEBUG_PRINT_E(FMEMP_DEBUG_TAG, format, ##__VA_ARGS__)
#define FMEMP_WARN(format, ...)    FT_DEBUG_PRINT_W(FMEMP_DEBUG_TAG, format, ##__VA_ARGS__)
#define FMEMP_INFO(format, ...)    FT_DEBUG_PRINT_I(FMEMP_DEBUG_TAG, format, ##__VA_ARGS__)
#define FMEMP_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FMEMP_DEBUG_TAG, format, ##__VA_ARGS__)

static inline boolean FMempTakeSema(SemaphoreHandle_t locker)
{
    FASSERT_MSG((NULL != locker), "Locker not exists.");
    if (pdFALSE == xSemaphoreTake(locker, portMAX_DELAY))
    {
        FMEMP_ERROR("Failed to give locker !!!");
        return FALSE;
    }

    return TRUE;
}

static inline void FMempGiveSema(SemaphoreHandle_t locker)
{
    FASSERT_MSG((NULL != locker), "Locker not exists.");
    if (pdFALSE == xSemaphoreGive(locker))
    {
        FMEMP_ERROR("Failed to give locker !!!");
    }

    return;
}

#ifdef FMEMP_TAG_DEBUG
static const char *status_info[] = {"allocated", "freed"};

/**
 * @name: FMempRemoveTagList
 * @msg: 删除内存块TAG链表
 * @return {*}
 * @param {FMemp} *memp, 内存池控制数据
 */
static void FMempRemoveTagList(FMemp *memp)
{
    FASSERT(memp != NULL);
    FMempTag *cur_tag = memp->tags_list;
    FMempTag *nxt_tag;

    while (NULL != cur_tag)
    {
        nxt_tag = cur_tag->next;
        FMempFree(memp, cur_tag);
        cur_tag = nxt_tag;
    }

    return;
}

/**
 * @name: FMempDumpTagList
 * @msg: 打印内存块TAG链表
 * @return {*}
 * @param {FMemp} *memp, 内存池控制数据
 */
static void FMempDumpTagList(FMemp *memp)
{
    FASSERT(memp != NULL);
    FMempTag *cur_tag = memp->tags_list;
    FMempTag *nxt_tag;

    FMEMP_INFO("Memory block status:  ");
    while (NULL != cur_tag)
    {
        nxt_tag = cur_tag->next;
        if (FMEMP_TAG_ALLOCATED == cur_tag->status)
        {
            FMEMP_INFO("    [%d]: @0x%lx, sz: %ld, st: %s, msg: %s",
                       cur_tag->id,
                       cur_tag->mem_addr,
                       cur_tag->mem_size,
                       status_info[cur_tag->status],
                       cur_tag->msg);
        }
        cur_tag = nxt_tag;
    }

    return;
}

/**
 * @name: FMempTagIfExists
 * @msg: 在TAG链表中寻找是否有指定地址对于的TAG节点
 * @return {boolean} TRUE: 已经找到，FALSE: 没有找到，需要新创建
 * @param {FMemp} *memp, 内存池控制数据
 * @param {uintptr} mem_addr, 指定的动态内存块起始地址
 * @param {FMempTagStatus} new_status, 设定的TAG节点状态
 * @param {fsize_t} new_size, 设定的TAG节点分配地址空间
 * @param {char} *new_msg, 设定的TAG节点标记信息
 */
static boolean FMempTagIfExists(FMemp *memp, uintptr mem_addr, FMempTagStatus new_status, fsize_t new_size, const char *new_msg)
{
    FASSERT(memp != NULL);
    FMempTag *cur_tag = memp->tags_list;
    FMempTag *nxt_tag;
    boolean found = FALSE;

    while (NULL != cur_tag)
    {
        nxt_tag = cur_tag->next;

        if (mem_addr == cur_tag->mem_addr)
        {
            if ((cur_tag->status == new_status) && (FMEMP_TAG_ALLOCATED == new_status))
            {
                FMEMP_ERROR("Allocate twice !!!");
            }
            else if ((cur_tag->status == new_status) && (FMEMP_TAG_FREEED == new_status))
            {
                FMEMP_ERROR("Free twice !!!");
            }

            FMEMP_DEBUG("Change @0x%lx from %s to %s.", cur_tag->mem_addr,
                        status_info[cur_tag->status],
                        status_info[new_status]);
            FMEMP_DEBUG("    size from %ld to %ld.",   cur_tag->mem_size,
                        new_size);
            cur_tag->status = new_status;
            cur_tag->mem_size = new_size;

            if (NULL != new_msg)
            {
                const fsize_t msg_len = min((fsize_t)(FMEMP_TAG_MSG_LEN - 1), strlen(new_msg));
                memcpy(cur_tag->msg, new_msg, msg_len);
                cur_tag->msg[msg_len] = '\0';
            }

            found = TRUE;
        }

        cur_tag = nxt_tag;
    }

    return found;
}

/**
 * @name: FMempDoTag
 * @msg: 在TAG链表中为分配的内存块添加TAG节点
 * @return {*}
 * @param {FMemp} *memp, 内存池控制数据
 * @param {void} *mem_addr, 动态内存块起始地址
 * @param {FMempTagStatus} status, 动态内存块状态
 * @param {fsize_t} size, 动态内存空间大小
 * @param {char} *msg, 动态内存块标记信息
 */
static void FMempDoTag(FMemp *memp, void *mem_addr, FMempTagStatus status, fsize_t size, const char *msg)
{
    FASSERT(memp != NULL);
    if (NULL == mem_addr)
    {
        /* NULL memory, allocate could be failed */
        return;
    }

    if (TRUE == FMempTagIfExists(memp, (uintptr)mem_addr, status, size, msg))
    {
        /* tag node already exist, no need to create new one */
        return;
    }

    FMempTag *tag = (FMempTag *)FMempMalloc(memp, sizeof(FMempTag));
    if (NULL == tag)
    {
        FMEMP_ERROR("Create new tag node failed !!!");
        return;
    }

    /* add new tag node as header */
    FMempTag *head_tag = memp->tags_list;
    tag->mem_addr = (uintptr)mem_addr;
    tag->mem_size = size;
    tag->status = status;

    tag->next = head_tag;

    if (NULL != msg)
    {
        const fsize_t msg_len = min((fsize_t)(FMEMP_TAG_MSG_LEN - 1), strlen(msg));
        memcpy(tag->msg, msg, msg_len);
        tag->msg[msg_len] = '\0';
    }

    if (NULL == head_tag)
    {
        tag->id = 0;
    }
    else
    {
        tag->id = head_tag->id + 1;
    }

    memp->tags_list = tag;
    return;
}

/**
 * @name: FMempMallocTag
 * @msg: 带TAG标记的内存申请
 * @return {void *} 分配的动态内存地址
 * @param {FMemp} *memp, 内存池控制数据
 * @param {fsize_t} size, 动态内存空间大小
 * @param {char} *file, 申请内存动作发送的源文件名
 * @param {unsigned long} line, 申请内存动作发送的源文件行号
 * @param {char} *msg, 申请内存动作发送的信息
 */
void *FMempMallocTag(FMemp *memp, fsize_t size, const char *file, unsigned long line, const char *msg)
{
    FASSERT(memp);
    char msg_buf[FMEMP_TAG_MSG_LEN] = {0};
    void *ptr = FMempMalloc(memp, size);
    snprintf(msg_buf, FMEMP_TAG_MSG_LEN, "%s_%d: %s", file, line, msg);
    FMempDoTag(memp, ptr, FMEMP_TAG_ALLOCATED, size, msg_buf);
    return ptr;
}

/**
 * @name: FMempMallocAlignTag
 * @msg: 带TAG标记的对齐内存申请
 * @return {void *} 分配的动态内存地址
 * @param {FMemp} *memp, 内存池控制数据
 * @param {fsize_t} size, 动态内存空间大小
 * @param {fsize_t} align, 分配内存的对齐方式
 * @param {char} *file, 申请内存动作发送的源文件名
 * @param {unsigned long} line, 申请内存动作发送的源文件行号
 * @param {char} *msg, 申请内存动作发送的信息
 */
void *FMempMallocAlignTag(FMemp *memp, fsize_t size, fsize_t align, const char *file, unsigned long line, const char *msg)
{
    FASSERT(memp);
    char msg_buf[FMEMP_TAG_MSG_LEN] = {0};
    void *ptr = FMempMallocAlign(memp, size, align);
    snprintf(msg_buf, FMEMP_TAG_MSG_LEN, "%s_%d: %s", file, line, msg);
    FMempDoTag(memp, ptr, FMEMP_TAG_ALLOCATED, size, msg_buf);
    return ptr;
}

/**
 * @name: FMempCallocTag
 * @msg: 带TAG标记的数组内存申请
 * @return {void *} 分配的动态内存地址
 * @param {FMemp} *memp, 内存池控制数据
 * @param {fsize_t} count, 动态内存数组的成员个数
 * @param {fsize_t} size 动态内存数组的单个成员大小
 * @param {char} *file, 申请内存动作发送的源文件名
 * @param {unsigned long} line, 申请内存动作发送的源文件行号
 * @param {char} *msg, 申请内存动作发送的信息
 */
void *FMempCallocTag(FMemp *memp, fsize_t count, fsize_t size, const char *file, unsigned long line, const char *msg)
{
    FASSERT(memp);
    char msg_buf[FMEMP_TAG_MSG_LEN] = {0};
    void *ptr = FMempCalloc(memp, count, size);
    snprintf(msg_buf, FMEMP_TAG_MSG_LEN, "%s_%d: %s", file, line, msg);
    FMempDoTag(memp, ptr, FMEMP_TAG_ALLOCATED, size, msg_buf);
    return ptr;
}

/**
 * @name: FMempReallocTag
 * @msg: 带TAG标记的动态内存重分配
 * @return {void *} 分配的动态内存地址
 * @param {FMemp} *memp, 内存池控制数据
 * @param {void} *ptr, 待重新回收分配的动态内存起始地址
 * @param {fsize_t} size, 重新分配的动态内存空间大小
 * @param {char} *file, 申请内存动作发送的源文件名
 * @param {unsigned long} line, 申请内存动作发送的源文件行号
 * @param {char} *msg, 申请内存动作发送的信息
 */
void *FMempReallocTag(FMemp *memp, void *ptr, fsize_t size, const char *file, unsigned long line, const char *msg)
{
    FASSERT(memp);
    char msg_buf[FMEMP_TAG_MSG_LEN] = {0};
    FMempDoTag(memp, ptr, FMEMP_TAG_FREEED, 0, NULL);
    ptr = FMempRealloc(memp, ptr, size);
    snprintf(msg_buf, FMEMP_TAG_MSG_LEN, "%s_%d: %s", file, line, msg);
    FMempDoTag(memp, ptr, FMEMP_TAG_ALLOCATED, size, msg_buf);
    return ptr;
}

/**
 * @name: FMempFreeTag
 * @msg: 带TAG标记的动态内存释放
 * @return {*}
 * @param {FMemp} *memp, 内存池控制数据
 * @param {void} *ptr, 待回收的动态内存起始地址
 */
void FMempFreeTag(FMemp *memp, void *ptr)
{
    FASSERT(memp);
    if (memp->tlsf_ptr)
    {
        FMempFree(memp, ptr);
        FMempDoTag(memp, ptr, FMEMP_TAG_FREEED, 0, NULL);
    }
    return;
}

#endif

/**
 * @name: FMempInit
 * @msg: 初始化内存池, 分配内存池的内存空间
 * @return {FError} FMEMP_SUCCESS表示初始化成功，返回其它值表示初始化失败
 * @param {FMemp} *memp, 内存池的控制数据
 * @param {void} *begin_addr, 分配给内存池的空间起始地址
 * @param {void} *end_addr, 分配给内存池的空间结束地址
 * @note begin_addr end_addr 指向为内存池指定的缓冲区的起止地址
 * 64位需要预留给内存池更大的空间
 */
FError FMempInit(FMemp *memp, void *begin_addr, void *end_addr)
{
    FASSERT(memp);
    size_t size;
    FError ret = FMEMP_SUCCESS;

    if ((FT_COMPONENT_IS_READY == memp->is_ready) || (NULL != memp->tlsf_ptr))
    {
        FMEMP_ERROR("Memory tlsf_ptr already inited, you may lose access to original memory tlsf_ptr.");
        return FMEMP_ERR_ALREADY_INIT;
    }

    if (begin_addr < end_addr)
    {
        size = (uintptr)end_addr - (uintptr)begin_addr;
    }
    else
    {
        FMEMP_ERROR("Invalid input buf beg: %p, end: %p.", begin_addr, end_addr);
        return FMEMP_ERR_INVALID_BUF;
    }

    FASSERT(NULL == memp->locker);
    FASSERT(NULL != (memp->locker = xSemaphoreCreateMutex()));

    /* no scheduler when init */
    taskENTER_CRITICAL();

    memp->tlsf_ptr = (tlsf_t)tlsf_create_with_pool(begin_addr, size);
    if (NULL == memp->tlsf_ptr)
    {
        FMEMP_ERROR("Allocate TLSF buf failed.");
        ret = FMEMP_ERR_INIT_TLFS;
        goto err_ret;
    }

    memp->pools_list = FMempMalloc(memp, sizeof(FMempPoolList *));
    if (NULL == memp->pools_list)
    {
        FMEMP_ERROR("Allocate TLSF block failed.");
        ret = FMEMP_ERR_BAD_MALLOC;
        goto err_ret;
    }

    FSListInit(&memp->pools_list->list);
    memp->pools_list->pool_addr = tlsf_get_pool(memp->tlsf_ptr);

    memp->is_ready = FT_COMPONENT_IS_READY;
#ifdef FMEMP_TAG_DEBUG
    memp->tags_list = NULL;
#endif

err_ret:
    if (FMEMP_SUCCESS != ret)
    {
        if (memp->tlsf_ptr) /* clear tlsf instance if exit with error */
        {
            tlsf_destroy(memp->tlsf_ptr);
            memp->tlsf_ptr = NULL;
        }
    }

    taskEXIT_CRITICAL(); /* allow schedule after init */

    return ret;
}

/**
 * @name: FMempDeinit
 * @msg: 释放所有分配的内存，删除内存池
 * @return {*}
 * @param {FMemp} *memp 内存池控制数据
 * @note 需要初始化后才能调用，调用此函数后，内存池分配的空间不再能使用
 */
void FMempDeinit(FMemp *memp)
{
    FASSERT(memp != NULL);

    FMempPoolList *pool_node = (FMempPoolList *)memp->pools_list;
    pool_t tlsf_ptr = pool_node->pool_addr;

    if ((FT_COMPONENT_IS_READY != memp->is_ready) && (NULL == memp->tlsf_ptr))
    {
        FMEMP_WARN("Memory tlsf_ptr not inited.");
        return;
    }

    /* no scheduler when init */
    taskENTER_CRITICAL();

#ifdef FMEMP_TAG_DEBUG
    FMempRemoveTagList(memp);
#endif
    if (memp->tlsf_ptr)
    {
        FSListRemove(&memp->pools_list->list, &pool_node->list);
        FMempFree(memp, pool_node);
        tlsf_remove_pool(memp->tlsf_ptr, tlsf_ptr);

        tlsf_destroy(memp->tlsf_ptr);
        memp->tlsf_ptr = NULL;
    }

    memp->is_ready = 0;

    FASSERT(NULL != memp->locker);
    vSemaphoreDelete(memp->locker);
    memp->locker = NULL;

    taskEXIT_CRITICAL(); /* allow schedule after init */

    return;
}

/**
 * @name: FMempMalloc
 * @msg: 从内存池申请一段空间
 * @return {void *} 申请到的空间，如果申请失败，返回NULL
 * @param {FMemp} *memp 内存池控制数据
 * @param {fsize_t} nbytes 申请的字节数
 * @note 需要初始化后才能调用，申请的空间再不再使用后需要调用FMempFree释放
 */
void *FMempMalloc(FMemp *memp, fsize_t nbytes)
{
    void *ptr = NULL;

    nbytes = (nbytes > 20) ? nbytes : 20;

    if (!FMempTakeSema(memp->locker))
    {
        return ptr;
    }

    if (memp->tlsf_ptr)
    {
        ptr = tlsf_malloc(memp->tlsf_ptr, nbytes);
    }

    FMempGiveSema(memp->locker);
    return ptr;
}

/**
 * @name: FMempMallocAlign
 * @msg: 按指定对齐方式申请一段空间
 * @return {void *} 申请到的空间，如果申请失败，返回NULL
 * @param {FMemp} *memp 内存池控制数据
 * @param {fsize_t} size 申请的字节数
 * @param {fsize_t} align 对齐字节数
 * @note 需要初始化后才能调用，申请的空间再不再使用后需要调用FMempFree释放
 */
void *FMempMallocAlign(FMemp *memp, fsize_t size, fsize_t align)
{
    void *ptr = NULL;

    size = (size > 20) ? size : 20;

    if (!FMempTakeSema(memp->locker))
    {
        return ptr;
    }

    if (memp->tlsf_ptr)
    {
        ptr = tlsf_memalign(memp->tlsf_ptr, align, size);
    }

    FMempGiveSema(memp->locker);
    return ptr;
}

/**
 * @name: FMempCalloc
 * @msg: 从内存池申请一段数组空间并清零
 * @return {void *} 申请到的空间，如果申请失败，返回NULL
 * @param {FMemp} *memp 内存池控制数据
 * @param {fsize_t} count 数据成员格式
 * @param {fsize_t} size 单个数据成员的字节数
 * @note 需要初始化后才能调用，申请的空间再不再使用后需要调用FMempFree释放
 */
void *FMempCalloc(FMemp *memp, fsize_t count, fsize_t size)
{
    void *ptr = NULL;
    fsize_t total_size;

    total_size = count * size;

    if (!FMempTakeSema(memp->locker))
    {
        return ptr;
    }

    ptr = FMempMalloc(memp, total_size);

    FMempGiveSema(memp->locker);
    if (ptr != NULL)
    {
        /* clean memory */
        memset(ptr, 0, total_size);
    }

    return ptr;
}

/**
 * @name: FMempRealloc
 * @msg: 回收原来申请的空间并重新分配
 * @return {void *} 替换后空间，如果替换失败，返回NULL
 * @param {FMemp} *memp 内存池控制数据
 * @param {void} *ptr 原来的空间
 * @param {fsize_t} nbytes 新申请的字节数
 * @note 需要初始化后才能调用，申请的空间再不再使用后需要调用FMempFree释放，调用函数后，原来的空间不再能使用，
 *  原空间的数据被移动到返回指针指向的空间
 */
void *FMempRealloc(FMemp *memp, void *ptr, fsize_t nbytes)
{
    if (!FMempTakeSema(memp->locker))
    {
        return ptr;
    }

    if (memp->tlsf_ptr)
    {
        ptr = tlsf_realloc(memp->tlsf_ptr, ptr, nbytes);
    }

    FMempGiveSema(memp->locker);
    return ptr;
}

/**
 * @name: FMempFree
 * @msg: 释放一段从内存池申请的空间
 * @return {void} 无
 * @param {FMemp} *memp 内存池控制数据
 * @param {void} *ptr 待释放的空间地址
 * @note 需要初始化后才能调用，传入的指针需要是FMempMalloc/FMempCalloc/FMempMallocAlign/FMempRealloc返回的
 */
void FMempFree(FMemp *memp, void *ptr)
{
    if (!FMempTakeSema(memp->locker))
    {
        return;
    }

    if (memp->tlsf_ptr)
    {
        tlsf_free(memp->tlsf_ptr, ptr);
    }

    FMempGiveSema(memp->locker);
}


static size_t used_mem = 0;
static size_t total_mem = 0;
static void FMemInfo(void *ptr, size_t size, int used, void *user)
{
    if (used)
    {
        used_mem += size;
    }

    total_mem += size;
}

/**
 * @name: FMemProbe
 * @msg: 获取当前内存池的使用情况
 * @return {*}
 * @param {FMemp} *memp 内存池控制数据
 * @param {u32} *total 总可用字节数
 * @param {u32} *used 已使用字节数
 * @param {u32} *max_used 已使用字节数的峰值
 * @note 需要初始化后才能调用
 */
void FMemProbe(FMemp *memp, u32 *total, u32 *used, u32 *max_used)
{
    used_mem = 0;
    total_mem = 0;

    if (!FMempTakeSema(memp->locker))
    {
        return;
    }

    tlsf_walk_pool(tlsf_get_pool(memp->tlsf_ptr), FMemInfo, 0);

    FMempGiveSema(memp->locker);

    *total = total_mem;
    *used = used_mem;
    *max_used = used_mem;
}

/**
 * @name: FMemListAll
 * @msg: 打印当前分配的内存块信息
 * @return {*}
 * @param {FMemp} *memp 内存池控制数据
 * @note 需要初始化后才能调用
 */
void FMemListAll(FMemp *memp)
{
    u8 i = 0;
    u8 len = 0;

    FMempPoolList *pool_node;
    FSListNode *node;
    node = &memp->pools_list->list;

    used_mem = 0;
    total_mem = 0;

    if (!FMempTakeSema(memp->locker))
    {
        return;
    }

    len = FSListLen(node) + 1;
    for (i = 0; i < len; i++)
    {
        pool_node = CONTAINER_OF(node, FMempPoolList, list);
        tlsf_walk_pool(pool_node->pool_addr, FMemInfo, (void *)1);
        node = node->next;
    }

    FMempGiveSema(memp->locker);

    FMEMP_INFO("Total memory: %d", total_mem);
    FMEMP_INFO("Used memory : %d", used_mem);
    FMEMP_INFO("Check tlsf: %d", tlsf_check(memp->tlsf_ptr));
    FMEMP_INFO("Check tlsf pool: %d", tlsf_check_pool(memp->tlsf_ptr));

#ifdef FMEMP_TAG_DEBUG
    FMempDumpTagList(memp);
#endif
}