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
 * FilePath: fspim_sfud_core.h
 * Date: 2022-02-10 14:53:44
 * LastEditTime: 2022-02-25 11:46:52
 * Description:  This file is for providing qspi based on sfud api;
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu    2021/12/10   first commit
 * 1.0.1 wangxiaodong 2022/12/1    parameter naming change
 */


#ifndef FSPIM_SFUD_H
#define FSPIM_SFUD_H 

#include "sfud.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "fsleep.h"
#include "fdebug.h"
#include "fspim.h"


#ifdef __cplusplus
extern "C"
{
#endif

sfud_err FSpimProbe(sfud_flash *flash);


#ifdef __cplusplus
}
#endif


#endif  

