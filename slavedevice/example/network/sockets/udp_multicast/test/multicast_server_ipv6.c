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
 * FilePath: multicast_server_ipv6.c
 * Date: 2022-09-23 14:28:25
 * LastEditTime: 2022-09-23 14:28:25
 * Description:  This file is for multicast ipv4 server
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  huanghe     2022/10/21  init
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include "../sdkconfig.h"

#if defined(CONFIG_EXAMPLE_IPV4_V6)

int main()
{
    //创建套接字
    int fd = socket(AF_INET6, SOCK_DGRAM, 0);
    printf("multicast address is %s ,port is %d \r\n", CONFIG_EXAMPLE_MULTICAST_IPV6_ADDR, CONFIG_EXAMPLE_PORT);
    //绑定本地网络信息
    struct sockaddr_in6 address = {AF_INET6, htons(CONFIG_EXAMPLE_PORT)};
    bind(fd, (struct sockaddr *)&address, sizeof address);

    //ipv6_mreq结构提供了用于IPv6地址的多播组的信息。
    struct ipv6_mreq group;
    //将接口索引指定为0，则使用默认的多播接口。
    group.ipv6mr_interface = 0;
    //IPv6组播组的地址。
    inet_pton(AF_INET6, CONFIG_EXAMPLE_MULTICAST_IPV6_ADDR, &group.ipv6mr_multiaddr);
    //将套接字加入到指定接口上提供的多播组。此选项仅对数据报和原始套接字有效（套接字类>型必须为SOCK_DGRAM或SOCK_RAW）。
    setsockopt(fd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, &group, sizeof(group));

    char buf[1024];
    char sendaddrbuf[64];
    socklen_t len = sizeof(struct sockaddr_in);
    struct sockaddr_in sendaddr;
    // 3. 通信
    while (1)
    {
        // 接收广播消息
        memset(buf, 0, sizeof(buf));
        // 阻塞等待数据达到
        recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&sendaddr, &len);
        printf("sendaddr:%s, port:%d\n", inet_ntop(AF_INET6, &sendaddr.sin_addr.s_addr,  sendaddrbuf, sizeof(sendaddrbuf)),  sendaddr.sin_port);
        printf("接收到的组播消息: %s\n", buf);
        sendto(fd, buf, strlen(buf) + 1, 0, (struct sockaddr *)&sendaddr, len);
    }
    close(fd);
    return 0;

}

#else

int main()
{
    printf("The test program does not support ipv6 \n");
    return 0;
}

#endif
