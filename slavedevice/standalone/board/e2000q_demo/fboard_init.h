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
 * @FilePath: fboard_init.h
 * @Date: 2023-07-24 19:50:48
 * @LastEditTime: 2023-07-24 19:50:48
 * @Description:  This file is for 
 * 
 * @Modify History: 
 *  Ver   Who       Date         Changes
 * ----- ------  --------       --------------------------------------
 * 1.0   liusm   2023/7/24      first release
 */

#ifndef  FBOARD_INIT_H
#define  FBOARD_INIT_H

#include "sdkconfig.h"

#ifdef __cplusplus
extern "C"
{
#endif

int FBoardStatusInit(void *para);

#ifdef __cplusplus
}
#endif

#endif
