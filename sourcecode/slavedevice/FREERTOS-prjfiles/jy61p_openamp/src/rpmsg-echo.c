/*
    rpmsg-echo.c
    OpenAMP 通信协议源文件
    by Lawrence Leung
    2024 飞腾风驰队
*/

#include "rpmsg-echo.h"
// #include "dht11.h"
#include "jy61p.h"

// extern u8 data_frame_lock;
// extern u8 data_frame[5];    // DHT11的打包好的数据，共5个字节

extern u8 jy61pLock;
extern float fAcc[3], fGyro[3], fAngle[3];
// extern float JY61P_datapack[12];

extern float JY61P_datapack1[6];
extern float JY61P_datapack2[6];
extern float JY61P_datapack3[6];


static int shutdown_req = 0;
void *platform;
struct rpmsg_device *rpdev;

/**
 * @brief OpenAMP rpmsg 接收信息事件回调函数
 * 
 * @param ept 端点结构体指针
 * @param data 从Master收到的数据指针
 * @param len 从Master收到的数据长度
 * @param src rpmsg通信协议的数据来源位置编号
 * @param priv 私有数据
 * @return int 返回值，成功或失败
 */
static int rpmsgJY61PResponse(struct rpmsg_endpoint *ept, void *data, size_t len, uint32_t src, void *priv)
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
    // if (rpmsg_send(ept, data, len) < 0) //这个函数只是用作echo例程的，不是我们自己的例程

    if (jy61pLock == 0) {

        jy61pFunc(fAcc,fGyro,fAngle);
        f_printk("lock:%d\r\n",jy61pLock);
        // JY61PPackData(fAcc,fGyro,fAngle,JY61P_datapack);
        JY61PPackData(fAcc,fGyro,fAngle,JY61P_datapack1,JY61P_datapack2,JY61P_datapack3);
        f_printk("[JY61P] Running \r\n");
        jy61pLock = 1;
        // if (rpmsg_send(ept, JY61P_datapack, sizeof(JY61P_datapack))) {
        //     ECHO_DEV_SLAVE_DEBUG_E("rpmsg_send failed.\r\n");
        // }
        f_printk("sent:1\r\n");
        if (rpmsg_send(ept, JY61P_datapack1, sizeof(JY61P_datapack1)) < 0) {
            ECHO_DEV_SLAVE_DEBUG_E("rpmsg_send failed.\r\n");
        }
        f_printk("sent:2\r\n");
        if (rpmsg_send(ept, JY61P_datapack2, sizeof(JY61P_datapack2)) < 0) {
            ECHO_DEV_SLAVE_DEBUG_E("rpmsg_send failed.\r\n");
        }
        f_printk("sent:3\r\n");
        if (rpmsg_send(ept, JY61P_datapack3, sizeof(JY61P_datapack3)) < 0) {
            ECHO_DEV_SLAVE_DEBUG_E("rpmsg_send failed.\r\n");
        }
        jy61pLock = 0;
    } else {
		f_printk("Data Locked! \r\n");
    }


    //自报家门，说出自己的src字段是什么
    ECHO_DEV_SLAVE_DEBUG_I("[SLAVE] My Name Is %s, My Source Is %d, Hello Master.", ept->name, src);

    return RPMSG_SUCCESS;
}

/**
 * @brief 当该端点服务解除绑定时的回调函数（一般为删除端点、释放内存空间）
 * 
 * @param ept 端点指针
 */
static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
    (void)ept;
    ECHO_DEV_SLAVE_DEBUG_I("[SLAVE] Unexpected remote endpoint destroy.\r\n");
    shutdown_req = 1;
}

/**
 * @brief OpenAMP rpmsg 通信端点初始化函数
 * 
 * @param rdev rpmsg 设备结构体指针
 * @param priv 平台指针，这里作为私有数据使用
 * @return int 返回值
 */
int FRpmsgCommunication(struct rpmsg_device *rdev, void *priv)
{
    int ret = 0;
    struct rpmsg_endpoint lept1, lept2;  //测试创建2个端点，全静态。
    shutdown_req = 0;
    //初始化OpenAMP的rpmsg通信框架
    f_printk("[OPENAMP] Start to create endpoints. \r\n");

    //端点1：JY61P
    taskENTER_CRITICAL();
    ret = rpmsg_create_ept(
        &lept1, // rpmsg 端点结构体指针
        rpdev, // rpmsg device 结构体指针
        "jy61p", // 端点名称
        10, // 端点起始地址
        RPMSG_ADDR_ANY,  // 端点目的地址
        rpmsgJY61PResponse,  // 当该端点接收信息时的回调函数
        rpmsg_service_unbind    // 当该端点服务解除绑定时的回调函数（一般为删除端点、释放内存空间）
        );
    if (ret)
    {
        ECHO_DEV_SLAVE_DEBUG_E("Failed to create endpoint. %d \r\n", ret);
        return -1;
    }
    f_printk("[OPENAMP] rpmsg_create_ept 001 finished. \r\n");


    taskEXIT_CRITICAL();
    
    // ret = rpmsg_create_ept(
    //     &lept1, // rpmsg 端点结构体指针
    //     rdev, // rpmsg device 结构体指针
    //     "dht11", // 端点名称
    //     10, // 端点起始地址
    //     RPMSG_ADDR_ANY,  // 端点目的地址
    //     rpmsgDHT11Response,  // 当该端点接收信息时的回调函数
    //     rpmsg_service_unbind    // 当该端点服务解除绑定时的回调函数（一般为删除端点、释放内存空间）
    //     );
    
    // f_printk("[OPENAMP] rpmsg_create_ept 001 finished. \r\n");
    // taskEXIT_CRITICAL();
    
    // if (ret)
    // {
    //     ECHO_DEV_SLAVE_DEBUG_E("Failed to create endpoint. %d \r\n", ret);
    //     return -1;
    // }

    //端点2：预留给GPS传感器，如果有需要，把下面的内容取消注释掉就好
    /*
    ECHO_DEV_SLAVE_DEBUG_I("[SLAVE] Try to create rpmsg endpoint [2].\r\n");
    taskENTER_CRITICAL();
    ret = rpmsg_create_ept(&lept2, rdev, RPMSG_SERVICE_NAME, 20, RPMSG_ADDR_ANY, rpmsg_endpoint_cb, rpmsg_service_unbind);
    taskEXIT_CRITICAL();
    if (ret)
    {
        ECHO_DEV_SLAVE_DEBUG_E("Failed to create endpoint. %d \r\n", ret);
        return -1;
    }
    */

    //备注：按照同样的方法可以创建为单点测距传感器的留下来的通道。

    ECHO_DEV_SLAVE_DEBUG_I("Successfully created rpmsg endpoint.\r\n");

    while (1)
    {
        platform_poll(platform);

        //f_printk("[OPENAMP] Running\r\n");
        // 当收到关闭请求时，退出程序
        if (shutdown_req)
        {
            break;
        }
        (pdMS_TO_TICKS(100));
    }

    //销毁端点。当然这是收到了shutdown_req之后才有的
    taskENTER_CRITICAL();
    rpmsg_destroy_ept(&lept1);
    // rpmsg_destroy_ept(&lept2);   //如果需要启用GPS传感器，取消注释掉这个就好
    taskEXIT_CRITICAL();

    return ret;
}

/**
 * @brief OpenAMP rpmsg通信初始化函数
 * 
 * @return int 返回值，退出OpenAMP程序
 */
int FOpenampInit(void)
{
    int ret = 0;

    //初始化平台

    taskENTER_CRITICAL();
    ret = platform_init(0, NULL, &platform);
    f_printk("[OPENAMP] platform_init finished. \r\n");
    taskEXIT_CRITICAL();
    if (ret)
    {
        ECHO_DEV_SLAVE_DEBUG_E("Failed to initialize platform.\r\n");
        platform_cleanup(platform);
        return -1;
    }

    taskENTER_CRITICAL();
    rpdev = platform_create_rpmsg_vdev(platform, 0, VIRTIO_DEV_SLAVE, NULL, NULL);
    f_printk("[OPENAMP] platform_create_rpmsg_vdev finished. \r\n");
    taskEXIT_CRITICAL();
    if (!rpdev)
    {
        ECHO_DEV_SLAVE_DEBUG_E("Failed to create rpmsg virtio device.\r\n");
        platform_cleanup(platform);
        return -1;
    }
    f_printk("[OPENAMP] entering FRpmsgCommunication. \r\n");
    return ret;
}

/**
 * @brief OpenAMP 通信入口函数
 * 
 * @return BaseType_t 返回值
 */
BaseType_t OpenAmpTask(void)
{
    BaseType_t ret;
    f_printk("[SLAVE] Start Initializing OpenAMP rpmsg Task \r\n");

    ret = xTaskCreate(
        (TaskFunction_t)FRpmsgCommunication, 
        (const char *)"OpenAMPTask", 
        (uint16_t)(4096 * 2),
        (void *)NULL,
        (UBaseType_t)10,
        NULL);
    if (ret != pdPASS) {
        f_printk("[SLAVE] Failed to create a rpmsg_echo task \r\n");
    }
    return ret;
}

/**
 * @brief OpenAMP版本号输出
 * 
 */
void OpenAmpVersionInfo (void) 
{
    // OPENAMP_MAIN_DEBUG_I("[SLAVE] Complier %s ,%s \r\n", __DATE__, __TIME__);
    // OPENAMP_MAIN_DEBUG_I("[SLAVE] OpenAMP lib version: %s (", openamp_version());
    // OPENAMP_MAIN_DEBUG_I("[SLAVE] OpenAMP major: %d, ", openamp_version_major());
    // OPENAMP_MAIN_DEBUG_I("[SLAVE] OpenAMP minor: %d, ", openamp_version_minor());
    // OPENAMP_MAIN_DEBUG_I("[SLAVE] OpenAMP patch: %d)\r\n", openamp_version_patch());

    // OPENAMP_MAIN_DEBUG_I("[SLAVE] Libmetal lib version: %s (", metal_ver());
    // OPENAMP_MAIN_DEBUG_I("[SLAVE] Libmetal major: %d, ", metal_ver_major());
    // OPENAMP_MAIN_DEBUG_I("[SLAVE] Libmetal minor: %d, ", metal_ver_minor());
    // OPENAMP_MAIN_DEBUG_I("[SLAVE] Libmetal patch: %d)\r\n", metal_ver_patch());
}