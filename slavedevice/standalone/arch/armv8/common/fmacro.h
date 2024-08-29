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
 * FilePath: fmacro.h
 * Date: 2022-02-10 14:53:41
 * LastEditTime: 2022-02-17 17:35:17
 * Description:  This files is for the current execution state selects the macro definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  huanghe	2021/11/06		first release
 */


#ifndef FMACRO_H
#define FMACRO_H

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Branch according to exception level
 */
/*
 * Branch according to exception level
 */
.macro  switch_el, xreg, el3_label, el2_label, el1_label
mrs \xreg, CurrentEL
cmp \xreg, 0xc
b.eq    \el3_label
cmp \xreg, 0x8
b.eq    \el2_label
cmp \xreg, 0x4
b.eq    \el1_label
.endm

#ifdef __cplusplus
}
#endif

#endif /* __ASM_ARM_MACRO_H__ */
