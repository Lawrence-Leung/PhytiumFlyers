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
 * FilePath: ftlb.S
 * Date: 2022-02-10 14:53:41
 * LastEditTime: 2022-02-17 17:31:30
 * Description:  This file is for tlb invalidation
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe     2021/7/3     first release
 */

#include "flinkage.h"
#include "fmacro.h"

/*
 * void AsmInvalidateTlbAll(void)
 *
 * invalidate all tlb entries.
*/
.pushsection .text.AsmInvalidateTlbAll, "ax"
ENTRY(AsmInvalidateTlbAll)
	switch_el x9, 3f, 2f, 1f
3:	tlbi	alle3
	dsb	sy
	isb
	b	0f
2:	tlbi	alle2
	dsb	sy
	isb
	b	0f
1:	tlbi	vmalle1
	dsb	sy
	isb
0:
	ret
ENDPROC(AsmInvalidateTlbAll)
.popsection
