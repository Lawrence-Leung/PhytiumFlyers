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
 * FilePath: felf.c
 * Date: 2021-08-31 11:16:59
 * LastEditTime: 2022-02-17 18:05:16
 * Description:  This file is for image information of boot
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  huanghe  2023/05/26       init
 */

#ifndef FIMAGE_INFO_H
#define FIMAGE_INFO_H

#include "sdkconfig.h"
#include "ftypes.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define FIMAGE_BAREMETAL_TYPE_ID 1
#define FIMAGE_MAGIC_CODE 0xCAFEBABEDEADBEEF
#define FIMAGE_USE_IMAGE_PARAMETER 1

typedef struct 
{
    u64 magic_code;             /* 魔数，用于标识此结构体是否有效 */
    u64 image_type;             /* image 类型，如 kernel、ramdisk 等 */
    u64 phy_address;            /* image 的物理地址 */
    u64 phy_endaddress;         /* image 的物理结束地址 */
    u64 virt_address;           /* image 的虚拟地址 */
    u64 virt_endaddress;        /* image 的虚拟结束地址 */
    /* boot parameters */
    u64 use_boot_parameters;    /* 是否使用 boot 参数 */
    u64 process_core;           /* image 运行的核心 */
    u64 startup_priority;       /* image 启动的优先级 */
    u64 startup_delay_ms;       /* image 启动的延迟时间（毫秒）*/
} FImageInfo;


#ifdef __cplusplus
extern "C"
{
#endif

#endif
