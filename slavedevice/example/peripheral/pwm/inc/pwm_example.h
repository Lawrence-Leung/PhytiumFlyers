/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
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
 * FilePath: pwm_example.h
 * Date: 2022-08-16 14:55:40
 * LastEditTime: 2022-08-19 11:42:40
 * Description:  This file is for pwm test example function declarations.
 *
 * Modify History:
 *  Ver   Who           Date           Changes
 * ----- ------       --------      --------------------------------------
 * 1.0  wangxiaodong  2022/8/24      first release
 */


#ifndef PWM_EXAMPLE_H
#define PWM_EXAMPLE_H

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(CONFIG_TARGET_E2000D)||defined(CONFIG_TARGET_E2000Q)
#define PWM_TEST_ID FPWM6_ID
#elif defined(CONFIG_TARGET_PHYTIUMPI)
#define PWM_TEST_ID FPWM2_ID
#endif
/* pwm test */
BaseType_t FFreeRTOSPwmCreate(u32 id);

#ifdef __cplusplus
}
#endif

#endif // !