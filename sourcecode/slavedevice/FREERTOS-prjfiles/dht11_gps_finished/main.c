/*
    main.c
    Slave端主函数源代码文件
    by 周子琳, Lawrence Leung
    2024 飞腾风驰队
*/
#include <stdio.h>
#include "dht11.h"  //DHT11 温度湿度传感器
#include "rpmsg-echo.h" //OpenAMP 数据传输

int main(void)
{
    BaseType_t ret;
    OpenAmpVersionInfo();   // 初始化时显示信息

    f_printk("[SLAVE] Start Initializing Sensors \r\n");
    FOpenampInit();
    
    /*
    ret = DHT11Task();        //执行循环读取DHT11传感器数据
    if (ret != pdPASS)
    {
        goto FAIL_EXIT;
    } else {
        f_printk("[SLAVE] DHT11 task created successfully. \r\n");
    }
    */

    ret = OpenAmpTask();    //执行循环处理OpenAMP
    if (ret != pdPASS)
    {
        goto FAIL_EXIT;
    } else {
        f_printk("[SLAVE] OpenAMP task created successfully. \r\n");
    }

    vTaskStartScheduler(); // 启动任务，开启调度 
    while (1)
    {
        f_printk("failed\r\n");
    } // 正常不会执行到这里 

FAIL_EXIT:
    printf("Failed,the ret value is 0x%x. \r\n", ret);
    return 0;
}
