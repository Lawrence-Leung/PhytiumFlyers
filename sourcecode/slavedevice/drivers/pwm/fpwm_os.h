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
 * FilePath: fpwm_os.h
 * Date: 2022-02-24 13:42:19
 * LastEditTime: 2022-03-21 16:59:58
 * Description:  This file is for providing function related definitions of pwm driver used in FreeRTOS.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2022/08/26  first commit
 */

#ifndef FPWM_OS_H
#define FPWM_OS_H

#include <FreeRTOS.h>
#include <semphr.h>
#include "ferror_code.h"
#include "fpwm.h"
#include "ftypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* freertos pwm error */
#define FREERTOS_PWM_SEM_ERROR    FT_CODE_ERR(ErrModBsp, ErrBspPwm, 10)

/* freertos pwm interrupt priority */
#define FREERTOS_PWM_IRQ_PRIORITY   IRQ_PRIORITY_VALUE_12

#define FREERTOS_PWM_CTRL_ENABLE        (1) /* enable pwm channel */
#define FREERTOS_PWM_CTRL_DISABLE       (2) /* disable pwm channel */
#define FREERTOS_PWM_CTRL_SET           (3) /* set pwm channel configuration */
#define FREERTOS_PWM_CTRL_GET           (4) /* get pwm channel configuration */
#define FREERTOS_PWM_CTRL_DB_SET        (5) /* enable pwm dead band function */
#define FREERTOS_PWM_CTRL_DB_GET        (6) /* disable pwm dead band function */
#define FREERTOS_PWM_CTRL_PULSE_SET     (7) /* set pwm pulse */


typedef struct
{
    u32 channel; /* 0-1 */
    FPwmVariableConfig pwm_cfg;
    FPwmDbVariableConfig db_cfg;
} FFreeRTOSPwmConfig;

typedef struct
{
    FPwmCtrl pwm_ctrl; /* pwm object */
    xSemaphoreHandle pwm_semaphore; /*!< pwm semaphore for resource sharing */
} FFreeRTOSPwm;

/* init freertos pwm instance */
FFreeRTOSPwm *FFreeRTOSPwmInit(u32 instance_id);

/* deinit freertos pwm instance */
FError FFreeRTOSPwmDeinit(FFreeRTOSPwm *os_pwm_p);

/* pwm channel enable or disable */
FError FFreeRTOSPwmEnable(FFreeRTOSPwm *os_pwm_p, FPwmChannel channel, boolean state);

/* set pwm channel config */
FError FFreeRTOSPwmSet(FFreeRTOSPwm *os_pwm_p, FPwmChannel channel, FPwmVariableConfig *pwm_cfg_p);

/* get pwm channel config */
FError FFreeRTOSPwmGet(FFreeRTOSPwm *os_pwm_p, FPwmChannel channel, FPwmVariableConfig *pwm_cfg_p);

/* set pwm db config */
FError FFreeRTOSPwmDbSet(FFreeRTOSPwm *os_pwm_p, FPwmDbVariableConfig *db_cfg_p);

/* get pwm db config */
FError FFreeRTOSPwmDbGet(FFreeRTOSPwm *os_pwm_p, FPwmDbVariableConfig *db_cfg_p);

/* set pwm channel pulse */
FError FFreeRTOSPwmPulseSet(FFreeRTOSPwm *os_pwm_p, FPwmChannel channel, u16 pulse);

#ifdef __cplusplus
}
#endif

#endif // !
