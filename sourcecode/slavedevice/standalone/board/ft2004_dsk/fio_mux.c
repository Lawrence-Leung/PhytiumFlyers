/*
 * @Copyright : (C) 2022 Phytium Information Technology, Inc. 
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
 * @FilePath: fio_mux.c
 * @Date: 2023-08-01 17:07:29
 * @LastEditTime: 2023-08-01 17:07:29
 * @Description:  This file is for 
 * 
 * @Modify History: 
 *  Ver   Who    Date           Changes
 * ----- ------  --------       --------------------------------------
 * 1.0   liusm   2023/08/01      first release
 */
#include "fio_mux.h"
#include "fioctrl_hw.h"
#include "fioctrl.h"
#include "fparameters.h"

#include "fassert.h"
#include "fdebug.h"

static FIOCtrl ioctrl;
#define FBOARD_IO_DEBUG_TAG "FBOARD_IO-CFG"
#define FBOARD_IO_ERROR(format, ...)   FT_DEBUG_PRINT_E(FBOARD_IO_DEBUG_TAG, format, ##__VA_ARGS__)
/*****************************************************************************/

/**
 * @name: FIOPadSetSpimMux
 * @msg: set iopad mux for spim cs
 * @return {*}
 * @param {u32} spim_id, instance id of spi
 */
void FIOPadSetSpimMux(u32 spim_id)
{
    FIOCtrlCfgInitialize(&ioctrl, FIOCtrlLookupConfig(FIOCTRL0_ID));
    if (spim_id == FSPI0_ID)
    {
        FIOCtrlSetFunc(&ioctrl, FIOCTRL_SPI0_CSN0_PAD, FIOCTRL_FUNC1); /* work as gpio */
    }
}

void FIOPadSetI2cMux(u32 i2c_id)
{
    FIOCtrlCfgInitialize(&ioctrl, FIOCtrlLookupConfig(FIOCTRL0_ID));

    switch (i2c_id)
    {
        case FI2C0_ID:
            FIOCtrlSetFunc(&ioctrl, FIOCTRL_I2C0_SCL_PAD, FIOCTRL_FUNC0);
            FIOCtrlSetFunc(&ioctrl, FIOCTRL_I2C0_SDA_PAD, FIOCTRL_FUNC0);
            break;
        case FI2C1_ID:
            FIOCtrlSetFunc(&ioctrl, FIOCTRL_ALL_PLL_LOCK_PAD, FIOCTRL_FUNC2);
            FIOCtrlSetFunc(&ioctrl, FIOCTRL_CRU_CLK_OBV_PAD, FIOCTRL_FUNC2);
            break;
        case FI2C2_ID:
            FIOCtrlSetFunc(&ioctrl, FIOCTRL_SWDO_SWJ_PAD, FIOCTRL_FUNC2);
            FIOCtrlSetFunc(&ioctrl, FIOCTRL_TDO_SWJ_IN_PAD, FIOCTRL_FUNC2);
            break;
        case FI2C3_ID:
            FIOCtrlSetFunc(&ioctrl, FIOCTRL_HDT_MB_DONE_STATE_PAD, FIOCTRL_FUNC2);
            FIOCtrlSetFunc(&ioctrl, FIOCTRL_HDT_MB_FAIL_STATE_PAD, FIOCTRL_FUNC2);
            break;
        default:
            FASSERT(0);
            break;
    }
}

void FIOPadSetCanMux(u32 can_id)
{
    FIOCtrlCfgInitialize(&ioctrl, FIOCtrlLookupConfig(FIOCTRL0_ID));

    if (can_id == FCAN0_ID)
    {
        FIOCtrlSetFunc(&ioctrl, FIOCTRL_TJTAG_TDI_PAD, FIOCTRL_FUNC1); /* can0-tx: func 1 */
        FIOCtrlSetFunc(&ioctrl, FIOCTRL_SWDITMS_SWJ_PAD, FIOCTRL_FUNC1); /* can0-rx: func 1 */
    }
    else if (can_id == FCAN1_ID)
    {
        FIOCtrlSetFunc(&ioctrl, FIOCTRL_NTRST_SWJ_PAD, FIOCTRL_FUNC1); /* can1-tx: func 1 */
        FIOCtrlSetFunc(&ioctrl, FIOCTRL_SWDO_SWJ_PAD, FIOCTRL_FUNC1); /* can1-rx: func 1 */
    }
    else if (can_id == FCAN2_ID)
    {

    }
    else
    {
        FBOARD_IO_ERROR("Can id error");
    }
}

void FIOPadDeInit(void)
{
    FIOCtrlDeInitialize(&ioctrl);
}
