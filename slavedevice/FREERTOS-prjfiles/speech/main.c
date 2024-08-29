/*
    main.c
    Slave端主函数源代码文�?
    by 周子�?, Lawrence Leung
    2024 飞腾风驰�?
*/
#include <stdio.h>
#include "fsleep.h"
#include "speech_i2c.h"
#include "FreeRTOS.h"
#include "task.h"
// #include "i2c.h"
// #include "rpmsg-echo.h" //OpenAMP 数据传输

void us(void *pvParameters)
{
    while (1)
    {
        // SPEECH_SDA_HIGH;
        // fsleep_microsec(10);
        // SPEECH_SDA_LOW;
        // fsleep_microsec(10);

        SetVolume(1);
        printf("set volume\r\n");
        SetReader(Reader_XiaoPing);
        printf("SetReader\r\n");
        speech_text("����ǲ����ܿƼ�",GBK);
        printf("speech!\r\n");
        while(GetChipStatus() != ChipStatus_Idle)  //等待芯片空闲
        {
          fsleep_millisec(2);
        //   printf("%x\r\n",GetChipStatus());
        }
        // IIC_Start();
        // IIC_Send_Byte(I2C_ADDR);
        // IIC_Stop();
        // speech_text("1",0x00);
        // f_printk("%0x\r\n",GetChipStatus());
        // fsleep_millisec(1);

    }
}

BaseType_t task(void)
{
    BaseType_t ret;

    ret = xTaskCreate(
        (TaskFunction_t)us, 
        (const char *)"OpenAMPTask", 
        (uint16_t)(4096 * 2),
        (void *)NULL,
        (UBaseType_t)1,
        NULL);
    if (ret != pdPASS) {
        f_printk("[SLAVE] Failed to create a rpmsg_echo task \r\n");
    }
    return ret;
}

int main(void)
{
    IIC_Init();
    SDA_OUT();
    BaseType_t ret;
    ret = task();
    if (ret != pdPASS) {
        printf("[SLAVE] Failed to create a rpmsg_echo task \r\n");
        return 1; // 任务创建失败，退出程�?
    }

    vTaskStartScheduler();
    while (1)
    {
        /* code */
    }
    
//     OpenAmpVersionInfo();   // 初始化时显示信息

//     f_printk("[SLAVE] Start Initializing Sensors \r\n");
//     FOpenampInit();

//     ret = OpenAmpTask();    //执行循环处理OpenAMP
//     if (ret != pdPASS)
//     {
//         goto FAIL_EXIT;
//     } else {
//         f_printk("[SLAVE] OpenAMP task created successfully. \r\n");
//     }

    // vTaskStartScheduler(); // 启动任务，开启调�? 
//     while (1)
//     {
//         f_printk("failed\r\n");
//     } // 正常不会执行到这�? 

FAIL_EXIT:
    printf("Failed,the ret value is 0x%x. \r\n", ret);
    // IIC_Init();
    // printf("init success!\r\n");
    // SetVolume(10);
    // printf("set volume\r\n");
    // SetReader(Reader_XiaoPing);
    // printf("SetReader\r\n");
    // speech_text("你好亚博智能科技",GB2312);
    // printf("speech!\r\n");
	// while(GetChipStatus() != ChipStatus_Idle)  //等待芯片空闲
	// {
	//   fsleep_microsec(10);
    //   printf("%x\r\n",GetChipStatus());
	// }
    
    return 0;
}
