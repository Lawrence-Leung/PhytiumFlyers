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
 * FilePath: if_cmd.c
 * Date: 2022-10-27 16:41:15
 * LastEditTime: 2022-10-27 16:41:15
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who  Date   Changes
 * ----- ------  -------- --------------------------------------
 */
#include "ftypes.h"
#include <stdio.h>
#include <string.h>
#include "ifconfig.h"
#include "sdkconfig.h"


#if defined(CONFIG_USE_LETTER_SHELL)
#include "../src/shell.h"

static void IfCmdUsage(void)
{
    printf("usage:\r\n");
    printf(" -- if you only input ifconfig ,list the information of all network interfaces \r\n");
    printf(" -- if you input ifconfig  <netif name> mtu <mtu value>, you can change the mtu of corresponding netif \r\n");
    printf("        -- <mtu value> mtu value of netif. Attention: This value can not exceed the preset value when we initialize \r\n");
    printf(" -- if you input ifconfig <netif name> <Ip address> <Gateway> <Netmask> ,you can change the ip of corresponding netif \r\n");
    printf("        -- <netif name> lwip netif name \r\n");
    printf("        -- <Ip address> ip address of netif \r\n");
    printf("        -- <Gateway> Gateway of netif \r\n");
    printf("        -- <Netmask> Netmask of netif \r\n");
    printf("\r\n\r\n");
}

static int  SetIfCmd(int argc, char *argv[])
{

    char *netif_name = NULL;
    char * ip_addr = NULL;
    char * gw_addr = NULL;
    char * nm_addr = NULL;
    char * mtu_value = NULL;
    char * mtu_flag = NULL;
    printf("argc is %d\r\n", argc);
    switch (argc)
    {
        case 5:
            nm_addr = argv[4];
        case 4:
            gw_addr = argv[3];
            mtu_value = argv[3];
        case 3:
            ip_addr = argv[2];
            mtu_flag = argv[2];
        case 2:
            netif_name = argv[1];
            break;
        default:
            break;
    }

    if(strcmp(mtu_flag ,"mtu") == 0)
    {
        SetMtu(netif_name, mtu_value);
    }
    else
    {
        SetIf(netif_name, ip_addr, gw_addr, nm_addr);
    } 
    return 0;
}

static int if_cmd(int argc, char *argv[])
{
    if (argc <= 1)
    {
        IfCmdUsage();
        ListIf();
    }
    else
    {
         SetIfCmd(argc, argv);  
    }
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), ifconfig, if_cmd, list network interface information);

#endif
