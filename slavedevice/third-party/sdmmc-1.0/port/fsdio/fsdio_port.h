
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
 * FilePath: fsdio_port.h
 * Date: 2022-02-10 14:53:44
 * LastEditTime: 2022-02-25 11:46:22
 * Description:  This files is for sdio port
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/12/7   init commit
 */

#ifndef FSDIO_PORT_H
#define FSDIO_PORT_H

#include "sdmmc_host_os.h"

#ifdef __cplusplus
extern "C"
{
#endif

sdmmc_err_t fsdio_host_init(sdmmc_host_instance_t *const instance, const sdmmc_host_config_t *config);

sdmmc_err_t fsdio_host_deinit(sdmmc_host_instance_t *const instance);

sdmmc_err_t fsdio_host_lock(sdmmc_host_t *const host);

void fsdio_host_unlock(sdmmc_host_t *const host);

#ifdef __cplusplus
}
#endif

#endif