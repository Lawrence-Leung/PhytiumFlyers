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
 * FilePath: multicast.c
 * Date: 2022-09-15 10:19:11
 * LastEditTime: 2022-09-15 10:19:11
 * Description:  This file is for running multicast example task
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  huanghe     2022/10/21  init
 * 1.1 liuzhihong   2023/01/12  driver and application restructure
 */

#include <string.h>
#include <sys/param.h>
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "err.h"

#include "netif.h"
#include "sockets.h"
#include "sockets_ext.h"
#include <netdb.h>
#include "lwip_port.h"

#include "ftypes.h"
#include "shell.h"
#include "fdebug.h"

#define MULTICAST_DEBUG_TAG "MULTICAST"
#define MULTICAST_PRINT_E(format, ...) FT_DEBUG_PRINT_E(MULTICAST_DEBUG_TAG, format, ##__VA_ARGS__)
#define MULTICAST_PRINT_I(format, ...) FT_DEBUG_PRINT_I(MULTICAST_DEBUG_TAG, format, ##__VA_ARGS__)
#define MULTICAST_PRINT_D(format, ...) FT_DEBUG_PRINT_D(MULTICAST_DEBUG_TAG, format, ##__VA_ARGS__)
#define MULTICAST_PRINT_W(format, ...) FT_DEBUG_PRINT_W(MULTICAST_DEBUG_TAG, format, ##__VA_ARGS__)


#define IPV4_DEBUG_TAG "IPV4"
#define IPV4_PRINT_E(format, ...) FT_DEBUG_PRINT_E(IPV4_DEBUG_TAG, format, ##__VA_ARGS__)
#define IPV4_PRINT_I(format, ...) FT_DEBUG_PRINT_I(IPV4_DEBUG_TAG, format, ##__VA_ARGS__)
#define IPV4_PRINT_D(format, ...) FT_DEBUG_PRINT_D(IPV4_DEBUG_TAG, format, ##__VA_ARGS__)
#define IPV4_PRINT_W(format, ...) FT_DEBUG_PRINT_W(IPV4_DEBUG_TAG, format, ##__VA_ARGS__)


#define IPV6_DEBUG_TAG "IPV6"
#define IPV6_PRINT_E(format, ...) FT_DEBUG_PRINT_E(IPV6_DEBUG_TAG, format, ##__VA_ARGS__)
#define IPV6_PRINT_I(format, ...) FT_DEBUG_PRINT_I(IPV6_DEBUG_TAG, format, ##__VA_ARGS__)
#define IPV6_PRINT_D(format, ...) FT_DEBUG_PRINT_D(IPV6_DEBUG_TAG, format, ##__VA_ARGS__)
#define IPV6_PRINT_W(format, ...) FT_DEBUG_PRINT_W(IPV6_DEBUG_TAG, format, ##__VA_ARGS__)

#define UDP_PORT CONFIG_EXAMPLE_PORT

#if defined(CONFIG_EXAMPLE_LOOPBACK)
    #define MULTICAST_LOOPBACK 1
#else
    #define MULTICAST_LOOPBACK 0
#endif

#define MULTICAST_TTL CONFIG_EXAMPLE_MULTICAST_TTL

#define MULTICAST_IPV4_ADDR CONFIG_EXAMPLE_MULTICAST_IPV4_ADDR
#define MULTICAST_IPV6_ADDR CONFIG_EXAMPLE_MULTICAST_IPV6_ADDR

#if defined(EXAMPLE_MULTICAST_LISTEN_ALL_IF)
    #define LISTEN_ALL_IF   1
#else
    #define LISTEN_ALL_IF   0
#endif

char eth_name[2] = {0};
static int multicast_sock = 0;
TaskHandle_t multicast_handle = NULL;
static struct netif *netif_p = NULL;

#ifdef CONFIG_EXAMPLE_IPV4
/* Add a socket, either IPV4-only or IPV6 dual mode, to the IPV4
   multicast group */
static int socket_add_ipv4_multicast_group(int sock, boolean assign_source_if)
{
    struct ip_mreq imreq = { 0 };
    struct in_addr iaddr = { 0 };
    int err = 0;
    /*  Configure source interface */
#if LISTEN_ALL_IF
    imreq.imr_interface.s_addr = IPADDR_ANY;
#else
    /* 多播输出接口的IP 地址 */
    ip_addr_t ipaddr;

    /* use default netif */
    extern struct netif *netif_default;
    if (netif_default == NULL)
    {
        printf("default netif not set.\r\n");
        goto err;
    }

    inet_addr_from_ip4addr(&iaddr, netif_ip4_addr(netif_default));

#endif /*  LISTEN_ALL_IF */
    /* Configure multicast address to listen to */
    err = inet_aton(MULTICAST_IPV4_ADDR, &imreq.imr_multiaddr.s_addr);
    if (err != 1)
    {
        IPV4_PRINT_E("Configured IPV4 multicast address '%s' is invalid.", MULTICAST_IPV4_ADDR);
        err = -1;
        goto err;
    }
    MULTICAST_PRINT_I("Configured IPV4 Multicast address %s", inet_ntoa(imreq.imr_multiaddr.s_addr));
    if (!IP_MULTICAST(ntohl(imreq.imr_multiaddr.s_addr)))
    {
        IPV4_PRINT_W("Configured IPV4 multicast address '%s' is not a valid multicast address. This will probably not work.", MULTICAST_IPV4_ADDR);
    }

    if (assign_source_if)
    {
        /* Assign the IPv4 multicast source interface, via its IP
         (only necessary if this socket is IPV4 only) */
        err = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, &iaddr,
                         sizeof(struct in_addr));
        if (err < 0)
        {
            IPV4_PRINT_E("Failed to set IP_MULTICAST_IF. Error %d", errno);
            goto err;
        }
    }

    err = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                     &imreq, sizeof(struct ip_mreq));
    if (err < 0)
    {
        IPV4_PRINT_E("Failed to set IP_ADD_MEMBERSHIP. Error %d", errno);
        goto err;
    }

err:
    return err;
}
#endif /* CONFIG_EXAMPLE_IPV4 */

#ifdef CONFIG_EXAMPLE_IPV6
static int create_multicast_ipv6_socket(struct netif *netif_test)
{
    struct sockaddr_in6 saddr = { 0 };
    int  netif_index;
    struct in6_addr if_inaddr = { 0 };
    struct ip6_addr if_ipaddr = { 0 };
    struct ipv6_mreq v6imreq = { 0 };
    int sock = -1;
    int err = 0;

    sock = socket(PF_INET6, SOCK_DGRAM, IPPROTO_IPV6);
    if (sock < 0)
    {
        IPV6_PRINT_E("Failed to create socket. Error %d", errno);
        return -1;
    }

    /* Bind the socket to any address */
    saddr.sin6_family = AF_INET6;
    saddr.sin6_port = htons(UDP_PORT);
    bzero(&saddr.sin6_addr.un, sizeof(saddr.sin6_addr.un));
    err = bind(sock, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in6));
    if (err < 0)
    {
        IPV6_PRINT_E("Failed to bind socket. Error %d", errno);
        goto err;
    }

    /* Selct the interface to use as multicast source for this socket. */
#if LISTEN_ALL_IF
    bzero(&if_inaddr.un, sizeof(if_inaddr.un));
#else
    char addrbuf[32] = { 0 };
    /* Read interface adapter link-local address and use it
        to bind the multicast IF to this socket.*/


    memcpy(&if_ipaddr, &netif_test->ip6_addr[0], sizeof(ip6_addr_t));

    inet6_addr_from_ip6addr(&if_inaddr, &if_ipaddr);
    inet6_ntoa_r(if_inaddr, addrbuf, sizeof(addrbuf) - 1);
    printf("addrbuf is %s .\r\n", addrbuf);
#endif /* LISTEN_ALL_IF */

    /* search for netif index */
    netif_index = netif_get_index(netif_test);
    if (netif_index < 0)
    {
        IPV6_PRINT_E("Failed to get netif index");
        goto err;
    }

    /* Assign the multicast source interface, via its IP */
    err = setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_IF, &netif_index, sizeof(uint8_t));
    if (err < 0)
    {
        IPV6_PRINT_E("Failed to set IPV6_MULTICAST_IF. Error %d", errno);
        goto err;
    }

    /* Assign multicast TTL (set separately from normal interface TTL) */
    uint8_t ttl = MULTICAST_TTL;
    setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &ttl, sizeof(uint8_t));
    if (err < 0)
    {
        IPV6_PRINT_E("Failed to set IPV6_MULTICAST_HOPS. Error %d", errno);
        goto err;
    }

#if MULTICAST_LOOPBACK
    /* select whether multicast traffic should be received by this device, too
        (if setsockopt() is not called, the default is no) */
    uint8_t loopback_val = MULTICAST_LOOPBACK;
    err = setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_LOOP,
                     &loopback_val, sizeof(uint8_t));
    if (err < 0)
    {
        IPV6_PRINT_E("Failed to set IPV6_MULTICAST_LOOP. Error %d", errno);
        goto err;
    }
#endif

    /* this is also a listening socket, so add it to the multicast */
    /* group for listening... */
#ifdef CONFIG_EXAMPLE_IPV6
    /* Configure multicast address to listen to */
    err = inet6_aton(MULTICAST_IPV6_ADDR, &v6imreq.ipv6mr_multiaddr);
    if (err != 1)
    {
        IPV6_PRINT_E("Configured IPV6 multicast address '%s' is invalid.", MULTICAST_IPV6_ADDR);
        goto err;
    }
    MULTICAST_PRINT_I("Configured IPV6 Multicast address %s", inet6_ntoa(v6imreq.ipv6mr_multiaddr));
    ip6_addr_t multi_addr;
    inet6_addr_to_ip6addr(&multi_addr, &v6imreq.ipv6mr_multiaddr);
    if (!ip6_addr_ismulticast(&multi_addr))
    {
        MULTICAST_PRINT_W("Configured IPV6 multicast address '%s' is not a valid multicast address. This will probably not work.", MULTICAST_IPV6_ADDR);
    }
    /* Configure source interface */
    v6imreq.ipv6mr_interface = (unsigned int)netif_index;
    err = setsockopt(sock, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP,
                     &v6imreq, sizeof(struct ipv6_mreq));
    if (err < 0)
    {
        IPV6_PRINT_E("Failed to set IPV6_ADD_MEMBERSHIP. Error %d", errno);
        goto err;
    }
#endif

    int only = 1; /* IPV6-only socket */

    err = setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &only, sizeof(int));
    if (err < 0)
    {
        IPV6_PRINT_E("Failed to set IPV6_V6ONLY. Error %d", errno);
        goto err;
    }

    /* All set, socket is configured for sending and receiving */
    return sock;

err:
    close(sock);
    return -1;
}
#endif

#ifdef CONFIG_EXAMPLE_IPV4_ONLY
static int create_multicast_ipv4_socket(void)
{
    struct sockaddr_in saddr = { 0 };
    int sock = -1;
    int err = 0;

    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0)
    {
        IPV4_PRINT_E("Failed to create socket. Error %d", errno);
        return -1;
    }

    /* Bind the socket to any address */
    saddr.sin_family = PF_INET;
    saddr.sin_port = htons(UDP_PORT);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    err = bind(sock, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in));
    if (err < 0)
    {
        IPV4_PRINT_E("Failed to bind socket. Error %d", errno);
        goto err;
    }


    /* Assign multicast TTL (set separately from normal interface TTL) */
    uint8_t ttl = MULTICAST_TTL;
    setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(uint8_t));
    if (err < 0)
    {
        IPV4_PRINT_E("Failed to set IP_MULTICAST_TTL. Error %d", errno);
        goto err;
    }

#if MULTICAST_LOOPBACK
    /* select whether multicast traffic should be received by this device, too */
    /* (if setsockopt() is not called, the default is no) */
    uint8_t loopback_val = MULTICAST_LOOPBACK;
    err = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP,
                     &loopback_val, sizeof(uint8_t));
    if (err < 0)
    {
        IPV4_PRINT_E("Failed to set IP_MULTICAST_LOOP. Error %d", errno);
        goto err;
    }
#endif

    /* this is also a listening socket, so add it to the multicast
     group for listening... */
    err = socket_add_ipv4_multicast_group(sock, TRUE);
    if (err < 0)
    {
        goto err;
    }

    /* All set, socket is configured for sending and receiving */
    return sock;

err:
    close(sock);
    return -1;
}
#endif /* CONFIG_EXAMPLE_IPV4_ONLY */

static void MulticastExampleTask(void *args)
{
    struct netif *netif_test = (struct netif *)args;
    while (1)
    {
        int multicast_sock;

#ifdef CONFIG_EXAMPLE_IPV4_ONLY
        multicast_sock = create_multicast_ipv4_socket();
        if (multicast_sock < 0)
        {
            MULTICAST_PRINT_E("Failed to create IPv4 multicast socket");
        }
#else
        multicast_sock = create_multicast_ipv6_socket(netif_test);
        if (multicast_sock < 0)
        {
            MULTICAST_PRINT_E("Failed to create IPv6 multicast socket");
        }
#endif

        if (multicast_sock < 0)
        {
            vTaskDelay(5 / portTICK_PERIOD_MS);
            continue;
        }

#ifdef CONFIG_EXAMPLE_IPV4
        /* set destination multicast addresses for sending from these sockets */
        struct sockaddr_in sdestv4 =
        {
            .sin_family = PF_INET,
            .sin_port = htons(UDP_PORT),
        };
        /* We know this inet_aton will pass because we did it above already */
        inet_aton(MULTICAST_IPV4_ADDR, &sdestv4.sin_addr.s_addr);
#endif

#ifdef CONFIG_EXAMPLE_IPV6
        struct sockaddr_in6 sdestv6 =
        {
            .sin6_family = PF_INET6,
            .sin6_port = htons(UDP_PORT),
        };
        /* We know this inet_aton will pass because we did it above already */
        inet6_aton(MULTICAST_IPV6_ADDR, &sdestv6.sin6_addr);
#endif

        /* Loop waiting for UDP received, and sending UDP packets if we don't
        see any. */
        int err = 1;
        while (err > 0)
        {
            struct timeval tv =
            {
                .tv_sec = 2,
                .tv_usec = 0,
            };
            fd_set rfds;
            FD_ZERO(&rfds);
            FD_SET(multicast_sock, &rfds);
            /* 等待数据接收事件 */
            int s = select(multicast_sock + 1, &rfds, NULL, NULL, &tv);
            if (s < 0)
            {
                MULTICAST_PRINT_E("Select failed: errno %d", errno);
                err = -1;
                break;
            }
            else if (s > 0)
            {
                if (FD_ISSET(multicast_sock, &rfds))
                {
                    /* Incoming datagram received */
                    char recvbuf[48];
                    char raddr_name[32] = { 0 };

                    struct sockaddr_storage raddr; /* Large enough for both IPv4 or IPv6 */
                    socklen_t socklen = sizeof(raddr);
                    int len = recvfrom(multicast_sock, recvbuf, sizeof(recvbuf) - 1, 0,
                                       (struct sockaddr *)&raddr, &socklen);
                    if (len < 0)
                    {
                        MULTICAST_PRINT_E("multicast recvfrom failed: errno %d", errno);
                        err = -1;
                        break;
                    }

                    /* Get the sender's address as a string */
#ifdef CONFIG_EXAMPLE_IPV4
                    if (raddr.ss_family == PF_INET)
                    {
                        inet_ntoa_r(((struct sockaddr_in *)&raddr)->sin_addr,
                                    raddr_name, sizeof(raddr_name) - 1);
                    }
#endif
#ifdef CONFIG_EXAMPLE_IPV6
                    if (raddr.ss_family == PF_INET6)
                    {
                        inet6_ntoa_r(((struct sockaddr_in6 *)&raddr)->sin6_addr, raddr_name, sizeof(raddr_name) - 1);
                    }
#endif
                    MULTICAST_PRINT_I("received %d bytes from %s:", len, raddr_name);

                    recvbuf[len] = 0; /* Null-terminate whatever we received and treat like a string... */
                    MULTICAST_PRINT_I("%s", recvbuf);
                }
            }
            else   /* s == 0 */
            {
                /* Timeout passed with no incoming data, so send something! */
                static int send_count;
                const char sendfmt[] = "Multicast #%d sent by Phytium\n";
                char sendbuf[48];
                char addrbuf[32] = { 0 };
                size_t len = snprintf(sendbuf, sizeof(sendbuf), sendfmt, send_count++);
                if (len > sizeof(sendbuf))
                {
                    MULTICAST_PRINT_E("Overflowed multicast sendfmt buffer!!");
                    send_count = 0;
                    err = -1;
                    break;
                }

                struct addrinfo hints =
                {
                    .ai_flags = AI_PASSIVE,
                    .ai_socktype = SOCK_DGRAM,
                };
                struct addrinfo *res;

#ifdef CONFIG_EXAMPLE_IPV4 /* Send an IPv4 multicast packet */

#ifdef CONFIG_EXAMPLE_IPV4_ONLY
                hints.ai_family = AF_INET; /* For an IPv4 socket */
#endif
                int err = getaddrinfo(CONFIG_EXAMPLE_MULTICAST_IPV4_ADDR,
                                      NULL,
                                      &hints,
                                      &res);
                if (err < 0)
                {
                    MULTICAST_PRINT_E("getaddrinfo() failed for IPV4 destination address. error: %d", err);
                    break;
                }
                if (res == 0)
                {
                    MULTICAST_PRINT_E("getaddrinfo() did not return any addresses");
                    break;
                }
#ifdef CONFIG_EXAMPLE_IPV4_ONLY
                ((struct sockaddr_in *)res->ai_addr)->sin_port = htons(UDP_PORT);
                inet_ntoa_r(((struct sockaddr_in *)res->ai_addr)->sin_addr, addrbuf, sizeof(addrbuf) - 1);
                MULTICAST_PRINT_I("Sending to IPV4 multicast address %s:%d...",  addrbuf, UDP_PORT);
#else
                ((struct sockaddr_in6 *)res->ai_addr)->sin6_port = htons(UDP_PORT);
                inet6_ntoa_r(((struct sockaddr_in6 *)res->ai_addr)->sin6_addr, addrbuf, sizeof(addrbuf) - 1);
                MULTICAST_PRINT_I("Sending to IPV6 (V4 mapped) multicast address %s port %d (%s)...",  addrbuf, UDP_PORT, CONFIG_EXAMPLE_MULTICAST_IPV4_ADDR);
#endif
                err = sendto(multicast_sock, sendbuf, len, 0, res->ai_addr, res->ai_addrlen);
                freeaddrinfo(res);
                if (err < 0)
                {
                    MULTICAST_PRINT_E("IPV4 sendto failed. errno: %d", errno);
                    break;
                }
#endif
#ifdef CONFIG_EXAMPLE_IPV6
                hints.ai_family = AF_INET6;
                hints.ai_protocol = 0;
                err = getaddrinfo(CONFIG_EXAMPLE_MULTICAST_IPV6_ADDR,
                                  NULL,
                                  &hints,
                                  &res);
                if (err < 0)
                {
                    MULTICAST_PRINT_E("getaddrinfo() failed for IPV6 destination address. error: %d", err);
                    break;
                }

                struct sockaddr_in6 *s6addr = (struct sockaddr_in6 *)res->ai_addr;
                s6addr->sin6_port = htons(UDP_PORT);
                inet6_ntoa_r(s6addr->sin6_addr, addrbuf, sizeof(addrbuf) - 1);
                MULTICAST_PRINT_I("Sending to IPV6 multicast address %s port %d...",  addrbuf, UDP_PORT);
                err = sendto(multicast_sock, sendbuf, len, 0, res->ai_addr, res->ai_addrlen);
                freeaddrinfo(res);
                if (err < 0)
                {
                    MULTICAST_PRINT_E("IPV6 sendto failed. errno: %d", errno);
                    break;
                }
#endif
            }
        }

        MULTICAST_PRINT_E("Shutting down socket and restarting...");
        shutdown(multicast_sock, 0);
        close(multicast_sock);
    }
    vTaskDelete(NULL);
}

static int MulticastMain(int argc, char *argv[])
{
    static int create_flg = 0;
    BaseType_t task_ret;

    /* prase multicast */

    if (argc > 1)
    {
        /* first find netif  */
        netif_p = LwipPortGetByName(argv[1]);
        if (netif_p == NULL)
        {
            printf("netif %s is not invalid.\r\n", argv[1]);
            return -1;
        }

        if (create_flg == 0)
        {
            /* step 1: Create multicast task */
            task_ret = xTaskCreate(&MulticastExampleTask, "mcast_task", 4096, netif_p, 5, &multicast_handle);

            if (task_ret != pdPASS)
            {
                create_flg = 0;
                MULTICAST_PRINT_E("Failed to create multicast task ");
                return -1;
            }
            create_flg++;
        }
        else
        {
            printf("Multicast task is already created,do not repeat the creation \r\n");
        }
    }
    else
    {
        printf("Please enter multicast <name> \r\n") ;
        printf("        -- netif_name is netif name \r\n");
    }

    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), multicast, MulticastMain, Testing multicast functions based on UDP);

