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
 * FilePath: ddma_spi_loopback.h
 * Date: 2022-07-18 11:01:37
 * LastEditTime: 2022-07-18 11:01:37
 * Description:  This file is for task create function define
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 zhugengyu 2022/08/26  first commit
 */
#ifndef  DDMA_SPI_LOOPBACK_H
#define  DDMA_SPI_LOOPBACK_H

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
BaseType_t FFreeRTOSRunDDMASpiLoopback(u32 spi_id, u32 bytes);

#ifdef __cplusplus
}
#endif

#endif