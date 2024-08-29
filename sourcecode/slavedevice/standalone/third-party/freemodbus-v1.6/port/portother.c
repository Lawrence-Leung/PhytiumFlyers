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
 * FilePath: portother.c
 * Date: 2022-10-13 11:27:55
 * LastEditTime: 2022-10-13 11:27:55
 * Description:  This file is for port hardware close
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  liushengming 2022/09/29    first commit
 */
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

void FModBusPortClose( void )
{
    xMBPortSerialClose(  );
    xMBPortTimersClose(  );
}
