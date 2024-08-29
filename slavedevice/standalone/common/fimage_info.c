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

#include "fimage_info.h"
#include "fcompiler.h"
#include "sdkconfig.h"
#include "fparameters.h"


#if defined(CONFIG_IMAGE_INFO)
#if CONFIG_IMAGE_CORE > FCORE_NUM
    #error "Check the configuration of IMAGE_CORE, which needs to be less than the maximum number of cores allowed by SOC"
#endif

FImageInfo fimage_info FCOMPILER_SECTION(".my_image_info") =
{
        .magic_code = FIMAGE_MAGIC_CODE,
        .image_type = FIMAGE_BAREMETAL_TYPE_ID,
        .phy_address = 0,
        .phy_endaddress = 0,
        .virt_address = 0,
        .virt_endaddress = 0,
        /* boot parameters */
        .use_boot_parameters = 1,
        .process_core = CONFIG_IMAGE_CORE,
};

#endif