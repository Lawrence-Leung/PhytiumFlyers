/*
    main.c
    Slave端主函数源代码文件
    2024 飞腾风驰队
*/
#include <stdio.h>
#include "dht11.h"   // DHT11 温度湿度传感器
#include "speech_i2c.h"
#include "rpmsg-echo.h" // OpenAMP 数据传输
#include "jy61p.h"
#include "uart.h"

SemaphoreHandle_t xMutex_speech;
SemaphoreHandle_t xMutex_isGetData;
SemaphoreHandle_t xMutex_jy61p;
SemaphoreHandle_t xMutex_GPSData;
int main(void)
{
    BaseType_t ret;
    OpenAmpVersionInfo(); // 初始化时显示信息

    f_printk("[SLAVE] Start Initializing Sensors \r\n");

    FOpenampInit();
    jy61pInit();
    GPSInit();
    SpeechInit();
    xMutex_speech = xSemaphoreCreateMutex();
    if (xMutex_speech == NULL)
    {
        printf("Failed to create mutex\n");
    }
    xMutex_isGetData = xSemaphoreCreateMutex();
    if (xMutex_isGetData == NULL)
    {
        printf("Failed to create mutex for isGetData\n");
    }
    xMutex_jy61p = xSemaphoreCreateMutex();
    if (xMutex_jy61p == NULL)
    {
        // 处理互斥量创建失败的情况
        printf("Failed to create mutex\n");
    }
    xMutex_GPSData = xSemaphoreCreateMutex();
    if (xMutex_GPSData == NULL) {
        printf("Failed to create GPS data mutex\n");
    }

    ret = OpenAmpTask(); // 执行循环处理OpenAMPTask任务
    if (ret != pdPASS)
    {
        goto FAIL_EXIT;
    }
    else
    {
        f_printk("[SLAVE] OpenAMP task created successfully. \r\n");
    }

    vTaskStartScheduler(); // 启动任务，开启调度
    while (1)
    {
        f_printk("[SLAVE] Slave system FreeRTOS core module failed. Please reboot device immediately. \r\n");
    } // 正常不会执行到这里

FAIL_EXIT:
    printf("[SLAVE] Failed, the return value of which is 0x%x. \r\n", ret);
    return 0;
}
