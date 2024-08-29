/*
 * This sample code demonstrates how to use file system of host processor
 * using proxy mechanism. Proxy service is implemented on host processor.
 * This application can print to the host console, take input from host console
 * and perform regular file I/O such as open, read, write and close.
 */

//首先这些引用是OpenAMP例程自带的
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>	//提供关于Open的函数
#include <unistd.h>	//提供关于Close、Write、Read的函数
#include <openamp/open_amp.h>
#include <openamp/rpmsg_retarget.h> 
//这意味着rpc服务和endpoint服务不能用到一起去。
#include "rsc_table.h"
#include "platform_info.h"

//下面是结合飞腾平台自行添加的引用
#include "fdebug.h"	//飞腾自己Debug的宏定义在这里出现


//大量宏定义，是OpenAMP例程的定义，而不是飞腾的
#define REDEF_O_CREAT   0000100
#define REDEF_O_EXCL    0000200
#define REDEF_O_RDONLY  0000000
#define REDEF_O_WRONLY  0000001
#define REDEF_O_RDWR    0000002
#define REDEF_O_APPEND  0002000
#define REDEF_O_ACCMODE 0000003
#define TERM_SYSCALL_ID  0x6UL

//套用了OpenAMP官方例程的宏格式，但具体所使用的函数是飞腾的函数
#define ECHO_DEV_SLAVE_DEBUG_TAG "PHY-SLAVE"    //这个是飞腾的
#define LPRINTF(format, ...) FT_DEBUG_PRINT_I( ECHO_DEV_SLAVE_DEBUG_TAG, format, ##__VA_ARGS__)
#define LPERROR(format, ...) FT_DEBUG_PRINT_E( ECHO_DEV_SLAVE_DEBUG_TAG, format, ##__VA_ARGS__)

//rpc关断
static void rpmsg_rpc_shutdown(struct rpmsg_rpc_data *rpc)
{
	(void)rpc;
	LPRINTF("RPMSG RPC is shutting down.\r\n");
}

int FRpmsgLawrenceRPC(struct rpmsg_device *rdev, void *priv)
{
	struct rpmsg_rpc_data rpc;
	struct rpmsg_rpc_syscall rpccall;
	int fd, bytes_written, bytes_read;
	char fname[] = "lawrence.txt";
	char wbuff[50];
	char rbuff[1024];
	char ubuff[50];
	float fdata;
	int idata;
	int ret;

	/* redirect I/Os */
    //1. 重定向IO
	LPRINTF("Initializating I/Os redirection...\r\n");
	ret = rpmsg_rpc_init(&rpc, rdev, "rpmsg_rpc_lawrence",
			     RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
			     priv, platform_poll, rpmsg_rpc_shutdown);
	rpmsg_set_default_rpc(&rpc);
	if (ret) {
		LPRINTF("Failed to initialize rpmsg rpc\r\n");
		return -1;
	}

    //2. 开始表演
	printf("\nRemote>Baremetal Remote Procedure Call (RPC) Demonstration\r\n");
    printf("\nI'm Lawrence Leung. It's your treat. \r\n");
	printf("\nRemote>***************************************************\r\n");

	printf("\nRemote>Rpmsg based retargetting to proxy initialized..\r\n");

	/* Remote performing file IO on Host */
	/*
	printf("\nRemote>FileIO demo ..\r\n");

	printf("\nRemote>Creating a file on host and writing to it..\r\n");
	fd = open(fname, REDEF_O_CREAT | REDEF_O_WRONLY | REDEF_O_APPEND,
		  S_IRUSR | S_IWUSR);
	printf("\nRemote>Opened file '%s' with fd = %d\r\n", fname, fd);

	sprintf(wbuff, "Content: My Name Lawrence Leung is Written to this file.");
	bytes_written = write(fd, wbuff, strlen(wbuff));
	printf("\nRemote>Wrote to fd = %d, size = %d, content = %s\r\n", fd,
	       bytes_written, wbuff);
	close(fd);
	printf("\nRemote>Closed fd = %d\r\n", fd);
	*/

	/* Remote performing file IO on Host */
	/*
	printf("\nRemote>Reading a file on host and displaying its contents..\r\n");
	fd = open(fname, REDEF_O_RDONLY, S_IRUSR | S_IWUSR);
	printf("\nRemote>Opened file '%s' with fd = %d\r\n", fname, fd);
	bytes_read = read(fd, rbuff, 1024);
	*(char *)(&rbuff[0] + bytes_read + 1) = 0;
	printf("\nRemote>Read from fd = %d, size = %d, printing contents below .. %s\r\n",
		fd, bytes_read, rbuff);
	close(fd);
	printf("\nRemote>Closed fd = %d\r\n", fd);
	*/

	while (1) {
		/* Remote performing STDIO on Host */
		printf("\nRemote>Remote firmware using scanf and printf ..\r\n");
		printf("\nRemote>Scanning user input from host..\r\n");
		printf("\nRemote>Enter name\r\n");
		ret = scanf("%s", ubuff);
		if (ret) {
			printf("\nRemote>Enter age\r\n");
			ret = scanf("%d", &idata);
			if (ret) {
				printf("\nRemote>Enter value for pi\r\n");
				ret = scanf("%f", &fdata);
				if (ret) {
					printf("\nRemote>User name = '%s'\r\n", ubuff);
					printf("\nRemote>User age = '%d'\r\n", idata);
					printf("\nRemote>User entered value of pi = '%f'\r\n", fdata);
				}
			}
		}
		if (!ret) {
			scanf("%s", ubuff);
			printf("Remote> Invalid value. Starting again....");
		} else {
			printf("\nRemote>Repeat demo ? (enter yes or no) \r\n");
			scanf("%s", ubuff);
			if ((strcmp(ubuff, "no")) && (strcmp(ubuff, "yes"))) {
				printf("\nRemote>Invalid option. Starting again....\r\n");
			} else if ((!strcmp(ubuff, "no"))) {
				printf("\nRemote>RPC retargetting quitting ...\r\n");
				break;
			}
		}
	}

	printf("\nRemote> Firmware's rpmsg-rpc-channel going down! \r\n");
	rpccall.id = TERM_SYSCALL_ID;
	(void)rpmsg_rpc_send(&rpc, &rpccall, sizeof(rpccall), NULL, 0);

	LPRINTF("Release remoteproc procedure call\r\n");
	rpmsg_rpc_release(&rpc);
	return 0;
}

/*
    程序入口
*/
int FOpenampStdioLawrence(void)
{
	void *platform;
	struct rpmsg_device *rpdev;
	int ret;

	LPRINTF("Starting application...\r\n");

	// 1. 初始化平台
	ret = platform_init(0, NULL, &platform);
	if (ret) {
		LPERROR("Failed to initialize platform.\r\n");
		ret = -1;
	} else {
        // 2. 创建vdev，注意这里用的是飞腾的VIRTIO_DEV_SLAVE定义
		rpdev = platform_create_rpmsg_vdev(platform, 0,
						   VIRTIO_DEV_SLAVE,
						   NULL, NULL);
		if (!rpdev) {
			LPERROR("Failed to create rpmsg virtio device.\r\n");
			ret = -1;
		} else {
            // 3. 开始执行正常程序。
			FRpmsgLawrenceRPC(rpdev, platform);
			platform_release_rpmsg_vdev(rpdev, platform);
			ret = 0;
		}
	}

	LPRINTF("Stopping application...\r\n");
	platform_cleanup(platform);

	return ret;
}
