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
 * FilePath: qspi_spiffs_example.h
 * Date: 2022-06-17 10:42:40
 * LastEditTime: 2022-06-17 10:42:40
 * Description:  This file is for the qspi_spiffs example function declarations.
 *
 * Modify History:
 *  Ver      Who            Date           Changes
 * -----   ------         --------    --------------------------------------
 * 1.0   wangxiaodong    2022/8/9       first release
 */

#ifndef QSPI_SPIFFS_EXAMPLE_H
#define QSPI_SPIFFS_EXAMPLE_H

#ifdef __cplusplus
extern "C"
{
#endif

/* qspi spiffs read and write test */
BaseType_t FFreeRTOSQspiSpiffsCreate(u32 qspi_id);


#ifdef __cplusplus
}
#endif

#endif // !