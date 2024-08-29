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
 * FilePath: fgdma_os.h
 * Date: 2022-07-20 10:54:37
 * LastEditTime: 2022-07-20 10:54:37
 * Description:  This file is for providing function related definitions of gdma driver used in FreeRTOS.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/7/27   init commit
 */
#ifndef  FGDMA_OS_H
#define  FGDMA_OS_H
/***************************** Include Files *********************************/
#include <FreeRTOS.h>
#include <semphr.h>
#include <event_groups.h>

#include "fparameters.h"
#include "fgdma.h"
#include "fmemory_pool.h"
/************************** Constant Definitions *****************************/
#ifdef __cplusplus
extern "C"
{
#endif

#define FFREERTOS_GDMA_OK                   FT_SUCCESS
#define FFREERTOS_GDMA_NOT_INIT             FT_CODE_ERR(ErrModPort, ErrGdma, 0)
#define FFREERTOS_GDMA_SEMA_ERR             FT_CODE_ERR(ErrModPort, ErrGdma, 1)
#define FFREERTOS_GDMA_ALREADY_INIT         FT_CODE_ERR(ErrModPort, ErrGdma, 2)
#define FFREERTOS_GDMA_ALLOCATE_FAIL        FT_CODE_ERR(ErrModPort, ErrGdma, 3)
#define FFREERTOS_GDMA_MEMCPY_FAIL          FT_CODE_ERR(ErrModPort, ErrGdma, 4)

#define FFREERTOS_GDMA_IRQ_PRIORITY         IRQ_PRIORITY_VALUE_12
/**************************** Type Definitions *******************************/

typedef struct
{
    FGdmaChan chan;
    FGdmaBdlDesc *bdl_list; /* descriptor of every chan, dynamic allocated */
} FFreeRTOSGdmaChan; /* instance of gdma channel in FreeRTOS */

typedef struct
{
    FGdma ctrl;
    SemaphoreHandle_t locker;
    FFreeRTOSGdmaChan chan[FGDMA_NUM_OF_CHAN];
    FMemp memp;          /* instance of memory pool */
    u8 memp_buf[SZ_16K]; /* buffer used to support dynamic memory */
} FFreeRTOSGdma; /* instance of gdma in FreeRTOS */

typedef struct
{
    u8 *src_buf; /* src memory buffer */
    u8 *dst_buf; /* dst memory buffer */
    fsize_t data_len; /* length of src & dst memory buffer */
} FFreeRTOSGdmaTranscation; /* config of one memcpy transaction */

typedef struct
{
    FFreeRTOSGdmaTranscation *trans; /* list of transcations */
    u32 valid_trans_num; /* num of transcations can be used in request */
    u32 total_trans_num; /* length of transcation buffer */
    FGdmaChanEvtHandler req_done_handler; /* callback for request done */
    void *req_done_args;
} FFreeRTOSGdmaRequest; /* config of one memcpy request, may include multiple transaction */

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/* init and get gdma instance */
FFreeRTOSGdma *FFreeRTOSGdmaInit(u32 instance_id);

/* deinit gdma instance */
FError FFreeRTOSGdmaDeInit(FFreeRTOSGdma *const instance);

/* start gdma transfer by request */
FError FFreeRTOSGdmaSetupChannel(FFreeRTOSGdma *const instance, u32 chan_id, const FFreeRTOSGdmaRequest *req);

/* revoke setup of allocated channel */
FError FFreeRTOSGdmaRevokeChannel(FFreeRTOSGdma *const instance, u32 chan_id);

/* start up dma transfer */
FError FFreeRTOSGdmaStart(FFreeRTOSGdma *const instance, u32 chan_id);


#ifdef __cplusplus
}
#endif

#endif