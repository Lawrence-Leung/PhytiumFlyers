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
 * FilePath: ifconfig.h
 * Date: 2022-12-13 09:47:52
 * LastEditTime: 2022-12-13 09:47:52
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who  Date   Changes
 * ----- ------  -------- --------------------------------------
 */

#ifndef _IFCONFIG_H
#define _IFCONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif

    void SetIf(char *netif_name, char *ip_addr, char *gw_addr, char *nm_addr);
    void ListIf(void);
    void SetMtu(char *netif_name, char *Mtu_value);
#ifdef __cplusplus
}
#endif


#endif
