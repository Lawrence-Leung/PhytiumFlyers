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
 * FilePath: fqspi_spiffs_port.h
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:24:52
 * Description:  This file is for providing spiffs api based on qspi.
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2022/8/5    first commit
 */

#ifndef  FQSPI_SPIFFS_PORT_H
#define  FQSPI_SPIFFS_PORT_H

#ifdef __cplusplus
extern "C"
{
#endif
/***************************** Include Files *********************************/
#include "ftypes.h"

#include "spiffs_port.h"
/************************** Constant Definitions *****************************/
enum
{
    FSPIFFS_QSPI_PORT_OK = 0,
    FSPIFFS_QSPI_PORT_SFUD_INIT_FAILED,
    FSPIFFS_QSPI_PORT_SFUD_NOT_READY,
    FSPIFFS_QSPI_PORT_SFUD_IO_ERROR,
    FSPIFFS_QSPI_PORT_ALREADY_INITED,
};

/* 根据SPIFFS的技术手册，最优页尺寸可以参考公式
    ~~~   Logical Page Size = Logical Block Size / 256   ~~~
 */
#define FSPIFFS_LOG_PAGE_SIZE           256 /* size of logic page */
#define FSPIFFS_LOG_BLOCK_SIZE          (FSPIFFS_LOG_PAGE_SIZE * 256)
/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int FSpiffsQspiInitialize(FSpiffs *const instance, fsize_t flash_id);
void FSpiffsQspiDeInitialize(FSpiffs *const instance);
const spiffs_config *FSpiffsQspiGetDefaultConfig(void);

#ifdef __cplusplus
}
#endif

#endif