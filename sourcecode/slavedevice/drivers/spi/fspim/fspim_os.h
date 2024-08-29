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
 * FilePath: fspim_os.h
 * Date: 2022-07-18 09:05:48
 * LastEditTime: 2022-07-18 09:05:48
 * Description:  This file is for providing function related definitions of spi master driver used in FreeRTOS.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/7/27   init commit
 */

#ifndef  FSPIM_OS_H
#define  FSPIM_OS_H
/***************************** Include Files *********************************/
#include <FreeRTOS.h>
#include <semphr.h>
#include <event_groups.h>

#include "fparameters.h"
#include "fspim.h"

#ifdef __cplusplus
extern "C"
{
#endif
/************************** Constant Definitions *****************************/
#define FFREERTOS_SPIM_OK                   FT_SUCCESS
#define FFREERTOS_SPIM_NOT_INIT             FT_CODE_ERR(ErrModPort, ErrBspSpi, 0)
#define FFREERTOS_SPIM_SEMA_ERR             FT_CODE_ERR(ErrModPort, ErrBspSpi, 1)
#define FFREERTOS_SPIM_ALREADY_INIT         FT_CODE_ERR(ErrModPort, ErrBspSpi, 2)
#define FFREERTOS_SPIM_WAIT_EVT_TIMOUT      FT_CODE_ERR(ErrModPort, ErrBspSpi, 3)
#define FFREERTOS_SPIM_DMA_RUNNING          FT_CODE_ERR(ErrModPort, ErrBspSpi, 4)

#define FFREERTOS_SPIM_IRQ_PRIORITY         IRQ_PRIORITY_VALUE_12

#define FFREERTOS_SPIM_MODE_0               0U /* CPOL = 0, CPHA = 0 */
#define FFREERTOS_SPIM_MODE_1               1U /* CPOL = 1, CPHA = 0 */
#define FFREERTOS_SPIM_MODE_2               2U /* CPOL = 0, CPHA = 1 */
#define FFREERTOS_SPIM_MODE_3               3U /* CPOL = 1, CPHA = 1 */
/**************************** Type Definitions *******************************/

typedef struct
{
    u32 spi_mode; /* use FFREERTOS_SPIM_MODE_* */
    boolean en_dma; /* TRUE: data transfer in DMA mode */
    boolean inner_loopback; /* TRUE: enable internal loopback, ext pin no longer usable */
} FFreeRTOSSpimConifg; /* freertos spim config */

typedef struct
{
    FSpim ctrl;
    FFreeRTOSSpimConifg config;
    SemaphoreHandle_t locker;
    EventGroupHandle_t evt;
#define FFREERTOS_TRANS_DONE     (0x1 << 0)
} FFreeRTOSSpim; /* freertos spim instance */

typedef struct
{
    const u8 *tx_buf; /* send buffer */
    u8 *rx_buf; /* receive buffer */
    fsize_t tx_len;
    fsize_t rx_len;
} FFreeRTOSSpiMessage; /* freertos spim transfer message */
/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/

/* init and get spi instance */
FFreeRTOSSpim *FFreeRTOSSpimInit(u32 instance_id, const FFreeRTOSSpimConifg *config);

/* deinit spi instance */
FError FFreeRTOSSpimDeInit(FFreeRTOSSpim *const instance);

/* for NON-DMA transfer, start spi transfer and wait transfer done in this function,
   for DMA transfer, start DMA channel first, then call this function and wait DMA channel end later */
FError FFreeRTOSSpimTransfer(FFreeRTOSSpim *const instance, const FFreeRTOSSpiMessage *message);

#ifdef __cplusplus
}
#endif

#endif