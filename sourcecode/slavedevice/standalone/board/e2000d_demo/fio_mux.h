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
 * @FilePath: fio_mux.h
 * @Date: 2023-07-25 11:04:45
 * @LastEditTime: 2023-07-25 11:04:46
 * @Description:  This file is for io-pad function definition
 * 
 * @Modify History: 
 *  Ver   Who    Date           Changes
 * ----- ------  --------       --------------------------------------
 * 1.0   liusm   2023/7/25      first release
 */
#ifndef  FIO_MUX_H
#define  FIO_MUX_H

#include "sdkconfig.h"
#include "ftypes.h"
#ifdef __cplusplus
extern "C"
{
#endif

void FIOPadSetSpimMux(u32 spim_id);
void FIOPadSetGpioMux(u32 gpio_id, u32 pin_id);
void FIOPadSetCanMux(u32 can_id);
void FIOPadSetQspiMux(u32 qspi_id, u32 cs_id);
void FIOPadSetPwmMux(u32 pwm_id, u32 pwm_channel);
void FIOPadSetAdcMux(u32 adc_id, u32 adc_channel);
void FIOPadSetMioMux(u32 mio_id);
void FIOPadSetTachoMux(u32 pwm_in_id);
void FIOPadSetUartMux(u32 uart_id);

void FIOPadDeInit(void);
#ifdef __cplusplus
}
#endif

#endif
