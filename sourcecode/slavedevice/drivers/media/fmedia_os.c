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
 * FilePath: fmedia_os.c
 * Date: 2022-09-15 14:20:19
 * LastEditTime: 2022-09-21 16:59:51
 * Description:  This file is for providing the media driver
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  Wangzq     2022/12/20  Modify the format and establish the version
 */
#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include "ftypes.h"
#include "fdebug.h"
#include "fparameters_comm.h"
#include "fmedia_os.h"
#include "fdcdp.h"
#include "fdp_hw.h"
#include "fdp.h"
#include "fdc_common_hw.h"

/***************** Macros (Inline Functions) Definitions *********************/

#define FMEDIA_DEBUG_TAG "FFreeRTOSMEDIA"
#define FMEDIA_ERROR(format, ...) FT_DEBUG_PRINT_E(FMEDIA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FMEDIA_INFO(format, ...) FT_DEBUG_PRINT_I(FMEDIA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FMEDIA_DEBUG(format, ...) FT_DEBUG_PRINT_D(FMEDIA_DEBUG_TAG, format, ##__VA_ARGS__)

static FFreeRTOSMedia os_media;
/************************** Function Prototypes ******************************/

/**
 * @name: FFreeRTOSMediaHwInit
 * @msg:  init the media,dc and dp
 * @param  {u32} channel is the dc channel
 * @param  {u32} width is the width
 * @param  {u32} height is the height
 * @param  {u32} multi_mode is multi display mode,0:clone,1:hor,2:ver
 * @param  {u32} color_depth is the color depth
 * @param  {u32} refresh_rate is the refresh rate of screen
 * @return err code information, 0 indicates success，others indicates failed
 */
FFreeRTOSMedia *FFreeRTOSMediaHwInit(u32 channel, u32 width, u32 height, u32 multi_mode,u32 color_depth,u32 refresh_rate)
{
    FError ret = FT_SUCCESS;
    FFreeRTOSMedia *instance_p = &os_media;
    u32 mode_id = FDcResToModeNum(width, height);

    FDcDpCfgInitialize(&instance_p->dcdp_ctrl);
    ret = FDcDpGetDefaultConfig(&instance_p->dcdp_ctrl);
    if (ret != FMEDIA_DP_SUCCESS)
    {
        FMEDIA_ERROR("Get default failed");
        goto err_exit;
    }
    ret = FDcDpSetBasicParam(&instance_p->dcdp_ctrl, width, height, color_depth, refresh_rate);
    if (ret != FMEDIA_DP_SUCCESS)
    {
        FMEDIA_ERROR("Set basic parameters failed");
        goto err_exit;
    }
    ret = FDcDpInitial(&instance_p->dcdp_ctrl, channel,width, height , mode_id, multi_mode);
    if (ret != FMEDIA_DP_SUCCESS)
    {
        FMEDIA_ERROR("DcDp initial failed");
        goto err_exit;
    }

err_exit:
    return (FT_SUCCESS == ret) ? instance_p : NULL; /* exit with NULL if failed */
}


/**
 * @name: FFreeRTOSMediaHpdReInit
 * @msg:  hpd init the media,dc and dp
 * @param  {u32} channel is the dc channel
 * @param  {u32} width is the width
 * @param  {u32} height is the height
 * @param  {u32} multi_mode is multi display mode,0:clone,1:hor,2:ver
 * @param  {u32} color_depth is the color depth
 * @param  {u32} refresh_rate is the refresh rate of screen
 * @return err code information, 0 indicates success，others indicates failed
 */
FError FFreeRTOSMediaHpdReInit(u32 channel,u32 width, u32 height,u32 multi_mode,u32 color_depth,u32 refresh_rate)
{
    FError ret = FT_SUCCESS;
    FFreeRTOSMedia *instance_p = &os_media;

    if (FDpChannelRegRead(instance_p->dcdp_ctrl.dp_instance_p[channel].config.dp_channe_base_addr, FDPTX_LINK_BW_SET) != 0)
    {
        FMEDIA_DEBUG("DP have been setted, do not need reconnected\r\n");
        instance_p->dcdp_ctrl.connect_flg[channel] = 0;
    }
    else
    {
        FMEDIA_DEBUG(" Reconnected\r\n");
         FDcDpCfgInitialize(&instance_p->dcdp_ctrl);
         ret = FDcDpGetDefaultConfig(&instance_p->dcdp_ctrl);
        if (ret != FMEDIA_DP_SUCCESS)
        {
            FMEDIA_ERROR("Get default failed");
        }
        ret = FDcDpSetBasicParam(&instance_p->dcdp_ctrl, width, height, color_depth, refresh_rate);
        if (ret != FMEDIA_DP_SUCCESS)
        {
            FMEDIA_ERROR("Set basic parameters failed");
        }
        ret = FDcDpInitial(&instance_p->dcdp_ctrl, channel, width, height, color_depth, multi_mode);
        instance_p->dcdp_ctrl.connect_flg[channel] = 0;
    }

    return ret;
}
