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
 * Description:  This files is for providing func that divide sfud api into qspi and spi.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangxiaodong 2022/8/9   first release
 */

/***************************** Include Files *********************************/

#include "sdkconfig.h"
#include "fassert.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

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
static xSemaphoreHandle xSpiffsSemaphore;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
void FSpiffsSemLock(void)
{
    xSemaphoreTake(xSpiffsSemaphore, portMAX_DELAY);
}

void FSpiffsSemUnlock(void)
{
    xSemaphoreGive(xSpiffsSemaphore);
}

void FSpiffsSemCreate(void)
{
    xSpiffsSemaphore = xSemaphoreCreateMutex();
}

void FSpiffsSemDelete(void)
{
    vSemaphoreDelete(xSpiffsSemaphore);
}

int FSpiffsInitialize(FSpiffs *const instance, FSpiffsPortType type)
{

    FSpiffsSemCreate();
#ifdef CONFIG_SPIFFS_ON_FSPIM_SFUD
    FASSERT(FSPIFFS_PORT_TO_FSPIM == type);
    return FSpiffsSpimInitialize(instance);
#endif
#ifdef CONFIG_SPIFFS_ON_FQSPI_SFUD
    FASSERT(FSPIFFS_PORT_TO_FQSPI == type);
    return FSpiffsQspiInitialize(instance);
#endif
}

void FSpiffsDeInitialize(FSpiffs *const instance)
{
    FSpiffsSemDelete();
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
