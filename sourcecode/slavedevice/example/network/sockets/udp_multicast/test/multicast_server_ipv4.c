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
 * FilePath: multicast_server_ipv4.c
 * Date: 2022-09-16 09:15:47
 * LastEditTime: 2022-09-16 09:15:47
 * Description:  This file is for multicast ipv4 server
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  huanghe     2022/10/21  init
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../sdkconfig.h"



#if defined(CONFIG_EXAMPLE_IPV4_ONLY) || defined(CONFIG_EXAMPLE_IPV4_V6)

#define GROUP_IP CONFIG_EXAMPLE_MULTICAST_IPV4_ADDR
#define TEST_PORT CONFIG_EXAMPLE_PORT


int main()
{
    // 1. 创建通信的套接字
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1)
    {
        perror("socket");
        exit(0);
    }
    printf("addr is %s \r\n", GROUP_IP);
    printf("TEST_PORT is %d \r\n", TEST_PORT);
    // 2. 通信的套接字和本地的IP与端口绑定
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(TEST_PORT);    // 大端
    addr.sin_addr.s_addr = htonl(INADDR_ANY);  // 0.0.0.0
    int ret = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        perror("bind");
        exit(0);
    }
    printf("加入到多播组 \r\n");
    // 3. 加入到多播组
    struct ip_mreq mreq; // 多播地址结构体
    mreq.imr_multiaddr.s_addr = inet_addr(GROUP_IP);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    ret = setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));

    char buf[1024];
    char sendaddrbuf[64];
    socklen_t len = sizeof(struct sockaddr_in);
    struct sockaddr_in sendaddr;
    printf("通信 \r\n");
    // 3. 通信
    while (1)
    {
        // 接收广播消息
        memset(buf, 0, sizeof(buf));
        // 阻塞等待数据达到

        recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&sendaddr, &len);
        printf("sendaddr:%s, port:%d\n", inet_ntop(AF_INET, &sendaddr.sin_addr.s_addr,  sendaddrbuf, sizeof(sendaddrbuf)),  sendaddr.sin_port);
        printf("接收到的组播消息: %s\n", buf);
        sendto(fd, buf, strlen(buf) + 1, 0, (struct sockaddr *)&sendaddr, len);
    }
    close(fd);
    return 0;
}

#else


int main()
{
    printf("The test program does not support ipv4 \n");
    return 0;
}


#endif
