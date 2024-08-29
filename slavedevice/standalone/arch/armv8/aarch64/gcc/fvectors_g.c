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
 * FilePath: fvectors_g.c
 * Date: 2022-02-10 14:53:41
 * LastEditTime: 2022-02-17 17:31:39
 * Description:  This file is for fpu stack pointer
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe     2021/7/3     first release
 */

#include "ftypes.h"


extern u8 _fpu_stack_end[];


volatile u8 *fpu_context = _fpu_stack_end ;
volatile u8 *fpu_context_base ;
volatile u8 fpu_status ;
