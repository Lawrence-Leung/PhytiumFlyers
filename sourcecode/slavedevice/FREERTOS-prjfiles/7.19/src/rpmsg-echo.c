/*
    rpmsg-echo.c
    OpenAMP ͨ��Э��Դ��??
    2024 ���ڷ��??
*/

#include "rpmsg-echo.h"
#include "dht11.h"
// #include "uart.h"
#include "gps.h"
#include "syn6288.h"
#include "jy61p.h"
#include "uart.h"
#include "speech_i2c.h"
#include "speech_logic.h"

/* �������岿�� */
/* 1. DHT11 ��������?? */
extern u8 humidity;		// ��DHT11��ȡ��ʪ??
extern u8 temperature;	// ��DHT11��ȡ����??
extern u8 data_frame_lock;
extern u8 data_frame[5];    // DHT11�Ĵ���õ����ݣ�??5����??

/* 2. GPSģ����� */
extern double latitude;
extern double longitude;
extern u8 GPS_data_frame_lock;	 // ����õ�����ʹ�õ���
extern u8 GPS_datapack[14];

/* 3. �����Ǵ�������?? */
extern u8 jy61pLock;
extern float fAcc[3], fGyro[3], fAngle[3];
// extern float JY61P_datapack[12];
extern float JY61P_datapack1[6];
extern float JY61P_datapack2[6];
extern float JY61P_datapack3[6];

/* 4. OpenAMP rpmsg ͨ��Э����ر��� */
static int shutdown_req = 0;
void *platform;
struct rpmsg_device *rpdev;

/**
 * @brief OpenAMP rpmsg ������Ϣ�¼��ص�������DHT11����??
 * 
 * @param ept �˵�ṹ��ָ??
 * @param data ��Master�յ�������ָ??
 * @param len ��Master�յ������ݳ�??
 * @param src rpmsgͨ��Э���������Դλ�ñ�??
 * @param priv ˽������
 * @return int ����ֵ���ɹ���ʧ??
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
    //if (rpmsg_send(ept, data, len) < 0) //�������ֻ������echo���̵ģ����������Լ�����??

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

    //�Ա����ţ�˵���Լ���src�ֶ���ʲ??
    ECHO_DEV_SLAVE_DEBUG_I("[SLAVE] My Name Is %s, My Source Is %d, Hello Master.", ept->name, src);

    return RPMSG_SUCCESS;
}

/**
 * @brief OpenAMP rpmsg ������Ϣ�¼��ص�������DHT11����??
 * 
 * @param ept �˵�ṹ��ָ??
 * @param data ��Master�յ�������ָ??
 * @param len ��Master�յ������ݳ�??
 * @param src rpmsgͨ��Э���������Դλ�ñ�??
 * @param priv ˽������
 * @return int ����ֵ���ɹ���ʧ??
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
    //if (rpmsg_send(ept, data, len) < 0) //�������ֻ������echo���̵ģ����������Լ�����??

    if (GPS_data_frame_lock == 0) {

        // UartWaitLoop();

        if(Save_Data.isPackable == true)
        {
            Save_Data.isPackable = false;
            GPSPackData(&latitude,&longitude,GPS_datapack);
            f_printk("[GPS] Finish Pack\r\n");
        }
        else{
            printf("[GPS] recv failed!\r\n");
        }

        
        GPS_data_frame_lock = 1;
        if (rpmsg_send(ept, GPS_datapack, sizeof(GPS_datapack))) {
            ECHO_DEV_SLAVE_DEBUG_E("rpmsg_send failed.\r\n");
        }
        GPS_data_frame_lock = 0;
    }

    //�Ա����ţ�˵���Լ���src�ֶ���ʲ??
    ECHO_DEV_SLAVE_DEBUG_I("[SLAVE] My Name Is %s, My Source Is %d, Hello Master.", ept->name, src);

    return RPMSG_SUCCESS;
}

/**
 * @brief OpenAMP rpmsg ������Ϣ�¼��ص�������SYN6288����ģ��
 * 
 * @param ept �˵�ṹ��ָ??
 * @param data ��Master�յ�������ָ??
 * @param len ��Master�յ������ݳ�??
 * @param src rpmsgͨ��Э���������Դλ�ñ�??
 * @param priv ˽������
 * @return int ����ֵ���ɹ���ʧ??
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

    /*�����յ�����??*/
    f_printk("%s\r\n",data);
    // syn6288Play();
    syn6288Play(data);

    //�Ա����ţ�˵���Լ���src�ֶ���ʲ??
    ECHO_DEV_SLAVE_DEBUG_I("[SLAVE] My Name Is %s, My Source Is %d, Hello Master.", ept->name, src);

    return RPMSG_SUCCESS;
}

/**
 * @brief OpenAMP rpmsg ������Ϣ�¼��ص�������SYN6288����ģ��
 * 
 * @param ept �˵�ṹ��ָ??
 * @param data ��Master�յ�������ָ??
 * @param len ��Master�յ������ݳ�??
 * @param src rpmsgͨ��Э���������Դλ�ñ�??
 * @param priv ˽������
 * @return int ����ֵ���ɹ���ʧ??
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

    /*�����յ�����??*/
    f_printk("%s\r\n",data);

    // speech_text(data,GBK);
    parse_data(data);

    //�Ա����ţ�˵���Լ���src�ֶ���ʲ??
    ECHO_DEV_SLAVE_DEBUG_I("[SLAVE] My Name Is %s, My Source Is %d, Hello Master.", ept->name, src);

    return RPMSG_SUCCESS;
}

/**
 * @brief OpenAMP rpmsg ������Ϣ�¼��ص�������JY61P�����Ǵ���??
 * 
 * @param ept �˵�ṹ��ָ??
 * @param data ��Master�յ�������ָ??
 * @param len ��Master�յ������ݳ�??
 * @param src rpmsgͨ��Э���������Դλ�ñ�??
 * @param priv ˽������
 * @return int ����ֵ���ɹ���ʧ??
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

        // jy61pFunc(fAcc,fGyro,fAngle);
        // f_printk("lock:%d\r\n",jy61pLock);
        JY61PPackData(fAcc,fGyro,fAngle,JY61P_datapack1,JY61P_datapack2,JY61P_datapack3);
        f_printk("[JY61P] Running \r\n");
        jy61pLock = 1;
        // f_printk("sent:1\r\n");
        if (rpmsg_send(ept, JY61P_datapack1, sizeof(JY61P_datapack1)) < 0) {
            ECHO_DEV_SLAVE_DEBUG_E("rpmsg_send failed.\r\n");
        }
        // f_printk("sent:2\r\n");
        if (rpmsg_send(ept, JY61P_datapack2, sizeof(JY61P_datapack2)) < 0) {
            ECHO_DEV_SLAVE_DEBUG_E("rpmsg_send failed.\r\n");
        }
        // f_printk("sent:3\r\n");
        if (rpmsg_send(ept, JY61P_datapack3, sizeof(JY61P_datapack3)) < 0) {
            ECHO_DEV_SLAVE_DEBUG_E("rpmsg_send failed.\r\n");
        }
        jy61pLock = 0;
    } else {
		f_printk("Data Locked! \r\n");
    }

    //�Ա����ţ�˵���Լ���src�ֶ���ʲ??
    ECHO_DEV_SLAVE_DEBUG_I("[SLAVE] My Name Is %s, My Source Is %d, Hello Master.", ept->name, src);

    return RPMSG_SUCCESS;
}

/**
 * @brief ���ö˵��������ʱ�Ļص�������һ��Ϊɾ���˵㡢�ͷ��ڴ�ռ䣩
 * 
 * @param ept �˵�ָ��
 */
static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
    (void)ept;
    ECHO_DEV_SLAVE_DEBUG_I("[SLAVE] Unexpected remote endpoint destroy.\r\n");
    shutdown_req = 1;
}

/**
 * @brief OpenAMP rpmsg ͨ�Ŷ˵��ʼ����??
 * 
 * @param rdev rpmsg �豸�ṹ��ָ??
 * @param priv ƽָ̨�룬������Ϊ˽������ʹ??
 * @return int ����??
 */
int FRpmsgCommunication(void) //struct rpmsg_device *rdev, void *priv)
{
    int ret = 0;
    struct rpmsg_endpoint lept1, lept2, lept3, lept4;  //���Դ���4���˵㣬ȫ��̬??
    shutdown_req = 0;
    //��ʼ��OpenAMP��rpmsgͨ�ſ��
    f_printk("[OPENAMP] Start to create endpoints. \r\n");

    // �˵�1��Ԥ����DHT11
    taskENTER_CRITICAL();
    ret = rpmsg_create_ept(
        &lept1, // rpmsg �˵�ṹ��ָ??
        rpdev, // rpmsg device �ṹ��ָ??
        "dht11", // �˵�����
        10, // �˵���ʼ��ַ
        RPMSG_ADDR_ANY,  // �˵�Ŀ�ĵ�ַ
        rpmsgDHT11Response,  // ���ö˵������Ϣʱ�Ļص�����
        rpmsg_service_unbind    // ���ö˵��������ʱ�Ļص�������һ��Ϊɾ���˵㡢�ͷ��ڴ�ռ䣩
        );
    if (ret) {
        ECHO_DEV_SLAVE_DEBUG_E("Failed to create endpoint. %d \r\n", ret);
        return -1;
    }
    f_printk("[OPENAMP] rpmsg_create_ept 001 finished. \r\n");
    taskEXIT_CRITICAL();

    // �˵�2��Ԥ����GPS���������������Ҫ�������������ȡ��ע�͵���??
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

    // �˵�3������ģ��
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

    // �˵�4 : JY61P
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
    //��ע������ͬ���ķ������Դ���Ϊ�����ഫ��������������ͨ��??
    ECHO_DEV_SLAVE_DEBUG_I("Successfully created rpmsg endpoint.\r\n");

    while (1)   // ��ѭ??
    {
        PlatformPollTasklette(platform);    //�µ�Platform Poll ������֧�ַ���������
        if (shutdown_req) { // ���յ��ر�����ʱ���˳���??
            break;
        }
    }

    //���ٶ˵㡣��Ȼ�����յ���shutdown_req֮�����??
    taskENTER_CRITICAL();
    rpmsg_destroy_ept(&lept1);
    rpmsg_destroy_ept(&lept2);   
    rpmsg_destroy_ept(&lept3);
    rpmsg_destroy_ept(&lept4);   
    taskEXIT_CRITICAL();

    return ret;
}

/**
 * @brief OpenAMP rpmsgͨ�ų�ʼ����??
 * 
 * @return int ����ֵ���˳�OpenAMP����
 */
int FOpenampInit(void)  //(void *platform, struct rpmsg_device *rpdev)
{
    int ret = 0;

    // ����1����ʼ��ƽ̨
    taskENTER_CRITICAL();
    ret = platform_init(0, NULL, &platform);
    f_printk("[OPENAMP] platform_init finished. \r\n");
    taskEXIT_CRITICAL();
    if (ret) {
        ECHO_DEV_SLAVE_DEBUG_E("Failed to initialize platform.\r\n");
        platform_cleanup(platform);
        return -1;
    }

    // ����2������rpmsg vdev�����豸
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
 * @brief ��������Ϣ
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
 * @brief OpenAMP ͨ����ں���
 * 
 * @return BaseType_t ����??
 */
BaseType_t OpenAmpTask(void)
{
    BaseType_t ret;
    f_printk("[SLAVE] Start Initializing OpenAMP rpmsg Task \r\n");

    // ��������1
    taskENTER_CRITICAL();	
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

    // // ��������2
    // ret = xTaskCreate(
    //     (TaskFunction_t)RTOSDebugOutputOnly,
    //     (const char *)"DebugTask",
    //     (uint16_t)(1024),
    //     (void *)NULL,
    //     (UBaseType_t)12,    //���֮�����ø��ߵ����ȼ�
    //     NULL);
    // if (ret != pdPASS) {
    //     f_printk("[DEBUG] Failed to create DebugTask task \r\n");
    //     return ret;
    // }


	
    ret = xTaskCreate((TaskFunction_t)DHT11CircularRead,  /* task entry */
                      (const char *)"DHT11Task",/* task name */
                      (uint16_t)512,  /* task stack size in words */
                      (void *)NULL, /* task params */
                      (UBaseType_t)1,  /* task priority */
                      NULL); /* task handler */
	if (ret != pdPASS) {
		f_printk("[SLAVE] Create DHT11 task failed. \r\n");
        return ret;
	}

    ret = xTaskCreate((TaskFunction_t)jy61pTask,  /* task entry */
                      (const char *)"jy61pTask",/* task name */
                      (uint16_t)1024,  /* task stack size in words */
                      (void *)NULL, /* task params */
                      (UBaseType_t)7,  /* task priority */
                      NULL); /* task handler */
	if (ret != pdPASS) {
		f_printk("[SLAVE] Create JY61P task failed. \r\n");
        return ret;
	}

    ret = xTaskCreate((TaskFunction_t)speechOut,  /* task entry */
                      (const char *)"speechTask",/* task name */
                      (uint16_t)1024,  /* task stack size in words */
                      (void *)NULL, /* task params */
                      (UBaseType_t)8,  /* task priority */
                      NULL); /* task handler */
	if (ret != pdPASS) {
		f_printk("[SLAVE] Create SPEECH task failed. \r\n");
        return ret;
	}

    // taskENTER_CRITICAL();	
    ret = xTaskCreate((TaskFunction_t)UartWaitLoop,  /* task entry */
                      (const char *)"GPSTask",/* task name */
                      (uint16_t)4096,  /* task stack size in words */
                      (void *)NULL, /* task params */
                      (UBaseType_t)9,  /* task priority */
                      NULL); /* task handler */
	if (ret != pdPASS) {
		f_printk("[SLAVE] Create DHT11 task failed. \r\n");
        return ret;
	}
	taskEXIT_CRITICAL();


    return ret;
}

/**
 * @brief OpenAMP�汾����??
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
