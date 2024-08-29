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
 * FilePath: spiffs_port.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:24:47
 * Description:  This file is for providing func that divide sfud api into qspi and spi. 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  zhugengyu   2022/4/15    first commit, support Spiffs
 */

/***************************** Include Files *********************************/

#include "sdkconfig.h"
#include "fassert.h"

#include "spiffs_port.h"

#ifdef CONFIG_SPIFFS_ON_FSPIM_SFUD
#include "fspim_spiffs_port.h"
#endif
#ifdef CONFIG_SPIFFS_ON_FQSPI_SFUD
#include "fqspi_spiffs_port.h"
#endif
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int FSpiffsInitialize(FSpiffs *const instance, FSpiffsPortType type, fsize_t flash_id)
{
#ifdef CONFIG_SPIFFS_ON_FSPIM_SFUD
    FASSERT(FSPIFFS_PORT_TO_FSPIM == type);
    return FSpiffsSpimInitialize(instance, flash_id);
#endif
#ifdef CONFIG_SPIFFS_ON_FQSPI_SFUD
    FASSERT(FSPIFFS_PORT_TO_FQSPI == type);
    return FSpiffsQspiInitialize(instance, flash_id);
#endif
}

void FSpiffsDeInitialize(FSpiffs *const instance)
{
#ifdef CONFIG_SPIFFS_ON_FSPIM_SFUD
    FSpiffsSpimDeInitialize(instance);
    return;
#endif
#ifdef CONFIG_SPIFFS_ON_FQSPI_SFUD
    return FSpiffsQspiDeInitialize(instance);
#endif
}

const spiffs_config *FSpiffsGetDefaultConfig(void)
{
#ifdef CONFIG_SPIFFS_ON_FSPIM_SFUD
    return FSpiffsSpimGetDefaultConfig();
#endif
#ifdef CONFIG_SPIFFS_ON_FQSPI_SFUD
    return FSpiffsQspiGetDefaultConfig();
#endif
}