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
 * FilePath: fqspi_sfud_core.h
 * Date: 2022-02-10 14:53:44
 * LastEditTime: 2022-02-25 11:46:38
 * Description:  This files is for providing sfud api based on qspi
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  wangxiaodong 2022/8/9   first commit
 */


#ifndef FQSPI_SFUD_CORE_H
#define FQSPI_SFUD_CORE_H

#include "sfud.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "sfud_def.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FQSPI0_SFUD_NAME "FQSPI0"

sfud_err FQspiProbe(sfud_flash *flash) ;

#ifdef __cplusplus
}
#endif

#endif // !
