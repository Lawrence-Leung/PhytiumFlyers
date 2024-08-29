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
 * FilePath: fspim_sfud_core.h
 * Date: 2022-08-19 10:09:31
 * LastEditTime: 2022-08-19 10:09:31
 * Description:  This files is for
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/8/26   first release
 */
#ifndef FSPIM_SFUD_CORE_H
#define FSPIM_SFUD_CORE_H

#include "sfud.h"
#include "sfud_def.h"

#ifdef __cplusplus
extern "C"
{
#endif

sfud_err FSpimProbe(sfud_flash *flash);

#ifdef __cplusplus
}
#endif

#endif // !