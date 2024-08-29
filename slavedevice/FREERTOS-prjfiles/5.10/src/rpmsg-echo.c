/*
    rpmsg-echo.c
    OpenAMP 通信协议源文件
    2024 飞腾风驰队
*/

#include "rpmsg-echo.h"
#include "dht11.h"
// #include "uart.h"
#include "gps.h"
#include "syn6288.h"
#include "jy61p.h"
#include "uart.h"
#include "speech_i2c.h"

/* 变量定义部分 */
/* 1. DHT11 传感器变量 */
extern u8 humidity;		// 从DHT11提取的湿度
extern u8 temperature;	// 从DHT11提取的温度
extern u8 data_frame_lock;
extern u8 data_frame[5];    // DHT11的打包好的数据，共5个字节

/* 2. GPS模块变量 */
extern double latitude;
extern double longitude;
extern u8 GPS_data_frame_lock;	 // 打包好的数据使用的锁
extern u8 GPS_datapack[14];

/* 3. 陀螺仪传感器变量 */
extern u8 jy61pLock;
extern float fAcc[3], fGyro[3], fAngle[3];
// extern float JY61P_datapack[12];
extern float JY61P_datapack1[6];
extern float JY61P_datapack2[6];
extern float JY61P_datapack3[6];

/* 4. OpenAMP rpmsg 通信协议相关变量 */
static int shutdown_req = 0;
void *platform;
struct rpmsg_device *rpdev;

/**
 * @brief OpenAMP rpmsg 接收信息事件回调函数：DHT11传感器
 * 
 * @param ept 端点结构体指针
 * @param data 从Master收到的数据指针
 * @param len 从Master收到的数据长度
 * @param src rpmsg通信协议的数据来源位置编号
 * @param priv 私有数据
 * @return int 返回值，成功或失败
 */
static int rpmsgDHT11Response(struct rpmsg_endpoint *ept, void *data, size_t len, uint32_t src, void *priv)
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
    //if (rpmsg_send(ept, data, len) < 0) //这个函数只是用作echo例程的，不是我们自己的例程

    if (data_frame_lock == 0) {

        DHT11ReadData (&temperature, &humidity);
		DHT11PackData (&temperature, &humidity, data_frame);
        f_printk("[DHT11] Running \r\n");
        data_frame_lock = 1;
        if (rpmsg_send(ept, data_frame, sizeof(data_frame))) {
            ECHO_DEV_SLAVE_DEBUG_E("rpmsg_send failed.\r\n");
        }
        data_frame_lock = 0;
    } else {
		f_printk("Data Locked! \r\n");
    }

    //自报家门，说出自己的src字段是什么
    ECHO_DEV_SLAVE_DEBUG_I("[SLAVE] My Name Is %s, My Source Is %d, Hello Master.", ept->name, src);

    return RPMSG_SUCCESS;
}

/**
 * @brief OpenAMP rpmsg 接收信息事件回调函数：DHT11传感器
 * 
 * @param ept 端点结构体指针
 * @param data 从Master收到的数据指针
 * @param len 从Master收到的数据长度
 * @param src rpmsg通信协议的数据来源位置编号
 * @param priv 私有数据
 * @return int 返回值，成功或失败
 */
static int rpmsgGPSResponse(struct rpmsg_endpoint *ept, void *data, size_t len, uint32_t src, void *priv)
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
    //if (rpmsg_send(ept, data, len) < 0) //这个函数只是用作echo例程的，不是我们自己的例程

    if (GPS_data_frame_lock == 0) {

        UartWaitLoop();
        f_printk("[GPS] Finish Loop\r\n");
        GPSPackData(&latitude,&longitude,GPS_datapack);

        f_printk("[GPS] Finish Pack\r\n");
        GPS_data_frame_lock = 1;
        if (rpmsg_send(ept, GPS_datapack, sizeof(GPS_datapack))) {
            ECHO_DEV_SLAVE_DEBUG_E("rpmsg_send failed.\r\n");
        }
        GPS_data_frame_lock = 0;
		f_printk("Data Locked! \r\n");
    }

    //自报家门，说出自己的src字段是什么
    ECHO_DEV_SLAVE_DEBUG_I("[SLAVE] My Name Is %s, My Source Is %d, Hello Master.", ept->name, src);

    return RPMSG_SUCCESS;
}

/**
 * @brief OpenAMP rpmsg 接收信息事件回调函数：SYN6288语音模块
 * 
 * @param ept 端点结构体指针
 * @param data 从Master收到的数据指针
 * @param len 从Master收到的数据长度
 * @param src rpmsg通信协议的数据来源位置编号
 * @param priv 私有数据
 * @return int 返回值，成功或失败
 */
static int rpmsgSYN6288Response(struct rpmsg_endpoint *ept, void *data, size_t len, uint32_t src, void *priv)
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

    /*播放收到的信息*/
    f_printk("%s\r\n",data);
    // syn6288Play();
    syn6288Play(data);

    //自报家门，说出自己的src字段是什么
    ECHO_DEV_SLAVE_DEBUG_I("[SLAVE] My Name Is %s, My Source Is %d, Hello Master.", ept->name, src);

    return RPMSG_SUCCESS;
}

/**
 * @brief OpenAMP rpmsg 接收信息事件回调函数：SYN6288语音模块
 * 
 * @param ept 端点结构体指针
 * @param data 从Master收到的数据指针
 * @param len 从Master收到的数据长度
 * @param src rpmsg通信协议的数据来源位置编号
 * @param priv 私有数据
 * @return int 返回值，成功或失败
 */
static int rpmsgSPEECHResponse(struct rpmsg_endpoint *ept, void *data, size_t len, uint32_t src, void *priv)
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

    /*播放收到的信息*/
    f_printk("%s\r\n",data);

    speech_text(data,GBK);

    //自报家门，说出自己的src字段是什么
    ECHO_DEV_SLAVE_DEBUG_I("[SLAVE] My Name Is %s, My Source Is %d, Hello Master.", ept->name, src);

    return RPMSG_SUCCESS;
}

/**
 * @brief OpenAMP rpmsg 接收信息事件回调函数：JY61P陀螺仪传感器
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
    if (jy61pLock == 0) {

        jy61pFunc(fAcc,fGyro,fAngle);
        f_printk("lock:%d\r\n",jy61pLock);
        JY61PPackData(fAcc,fGyro,fAngle,JY61P_datapack1,JY61P_datapack2,JY61P_datapack3);
        f_printk("[JY61P] Running \r\n");
        jy61pLock = 1;
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
int FRpmsgCommunication(void) //struct rpmsg_device *rdev, void *priv)
{
    int ret = 0;
    struct rpmsg_endpoint lept1, lept2, lept3, lept4;  //测试创建4个端点，全静态。
    shutdown_req = 0;
    //初始化OpenAMP的rpmsg通信框架
    f_printk("[OPENAMP] Start to create endpoints. \r\n");

    // 端点1：预留给DHT11
    taskENTER_CRITICAL();
    ret = rpmsg_create_ept(
        &lept1, // rpmsg 端点结构体指针
        rpdev, // rpmsg device 结构体指针
        "dht11", // 端点名称
        10, // 端点起始地址
        RPMSG_ADDR_ANY,  // 端点目的地址
        rpmsgDHT11Response,  // 当该端点接收信息时的回调函数
        rpmsg_service_unbind    // 当该端点服务解除绑定时的回调函数（一般为删除端点、释放内存空间）
        );
    if (ret) {
        ECHO_DEV_SLAVE_DEBUG_E("Failed to create endpoint. %d \r\n", ret);
        return -1;
    }
    f_printk("[OPENAMP] rpmsg_create_ept 001 finished. \r\n");
    taskEXIT_CRITICAL();

    // 端点2：预留给GPS传感器，如果有需要，把下面的内容取消注释掉就好
    ECHO_DEV_SLAVE_DEBUG_I("[SLAVE] Try to create rpmsg endpoint [2].\r\n");
    taskENTER_CRITICAL();
    ret = rpmsg_create_ept(&lept2, rpdev, "gps", 20, RPMSG_ADDR_ANY, rpmsgGPSResponse, rpmsg_service_unbind);
    taskEXIT_CRITICAL();
    if (ret)
    {
        ECHO_DEV_SLAVE_DEBUG_E("Failed to create endpoint. %d \r\n", ret);
        return -1;
    }
    f_printk("[OPENAMP] rpmsg_create_ept 002 finished. \r\n");

    // 端点3：SYN6288 语音模块
    ECHO_DEV_SLAVE_DEBUG_I("[SLAVE] Try to create rpmsg endpoint [2].\r\n");
    taskENTER_CRITICAL();
    ret = rpmsg_create_ept(&lept3, rpdev, "speech", 30, RPMSG_ADDR_ANY, rpmsgSPEECHResponse, rpmsg_service_unbind);
    taskEXIT_CRITICAL();
    if (ret)
    {
        ECHO_DEV_SLAVE_DEBUG_E("Failed to create endpoint. %d \r\n", ret);
        return -1;
    }
    f_printk("[OPENAMP] rpmsg_create_ept 003 finished. \r\n");

    // 端点4 : JY61P
    ECHO_DEV_SLAVE_DEBUG_I("[SLAVE] Try to create rpmsg endpoint [2].\r\n");
    taskENTER_CRITICAL();
    ret = rpmsg_create_ept(&lept4, rpdev, "jy61p", 40, RPMSG_ADDR_ANY, rpmsgJY61PResponse, rpmsg_service_unbind);
    taskEXIT_CRITICAL();
    if (ret)
    {
        ECHO_DEV_SLAVE_DEBUG_E("Failed to create endpoint. %d \r\n", ret);
        return -1;
    }
    f_printk("[OPENAMP] rpmsg_create_ept 004 finished. \r\n");
    //备注：按照同样的方法可以创建为单点测距传感器的留下来的通道。
    ECHO_DEV_SLAVE_DEBUG_I("Successfully created rpmsg endpoint.\r\n");

    while (1)   // 死循环
    {
        PlatformPollTasklette(platform);    //新的Platform Poll 函数，支持非阻塞操作
        if (shutdown_req) { // 当收到关闭请求时，退出程序
            break;
        }
    }

    //销毁端点。当然这是收到了shutdown_req之后才有的
    taskENTER_CRITICAL();
    rpmsg_destroy_ept(&lept1);
    rpmsg_destroy_ept(&lept2);   
    rpmsg_destroy_ept(&lept3);
    rpmsg_destroy_ept(&lept4);   
    taskEXIT_CRITICAL();

    return ret;
}

/**
 * @brief OpenAMP rpmsg通信初始化函数
 * 
 * @return int 返回值，退出OpenAMP程序
 */
int FOpenampInit(void)  //(void *platform, struct rpmsg_device *rpdev)
{
    int ret = 0;

    // 步骤1：初始化平台
    taskENTER_CRITICAL();
    ret = platform_init(0, NULL, &platform);
    f_printk("[OPENAMP] platform_init finished. \r\n");
    taskEXIT_CRITICAL();
    if (ret) {
        ECHO_DEV_SLAVE_DEBUG_E("Failed to initialize platform.\r\n");
        platform_cleanup(platform);
        return -1;
    }

    // 步骤2：创建rpmsg vdev虚拟设备
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
 * @brief 输出相关信息
 * 
 */
void RTOSDebugOutputOnly (void) {
    f_printk("[DEBUG] Hello, World, Start debug info output. \r\n");
    vTaskDelay(500 / portTICK_PERIOD_MS);

    while (1) {
        f_printk("[DEBUG] Hello, World! This is a debugging Information. \r\n");
        vTaskDelay(250 / portTICK_PERIOD_MS);
    }
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

    // 创建任务1
    ret = xTaskCreate(
        (TaskFunction_t)FRpmsgCommunication, 
        (const char *)"OpenAMPTask", 
        (uint16_t)(4096 * 2),
        (void *)NULL,
        (UBaseType_t)10,
        NULL);
    if (ret != pdPASS) {
        f_printk("[SLAVE] Failed to create a rpmsg_echo task \r\n");
        return ret;
    }

    // 创建任务2
    ret = xTaskCreate(
        (TaskFunction_t)RTOSDebugOutputOnly,
        (const char *)"DebugTask",
        (uint16_t)(1024),
        (void *)NULL,
        (UBaseType_t)12,    //相比之下设置更高的优先级
        NULL);
    if (ret != pdPASS) {
        f_printk("[DEBUG] Failed to create DebugTask task \r\n");
        return ret;
    }


	taskENTER_CRITICAL();	//需要保障无法被干扰的状态
    ret = xTaskCreate((TaskFunction_t)DHT11CircularRead,  /* task entry */
                      (const char *)"DHT11Task",/* task name */
                      (uint16_t)1024,  /* task stack size in words */
                      (void *)NULL, /* task params */
                      (UBaseType_t)12,  /* task priority */
                      NULL); /* task handler */
	if (ret != pdPASS) {
		f_printk("[SLAVE] Create DHT11 task failed. \r\n");
        return ret;
	}
	taskEXIT_CRITICAL();

    taskENTER_CRITICAL();	//需要保障无法被干扰的状态
    ret = xTaskCreate((TaskFunction_t)DHT11CircularRead,  /* task entry */
                      (const char *)"DHT11Task",/* task name */
                      (uint16_t)1024,  /* task stack size in words */
                      (void *)NULL, /* task params */
                      (UBaseType_t)3,  /* task priority */
                      NULL); /* task handler */
	if (ret != pdPASS) {
		f_printk("[SLAVE] Create DHT11 task failed. \r\n");
        return ret;
	}
	taskEXIT_CRITICAL();


    return ret;
}

/**
 * @brief OpenAMP版本号输出
 * 
 */
void OpenAmpVersionInfo (void) 
{
    OPENAMP_MAIN_DEBUG_I("[SLAVE] Complier %s ,%s \r\n", __DATE__, __TIME__);
    OPENAMP_MAIN_DEBUG_I("[SLAVE] OpenAMP lib version: %s (", openamp_version());
    OPENAMP_MAIN_DEBUG_I("[SLAVE] OpenAMP major: %d, ", openamp_version_major());
    OPENAMP_MAIN_DEBUG_I("[SLAVE] OpenAMP minor: %d, ", openamp_version_minor());
    OPENAMP_MAIN_DEBUG_I("[SLAVE] OpenAMP patch: %d)\r\n", openamp_version_patch());

    OPENAMP_MAIN_DEBUG_I("[SLAVE] Libmetal lib version: %s (", metal_ver());
    OPENAMP_MAIN_DEBUG_I("[SLAVE] Libmetal major: %d, ", metal_ver_major());
    OPENAMP_MAIN_DEBUG_I("[SLAVE] Libmetal minor: %d, ", metal_ver_minor());
    OPENAMP_MAIN_DEBUG_I("[SLAVE] Libmetal patch: %d)\r\n", metal_ver_patch());
}
