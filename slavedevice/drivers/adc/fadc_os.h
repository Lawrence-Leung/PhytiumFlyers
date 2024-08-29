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
 * FilePath: fadc_os.h
 * Date: 2022-08-24 16:42:19
 * LastEditTime: 2022-08-26 17:59:12
 * Description:  This file is for providing function related definitions of adc driver used in FreeRTOS.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2022/08/31  first commit
 * 1.1 wangxiaodong 2022/11/01  file name adaptation
 */

#ifndef FADC_OS_H
#define FADC_OS_H

#include <FreeRTOS.h>
#include <semphr.h>
#include "ferror_code.h"
#include "fadc.h"
#include "ftypes.h"
#include "fparameters.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* freertos adc error */
#define FREERTOS_ADC_SEM_ERROR    FT_CODE_ERR(ErrModBsp, ErrBspAdc, 10)

/* freertos adc interrupt priority */
#define FREERTOS_ADC_IRQ_PRIORITY   IRQ_PRIORITY_VALUE_12

typedef struct
{
    FAdcChannel channel;
    u16 value;
    FAdcConvertConfig convert_config; /* adc convert config */
    FAdcThresholdConfig threshold_config; /* adc channel threshold config */
    FAdcIntrEventType event_type; /* adc interrupt event type */
} FFreeRTOSAdcConfig;

typedef struct
{
    FAdcCtrl adc_ctrl; /* adc object */
    xSemaphoreHandle adc_semaphore; /*!< adc semaphore for resource sharing */
} FFreeRTOSAdc;

/* init freertos adc instance */
FFreeRTOSAdc *FFreeRTOSAdcInit(u32 instance_id);

/* deinit freertos adc instance */
FError FFreeRTOSAdcDeinit(FFreeRTOSAdc *os_adc_p);

/* adc config */
FError FFreeRTOSAdcSet(FFreeRTOSAdc *os_adc_p, FFreeRTOSAdcConfig *adc_cfg_p);

/* adc channel convert result read */
FError FFreeRTOSAdcRead(FFreeRTOSAdc *os_adc_p, FAdcChannel channel, u16 *val);

#ifdef __cplusplus
}
#endif

#endif // !
