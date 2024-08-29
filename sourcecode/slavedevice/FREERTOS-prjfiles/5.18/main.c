/*
    main.c
    Slave端主函数源代码文件
    2024 飞腾风驰队
*/
#include <stdio.h>
#include "dht11.h"      // DHT11 温度湿度传感器
#include "syn6288.h"    // SYN6288 语音模块
#include "speech_i2c.h"
#include "rpmsg-echo.h" // OpenAMP 数据传输
#include "uart.h"

int main(void)
{
    BaseType_t ret;
    OpenAmpVersionInfo();   // 初始化时显示信息

    f_printk("[SLAVE] Start Initializing Sensors \r\n");
    UartInit();
    FOpenampInit();
    // syn6288UartInit();
    SpeechInit();
    // SetVolume(1);

    ret = OpenAmpTask();    // 执行循环处理OpenAMPTask任务
    if (ret != pdPASS)
    {
        goto FAIL_EXIT;
    } else {
        f_printk("[SLAVE] OpenAMP task created successfully. \r\n");
    }

    vTaskStartScheduler();  // 启动任务，开启调度 
    while (1)
    {
        f_printk("[SLAVE] Slave system FreeRTOS core module failed. Please reboot device immediately. \r\n");
    } // 正常不会执行到这里 

FAIL_EXIT:
    printf("[SLAVE] Failed, the return value of which is 0x%x. \r\n", ret);
    return 0;
}
