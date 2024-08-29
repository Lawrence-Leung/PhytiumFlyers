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
 * FilePath: cc.h
 * Date: 2022-11-07 11:49:13
 * LastEditTime: 2022-11-07 11:49:14
 * Description:  This file is for the lwIP TCP/IP stack.
 *
 * Modify History:
 *  Ver   Who       Date   Changes
 * ----- ------    -------- --------------------------------------
 *  1.0   liuzhihong  2022/11/26  first release
 */

#ifndef CC_H
#define CC_H


#if defined __ANDROID__
#define LWIP_UNIX_ANDROID
#elif defined __linux__
#define LWIP_UNIX_LINUX
#elif defined __APPLE__
#define LWIP_UNIX_MACH
#elif defined __OpenBSD__
#define LWIP_UNIX_OPENBSD
#elif defined __CYGWIN__
#define LWIP_UNIX_CYGWIN
#elif defined __GNU__
#define LWIP_UNIX_HURD
#endif

#define LWIP_NO_STDINT_H  1 /*not include "stdint.h"*/

/* define basic types */
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef uint8_t       u8_t;            // unsigned 8-bit
typedef signed char   s8_t;               // signed 8-bit
typedef uint16_t      u16_t;          // unsigned 16-bit
typedef short         s16_t;             // signed 16-bit
typedef uint32_t      u32_t;          // unsigned 32-bit
typedef signed int         s32_t;              // signed 32-bit 
typedef uint64_t      u64_t;          // unsigned 64-bit
#if defined(__aarch64__)
typedef u64_t         mem_ptr_t;
#else
typedef u32_t         mem_ptr_t;
#endif

/* define printf format */
#define U16_F     "hu"
#define S16_F     "d"
#define X16_F     "hx"
#define U32_F     "u"
#define S32_F     "d"
#define X32_F     "x"
#if defined(__aarch64__)
#define SZT_F     "luz"
#else
#define SZT_F     "uz"
#endif

#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif

#define LWIP_TIMEVAL_PRIVATE 0
#include <sys/time.h>

#define LWIP_ERRNO_INCLUDE <errno.h>

#if defined(LWIP_UNIX_LINUX) || defined(LWIP_UNIX_HURD)
    #define LWIP_ERRNO_STDINCLUDE   1
#endif

#define LWIP_RAND() ((u32_t)rand())

/* different handling for unit test, normally not needed */
#ifdef LWIP_NOASSERT_ON_ERROR
#define LWIP_ERROR(message, expression, handler) do { if (!(expression)) { \
  handler;}} while(0)
#endif

#if defined(LWIP_UNIX_ANDROID) && defined(FD_SET)
typedef __kernel_fd_set fd_set;
#endif

#if defined(LWIP_UNIX_MACH)
/* sys/types.h and signal.h bring in Darwin byte order macros. pull the
   header here and disable LwIP's version so that apps still can get
   the macros via LwIP headers and use system headers */
#include <sys/types.h>
#define LWIP_DONT_PROVIDE_BYTEORDER_FUNCTIONS
#endif

struct sio_status_s;
typedef struct sio_status_s sio_status_t;
#define sio_fd_t sio_status_t*
#define __sio_fd_t_defined

typedef unsigned int sys_prot_t;


#ifdef __cplusplus
}
#endif

#endif /* LWIP_ARCH_CC_H */
