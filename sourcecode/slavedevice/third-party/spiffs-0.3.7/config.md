# SPIFFS文件系统移植配置解释
移植SPIFFS过程中需要对其文件目录`spiffs-0.3.7\src\default`下`spiffs_config.h`配置文件进行一定修改，来使其适配不同的应用环境。本文档介绍飞腾FreeTROS环境下SPIFFS的相关配置。

### github
在github上有wiki说明SPIFFS相关配置信息：
- http://github.com/pellepl/spiffs/wiki

SPIFFS技术手册：
- http://blog.csdn.net/zhangjinxing_2006/article/details/75050611

### 重要移植概述
在SPIFFS暴露的配置中，由许多是不需要修改的。在用户移植的过程中，可以参考如下配置设置。
#####基础函数库配置
```
#ifndef SPIFFS_CONFIG_H_
#define SPIFFS_CONFIG_H_

// ----------- 8< ------------
// Following includes are for the linux test build of spiffs
// These may/should/must be removed/altered/replaced in your target
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>

#include "ftypes.h"
#include "fdebug.h"

#ifdef _SPIFFS_TEST
#include "testrunner.h"
#endif
// ----------- >8 ------------
```
在一系列C语言库函数后，我们加入了飞腾的定义库`"ft_types.h"`与debug库`"ft_debug.h"`

#### Debug方式
使用飞腾定义好的debug方法
```
// port debug printfs
// Set generic spiffs debug output call.
#ifndef SPIFFS_DBG
#define SPIFFS_DBG(_f, ...) FT_DEBUG_PRINT_D("SPIFFS", _f, ##__VA_ARGS__)
#endif
// Set spiffs debug output call for garbage collecting.
#ifndef SPIFFS_GC_DBG
#define SPIFFS_GC_DBG(_f, ...) FT_DEBUG_PRINT_D("SPIFFS-GC", _f, ##__VA_ARGS__)
#endif
// Set spiffs debug output call for caching.
#ifndef SPIFFS_CACHE_DBG
#define SPIFFS_CACHE_DBG(_f, ...) FT_DEBUG_PRINT_D("SPIFFS-CACHE", _f, ##__VA_ARGS__)
#endif
// Set spiffs debug output call for system consistency checks.
#ifndef SPIFFS_CHECK_DBG
#define SPIFFS_CHECK_DBG(_f, ...) FT_DEBUG_PRINT_D("SPIFFS-CHECK", _f, ##__VA_ARGS__)
#endif
// Set spiffs debug output call for all api invocations.
#ifndef SPIFFS_API_DBG
#define SPIFFS_API_DBG(_f, ...) FT_DEBUG_PRINT_D("SPIFFS-API", _f, ##__VA_ARGS__)
#endif
```
#### Cache配置
如果启用Cache缓存机制，会大大加速执行速度，但是必须为SPIFFS提供额外的RAM内存。强烈建议打开`SPIFFS_CACHE_WR`选项，这个选项会使得写入flash的数据进入缓存区，多次得到写入的数据后，再一次性写入flash。如果没有打开这个选项，所有对flash的写入都是直接的，这样会使得flash寿命减少。
```
// Enables/disable memory read caching of nucleus file system operations.
// If enabled, memory area must be provided for cache in SPIFFS_mount.

#ifndef  SPIFFS_CACHE
#define SPIFFS_CACHE                    1  /* 在RAM中为SPIFFS提供缓存区 */
#endif

#if SPIFFS_CACHE

// Enables memory write caching for file descriptors in hydrogen
#ifndef  SPIFFS_CACHE_WR
#define SPIFFS_CACHE_WR                 1  /* 使能文件描述符写缓存 */
#endif

#endif

#ifndef SPIFFS_TEMPORAL_FD_CACHE
#define SPIFFS_TEMPORAL_FD_CACHE              1 /* 启用此选项可优化文件的打开。spiffs 获得的文件描述符越多，缓存就越有效。请注意，这会向每个文件描述符添加额外的 6 个字节 */
#endif

// Temporal file cache hit score. Each time a file is opened, all cached files
// will lose one point. If the opened file is found in cache, that entry will
// gain SPIFFS_TEMPORAL_CACHE_HIT_SCORE points. One can experiment with this
// value for the specific access patterns of the application. However, it must
// be between 1 (no gain for hitting a cached entry often) and 255.
#ifndef SPIFFS_TEMPORAL_CACHE_HIT_SCORE
#define SPIFFS_TEMPORAL_CACHE_HIT_SCORE       4 /* 启用文件缓存优先级系统 */
#endif


```
#### 基本flash信息配置
飞腾不止使用单个SPIFFS实例，故将`SPIFFS_SINGLETON`设为0。并且不需要预设flash信息为某一固定值，通过其他模块的接口，可以实现读取装在SPIM上的flash信息。对于一般用户来说，可以将此选项打开，并直接在SPIFFS层面设置好相关配置。
```
#ifndef SPIFFS_SINGLETON
#define SPIFFS_SINGLETON 0  /* 0表示只使用单个SPIFFS实例 */
#endif

#if SPIFFS_SINGLETON

// Instead of giving parameters in config struct, singleton build must
// give parameters in defines below.
#ifndef SPIFFS_CFG_PHYS_SZ
#define SPIFFS_CFG_PHYS_SZ(ignore)        (1024*1024*2) /* flash分配给SPIFFS的容量 2MB */
#endif

#ifndef SPIFFS_CFG_PHYS_ERASE_SZ
#define SPIFFS_CFG_PHYS_ERASE_SZ(ignore)  (65536) /* flash擦除单位 */
#endif

#ifndef SPIFFS_CFG_PHYS_ADDR
#define SPIFFS_CFG_PHYS_ADDR(ignore)      (0) /* flash起始物理地址 */
#endif

#ifndef SPIFFS_CFG_LOG_PAGE_SZ
#define SPIFFS_CFG_LOG_PAGE_SZ(ignore)    (256) /* 页大小，写数据的单位，为256的整数倍 */
#endif

#ifndef SPIFFS_CFG_LOG_BLOCK_SZ
#define SPIFFS_CFG_LOG_BLOCK_SZ(ignore)   (65536) /* 块大小，一般与 flash 擦除单位保持一致 */
#endif

#endif
```
如果不能正确配置这些类型，可能会由溢出和一堆其他问题造成死循环。在您无法弄清楚文件系统的需求配置时，可以选择将容量设置稍大一点，因为如果设置得太小，会导致更加严重的后果。
#### flash文件系统检查
打开flash检查magic开关，设置启用按文件系统长度来检查flash配置，保证flash配置符合SPIFFS要求，如果进一步打开`SPIFFS_USE_MAGIC_LENGTH`，则会进一步比较挂载文件系统与flash配置长度是否一致。
```
#ifndef SPIFFS_USE_MAGIC
#define SPIFFS_USE_MAGIC                (1)
#endif

#if SPIFFS_USE_MAGIC

// Only valid when SPIFFS_USE_MAGIC is enabled. If SPIFFS_USE_MAGIC_LENGTH is
// enabled, the magic will also be dependent on the length of the filesystem.
// For example, a filesystem configured and formatted for 4 megabytes will not
// be accepted for mounting with a configuration defining the filesystem as 2
// megabytes.
#ifndef SPIFFS_USE_MAGIC_LENGTH
#define SPIFFS_USE_MAGIC_LENGTH         (1)
#endif

#endif
```
#### 上锁操作
为了适配FreeRTOS这种多任务的实时操作系统，需要打开SPIFFS的上锁功能开关，并将写好的上锁和解锁接口告诉SPIFFS。
```
extern void FSpiffsSemLock(void);
extern void FSpiffsSemUnlock(void);

// SPIFFS_LOCK and SPIFFS_UNLOCK protects spiffs from reentrancy on api level
// These should be defined on a multithreaded system

// define this to enter a mutex if you're running on a multithreaded system
#ifndef SPIFFS_LOCK
#define SPIFFS_LOCK(fs) FSpiffsSemLock()
#endif

// define this to exit a mutex if you're running on a multithreaded system
#ifndef SPIFFS_UNLOCK
#define SPIFFS_UNLOCK(fs) FSpiffsSemUnlock()
#endif
```
锁函数定义
```
void FSpiffsSemLock(void)
{
    xSemaphoreTake(xSpiffsSemaphore, portMAX_DELAY);
}

void FSpiffsSemUnlock(void)
{
    xSemaphoreGive(xSpiffsSemaphore);
}
```
#### 读写函数接口
在创建的SPIFFS文件系统config实例中，需要添加读写文件与擦除数据的操作
```
static const spiffs_config cfg =
    {
        .phys_addr = FSPIFFS_FLASH_START_ADDR, /* start spiffs at start of spi flash */
        .phys_size = FSPIFFS_FLASH_SIZE, /* flash_capcity in use */ 
        .phys_erase_block = FSPIFFS_LOG_BLOCK_SIZE, /* according to datasheet */
        .log_block_size = FSPIFFS_LOG_BLOCK_SIZE, /* let us not complicate things */
        .log_page_size = FSPIFFS_LOG_PAGE_SIZE, /* as we said */
        .hal_read_f = FSpiffsRead,
        .hal_write_f = FSpiffsWrite,
        .hal_erase_f = FSpiffsErase
    };
```
其中文件系统的地址、大小等参数由读取到的flash或者具体的用例确定，而读写操作的函数是基于sfud库中的函数实现的
    
