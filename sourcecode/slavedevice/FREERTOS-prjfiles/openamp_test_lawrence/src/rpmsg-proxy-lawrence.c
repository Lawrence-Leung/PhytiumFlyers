/*
    rpmsg-proxy-lawrence 程序
    Lawrence Leung 2024.3.2

    运行于Slave端，提供两个endpoint：一个是proxy，另一个是message

* This is a sample demonstration application that showcases usage of proxy from the remote core.
* This application is meant to run on the remote CPU running baremetal.
* This applicationr can print to to host console and perform file I/O using proxy mechanism.

todo：
由于slave端没有自己的文件系统，因此所有的open, close, read, write目前暂时没有飞腾官方的实现。
*/

/***************************** Include Files *********************************/
#include <stdio.h>
#include <openamp/open_amp.h>
#include <metal/alloc.h>
#include "platform_info.h"
#include "rpmsg_service.h"
#include <metal/sleep.h>
#include "rsc_table.h"
#include "shell.h"
#include "fcache.h"
#include "fdebug.h"

#include "fcntl.h"	//新增
#include <unistd.h>	//新增
#include <openamp/rpmsg_retarget.h> //新增

/************************** Constant Definitions *****************************/
/***************** Macros (Inline Functions) Definitions *********************/

#define     SHUTDOWN_MSG                0xEF56A55A
#define     ECHO_DEV_SLAVE_DEBUG_TAG "PHY-SLAVE"
#define     ECHO_DEV_SLAVE_DEBUG_I(format, ...) FT_DEBUG_PRINT_I( ECHO_DEV_SLAVE_DEBUG_TAG, format, ##__VA_ARGS__)
#define     ECHO_DEV_SLAVE_DEBUG_W(format, ...) FT_DEBUG_PRINT_W( ECHO_DEV_SLAVE_DEBUG_TAG, format, ##__VA_ARGS__)
#define     ECHO_DEV_SLAVE_DEBUG_E(format, ...) FT_DEBUG_PRINT_E( ECHO_DEV_SLAVE_DEBUG_TAG, format, ##__VA_ARGS__)
#define     PROXY_BUFF_SIZE             1024    //新增 by Lawrence

/************************** Variable Definitions *****************************/
static int shutdown_req = 0;    //Shutdown请求变量
static struct metal_io_region *shbuf_io;    //共享缓存IO结构体变量，用于Proxy
static int err_cnt = 0;         //报错次数

/************************** Function Prototypes ******************************/
/*-----------------------------------------------------------------------------*
 *  功能函数
 *-----------------------------------------------------------------------------*/
//以下的几个函数都是移植的
static int copy_from_shbuf(void *dst, void *shbuf, int len)
{
	int ret;
	unsigned long offset = metal_io_virt_to_offset(shbuf_io, shbuf);

	if (offset == METAL_BAD_OFFSET) {
		ECHO_DEV_SLAVE_DEBUG_E("no offset within IO region for data ptr: %p\r\n",
			shbuf);
		return -1;
	}

	ret = metal_io_block_read(shbuf_io, offset, dst, len);
	if (ret < 0)
		ECHO_DEV_SLAVE_DEBUG_E("metal_io_block_read failed with err: %d\r\n", ret);

	return ret;
}


static int handle_open(struct rpmsg_rpc_syscall *syscall,
		       struct rpmsg_endpoint *ept)
{
	char *buf;
	struct rpmsg_rpc_syscall resp;
	int fd, ret;

	if (!syscall || !ept)
		return -1;
	buf = (char *)syscall;
	buf += sizeof(*syscall);

	/* Open remote fd */
    //todo: 没有文件系统，无法实现open, close, read, write
	fd = open(buf, syscall->args.int_field1, syscall->args.int_field2);

	/* Construct rpc response */
	resp.id = OPEN_SYSCALL_ID;
	resp.args.int_field1 = fd;
	resp.args.int_field2 = 0;	/*not used */
	resp.args.data_len = 0;	/*not used */

	/* Transmit rpc response */
	ret = rpmsg_send(ept, (void *)&resp, sizeof(resp));

	return ret > 0 ?  0 : ret;
}

static int handle_close(struct rpmsg_rpc_syscall *syscall,
			struct rpmsg_endpoint *ept)
{
	struct rpmsg_rpc_syscall resp;
	int ret;

	if (!syscall || !ept)
		return -1;
	/* Close remote fd */
	// todo
	ret = close(syscall->args.int_field1);

	/* Construct rpc response */
	resp.id = CLOSE_SYSCALL_ID;
	resp.args.int_field1 = ret;
	resp.args.int_field2 = 0;	/*not used */
	resp.args.data_len = 0;	/*not used */

	/* Transmit rpc response */
	ret = rpmsg_send(ept, &resp, sizeof(resp));

	return ret > 0 ?  0 : ret;
}

static int handle_read(struct rpmsg_rpc_syscall *syscall,
		       struct rpmsg_endpoint *ept)
{
	struct rpmsg_rpc_syscall *resp;
	unsigned char buf[PROXY_BUFF_SIZE];
	unsigned char *payload;
	int bytes_read, payload_size;
	int ret;

	if (!syscall || !ept)
		return -1;
	payload = buf + sizeof(*resp);

	/*
	 * For STD_IN read up to the buf size. Otherwise read
	 * only the size requested in in syscall->rgs.int_field2
	 */
	bytes_read = sizeof(buf) - sizeof(*resp);
	if (!syscall->args.int_field1 && syscall->args.int_field2 < bytes_read)
		bytes_read = syscall->args.int_field2;

	// todo
	bytes_read = read(syscall->args.int_field1, payload, bytes_read);

	/* Construct rpc response */
	resp = (struct rpmsg_rpc_syscall *)buf;
	resp->id = READ_SYSCALL_ID;
	resp->args.int_field1 = bytes_read;
	resp->args.int_field2 = 0;	/* not used */
	resp->args.data_len = bytes_read;

	payload_size = sizeof(*resp) +
		       ((bytes_read > 0) ? bytes_read : 0);

	/* Transmit rpc response */
	ret = rpmsg_send(ept, buf, payload_size);

	return ret > 0 ?  0 : ret;
}

static int handle_write(struct rpmsg_rpc_syscall *syscall,
			struct rpmsg_endpoint *ept)
{
	struct rpmsg_rpc_syscall resp;
	unsigned char *buf;
	int bytes_written;
	int ret;

	if (!syscall || !ept)
		return -1;
	buf = (unsigned char *)syscall;
	buf += sizeof(*syscall);
	/* Write to remote fd */
	// todo
	bytes_written = write(syscall->args.int_field1, buf,
			      syscall->args.int_field2);

	/* Construct rpc response */
	resp.id = WRITE_SYSCALL_ID;
	resp.args.int_field1 = bytes_written;
	resp.args.int_field2 = 0;	/*not used */
	resp.args.data_len = 0;	/*not used */

	/* Transmit rpc response */
	ret = rpmsg_send(ept, (void *)&resp, sizeof(resp));

	return ret > 0 ?  0 : ret;
}

static int handle_rpc(struct rpmsg_rpc_syscall *syscall,
		      struct rpmsg_endpoint *ept)
{
	int retval;

	/* Handle RPC */
	switch (syscall->id) {
	case OPEN_SYSCALL_ID:
		{
			retval = handle_open(syscall, ept);
			break;
		}
	case CLOSE_SYSCALL_ID:
		{
			retval = handle_close(syscall, ept);
			break;
		}
	case READ_SYSCALL_ID:
		{
			retval = handle_read(syscall, ept);
			break;
		}
	case WRITE_SYSCALL_ID:
		{
			retval = handle_write(syscall, ept);
			break;
		}
	case TERM_SYSCALL_ID:
		{
			ECHO_DEV_SLAVE_DEBUG_I("Received termination request\r\n");
			shutdown_req = 1;
			retval = 0;
			break;
		}
	default:
		{
			ECHO_DEV_SLAVE_DEBUG_E("Invalid RPC sys call ID: %d:%d!\r\n",
			     (int)syscall->id, (int)WRITE_SYSCALL_ID);
			retval = -1;
			break;
		}
	}

	return retval;
}
/*
补充注释：

*/

/*-----------------------------------------------------------------------------*
 *  RPMSG endpoint callbacks
 *-----------------------------------------------------------------------------*/
//message端点使用的echo功能回调函数
static int rpmsg_message_ept_cb(struct rpmsg_endpoint *ept, void *data, size_t len, uint32_t src, void *priv)
{
    (void)priv;
    (void)src;

    /* On reception of a shutdown we signal the application to terminate */
    if ((*(unsigned int *)data) == SHUTDOWN_MSG)
    {
        ECHO_DEV_SLAVE_DEBUG_I("Shutdown message is received.\r\n");
        shutdown_req = 1;
        return RPMSG_SUCCESS;
    }

    /* Send data back to master */
    if (rpmsg_send(ept, data, len) < 0)
    {
        ECHO_DEV_SLAVE_DEBUG_E("rpmsg_send failed.\r\n");
    }
    //自报家门，说出自己的src字段是什么
    ECHO_DEV_SLAVE_DEBUG_I("My Name Is %s, My Source Is %d, Hello Lawrence :)", ept->name, src);


    return RPMSG_SUCCESS;
}

//proxy端点使用的proxy功能回调函数
static int rpmsg_proxy_ept_cb(
    struct rpmsg_endpoint *ept,     //终端结构体
    void *data,
    size_t len,
    uint32_t src,
    void *priv) 
{
    (void)priv;
    (void)src;
    int ret;    //内部的ret变量
    unsigned char buf[PROXY_BUFF_SIZE]; //新增，用于协议rpc操作的syscall字段
    struct rpmsg_rpc_syscall *syscall;  //新增，注意rpc = rempte procedure call

    //数据完整性检查，检查接收到的数据长度是否小于syscall结构的大小
	if (len < (int)sizeof(*syscall)) {
		ECHO_DEV_SLAVE_DEBUG_E("Received data is less than the rpc structure: %zd\r\n",
			len);
		err_cnt++;
		return RPMSG_SUCCESS;
	}

	/* In case the shared memory is device memory
	 * E.g. For now, we only use UIO device memory in Linux.
	 */
	if (len > PROXY_BUFF_SIZE)
		len = PROXY_BUFF_SIZE;
	ret = copy_from_shbuf(buf, data, len);  //数据复制
	if (ret < 0)
		return ret;

    //将缓冲区的数据转换为rpmsg_rpc_syscall结构，并调用handle_rpc函数处理远程过程调用
    //远程过程调用操作包括: open, close, read, write, term。
	syscall = (struct rpmsg_rpc_syscall *)buf;
	if (handle_rpc(syscall, ept)) {
		ECHO_DEV_SLAVE_DEBUG_E("\nHandling remote procedure call errors:\r\n");
		ECHO_DEV_SLAVE_DEBUG_I("rpc id %d\r\n", syscall->id);
		ECHO_DEV_SLAVE_DEBUG_I("rpc int field1 %d\r\n",
		       syscall->args.int_field1);
		ECHO_DEV_SLAVE_DEBUG_I("\nrpc int field2 %d\r\n",
		       syscall->args.int_field2);
		err_cnt++;
	}

    /* 当收到Shutdown信息时，直接返回 */
    if ((*(unsigned int *)data) == SHUTDOWN_MSG)
    {
        ECHO_DEV_SLAVE_DEBUG_I("Shutdown message is received.\r\n");
        shutdown_req = 1;
        return RPMSG_SUCCESS;
    }
    
    return RPMSG_SUCCESS;
}
/*
    补充注释：
    - In case the shared memory is device memory
	- E.g. For now, we only use UIO device memory in Linux.

    这段注释是在讨论VirtIO中共享内存的一个特定情况，即共享内存是设备内存而非常规的系统内存。在这里，"设备内存"通常指的是分配给硬件设备的内存区域，这些内存区域可以被设备直接访问，但不是由常规的系统内存管理机制管理。

    注释中的“E.g. For now, we only use UIO device memory in Linux.”提供了一个具体的例子，即在Linux中，目前只使用UIO（Userspace I/O）设备内存。UIO是一种允许用户空间程序直接访问硬件设备内存的机制。它通常用于那些需要避免内核空间和用户空间之间频繁切换的情况，从而可以提高性能。

    在VirtIO的上下文中，如果共享内存是设备内存，这意味着VirtIO设备和驱动程序之间的通信可能直接在这些特殊的内存区域上进行，而不是在普通的RAM上。这样可以减少数据在不同内存区域之间的复制，从而提高效率，尤其是在高性能或实时应用中尤为重要。但这也意味着需要特别注意内存管理和同步，以确保数据的一致性和系统的稳定性。
*/

//通用的解绑定回调函数
static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
    (void)ept;
    ECHO_DEV_SLAVE_DEBUG_I("Unexpected remote endpoint destroy.\r\n");
    shutdown_req = 1;
}

/*-----------------------------------------------------------------------------*
 *  Application
 *-----------------------------------------------------------------------------*/
// 修改测试版本, by Lawrence Leung 2024.2.29
static int FRpmsgLawrenceProxy(struct rpmsg_device *rdev, void *priv)
{
    int ret = 0;
    struct rpmsg_endpoint lept0, lept1;  //测试创建3个端点，全静态。
    shutdown_req = 0;
    struct rpmsg_virtio_device *rvdev;   //这是一个VirtIO设备，用于分配Shmem，新增。

    //step 1 创建两个端点，一个用于Proxy交互，另一个用于通信。
    ECHO_DEV_SLAVE_DEBUG_I("[LAWRENCE] Try to create rpmsg endpoint [PROXY].\r\n");

    //端点0。这个端点具有shbuf，可以用于Proxy通信。
    ret = rpmsg_create_ept(&lept0, rdev, "proxy", 0, RPMSG_ADDR_ANY, rpmsg_proxy_ept_cb, rpmsg_service_unbind);
    if (ret) {
        ECHO_DEV_SLAVE_DEBUG_E("Failed to create endpoint. %d \r\n", ret);
        return -1;
    }
    rvdev = metal_container_of(rdev, struct rpmsg_virtio_device, rdev); //给定rpmsg_device指针，获取对应的rpmsg_virtio_device指针。
    shbuf_io = rvdev->shbuf_io; //从rpmsg_device获取对应的shbuf_io
    if (!shbuf_io) {
        ECHO_DEV_SLAVE_DEBUG_E("No IO region for Lawrence's proxy app. %d \r\n", ret);
        return -1;
    }

    //端点1。这个端点只能进行正常的Endpoint通信。
    ECHO_DEV_SLAVE_DEBUG_I("[LAWRENCE] Try to create rpmsg endpoint [MESSAGE].\r\n");
    ret = rpmsg_create_ept(&lept1, rdev, "message", 10, RPMSG_ADDR_ANY, rpmsg_message_ept_cb, rpmsg_service_unbind);
    if (ret) {
        ECHO_DEV_SLAVE_DEBUG_E("Failed to create endpoint. %d \r\n", ret);
        return -1;
    }
    ECHO_DEV_SLAVE_DEBUG_I("[LAWRENCE] Hello, Lawrence. You've done a good job of creating 2 endpoints with different usages.\r\n");

    //step 2 开始死循环
    while (1) {
        platform_poll(priv);
        if (shutdown_req) { //当遇到Shutdown请求时，退出循环
            break;
        }
    }

    //step 3 销毁端点，这里需要有2个。当然这是收到了shutdown_req之后才有的。
    //正常程序不会执行到这里。
    rpmsg_destroy_ept(&lept0);
    rpmsg_destroy_ept(&lept1);

    return ret;
}

/*-----------------------------------------------------------------------------*
 *  Application entry point
 *-----------------------------------------------------------------------------*/

int FOpenampProxyLawrence(void)
{
    int ret = 0;
    void *platform;
    struct rpmsg_device *rpdev;
    //step 1 初始化平台
    ret = platform_init(0, NULL, &platform);
    if (ret)
    {
        ECHO_DEV_SLAVE_DEBUG_E("Failed to initialize platform.\r\n");
        platform_cleanup(platform);
        return -1;
    }

    //step 2 创建一个vdev设备
    rpdev = platform_create_rpmsg_vdev(platform, 0, VIRTIO_DEV_SLAVE, NULL, NULL);
    if (!rpdev)
    {
        ECHO_DEV_SLAVE_DEBUG_E("Failed to create rpmsg virtio device.\r\n");
        ret = platform_cleanup(platform);
        return ret;
    }
    
    //step 3 执行应用程序，这里是一个死循环。
    ret = FRpmsgLawrenceProxy(rpdev, platform);   

    //step 4 关闭应用程序
    if (ret)
    {
        ECHO_DEV_SLAVE_DEBUG_E("Failed to running echoapp");
        return platform_cleanup(platform);
    }
    ECHO_DEV_SLAVE_DEBUG_I("Stopping application...");
    return ret;
}
