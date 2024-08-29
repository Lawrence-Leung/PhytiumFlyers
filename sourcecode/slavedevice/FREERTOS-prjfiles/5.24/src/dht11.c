/*
    dht11.c
    DHT11 温度传感器 驱动程序
	by 周子琳, Lawrence Leung
    2024 飞腾风驰队
*/

#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "fdebug.h"
#include "fsleep.h"
#include "fgpio_os.h"
#include "fio_mux.h"
#include "dht11.h"

//GPIO3_2的配置
#define PIN_IRQ_OCCURED     (0x1 << 0)
#define DHT11_IO_INDEX FFREERTOS_GPIO_PIN_INDEX(4, 0, 13)
#define GPIO_WORK_TASK_NUM  2U

/***************** Macros (Inline Functions) Definitions *********************/
#define FGPIO_DEBUG_TAG "GPIO-IO"
#define FGPIO_ERROR(format, ...) FT_DEBUG_PRINT_E(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGPIO_WARN(format, ...)  FT_DEBUG_PRINT_W(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGPIO_INFO(format, ...)  FT_DEBUG_PRINT_I(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGPIO_DEBUG(format, ...) FT_DEBUG_PRINT_D(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)

static EventGroupHandle_t event = NULL;
static TaskHandle_t dht11_task_Handle = NULL;
static TaskHandle_t AppTask_Handle = NULL;

// 定义引脚配置
static FFreeRTOSFGpio *dht11_gpio= NULL;
static FFreeRTOSGpioConfig dht11_in_cfg;
static FFreeRTOSGpioConfig dht11_out_cfg;
static xSemaphoreHandle init_locker = NULL;

static u32 in_pin = DHT11_IO_INDEX; 
static u32 out_pin = DHT11_IO_INDEX;

// 所读出的数据
u8 humidity;		// 从DHT11提取的湿度
u8 temperature;	// 从DHT11提取的温度
u8 data_frame_lock;	 // 打包好的数据使用的锁
u8 data_frame[5];    // 打包好的数据，共5个字节

static boolean isInit = 0;
static boolean isStart = 0;

static FFreeRTOSGpioPinConfig DHT11_DATA_config =
{
    .pin_idx = DHT11_IO_INDEX,
    .mode = FGPIO_DIR_INPUT,
    .en_irq = FALSE,
    .irq_handler = NULL,
    .irq_args = NULL
};
// 发送信号：配置为输出
static FFreeRTOSGpioPinConfig DHT11_IO_config =
{
    .pin_idx = DHT11_IO_INDEX,
    .mode = FGPIO_DIR_OUTPUT,
    .en_irq = FALSE
};

/**
 * @brief 将引脚设置为输出模式
 * 
 */

void DHT11IoInit(void)
{
	FError err = FT_SUCCESS;
	//FGPIO_INFO("out_pin: 0x%x", DHT11_IO_INDEX);
	dht11_gpio = FFreeRTOSGpioInit(FFREERTOS_GPIO_PIN_CTRL_ID(DHT11_IO_INDEX), &dht11_out_cfg);			//获取instance
	FIOPadSetGpioMux(FFREERTOS_GPIO_PIN_CTRL_ID(DHT11_IO_INDEX), FFREERTOS_GPIO_PIN_ID(DHT11_IO_INDEX)); /* set io pad */
	
	// DHT11_IO_config.pin_idx = DHT11_IO_INDEX;
	// err = FFreeRTOSSetupPin(dht11_gpio, &DHT11_IO_config);
    FASSERT_MSG(FT_SUCCESS == err, "Init output gpio pin failed.");
	f_printk("[DHT11] DHT11 init success!\r\n");
    // vTaskDelete(NULL);
	isInit = 1;
}


void DHT11IoInitOut(void)
{
	FError err = FT_SUCCESS;
	//FGPIO_INFO("out_pin: 0x%x", DHT11_IO_INDEX);
	// dht11_gpio = FFreeRTOSGpioInit(FFREERTOS_GPIO_PIN_CTRL_ID(DHT11_IO_INDEX), &dht11_out_cfg);			//获取instance
	// FIOPadSetGpioMux(FFREERTOS_GPIO_PIN_CTRL_ID(DHT11_IO_INDEX), FFREERTOS_GPIO_PIN_ID(DHT11_IO_INDEX)); /* set io pad */
	
	// DHT11_IO_config.pin_idx = DHT11_IO_INDEX;
	err = FFreeRTOSSetupPin(dht11_gpio, &DHT11_IO_config);
    FASSERT_MSG(FT_SUCCESS == err, "Init output gpio pin failed.");
	// f_printk("[DHT11] DHT11 init output success!\r\n");
    // vTaskDelete(NULL);
}

/**
 * @brief DHT11的IO输入初始化
 * 
 */
void DHT11IoInitIn(void)
{
	FError err = FT_SUCCESS;
	err = FFreeRTOSSetupPin(dht11_gpio, &DHT11_DATA_config);
	FASSERT_MSG(FT_SUCCESS == err, "Init input gpio pin failed.");
	
}

/**
 * @brief 向dht11发生开始信号
 * 
 */
void DHT11Rst(void)
{
	f_printk("[DHT11] Resetting. \r\n");
	if(!isInit)
		DHT11IoInit();
	DHT11IoInitOut(); //输出模式
	
	FFreeRTOSPinWrite(dht11_gpio, DHT11_IO_INDEX, FGPIO_PIN_LOW); //拉低电平至少18ms
	fsleep_millisec(18);		//延时20ms
	
	
	FFreeRTOSPinWrite(dht11_gpio, DHT11_IO_INDEX, FGPIO_PIN_HIGH); //拉高电平20~40us
	// if(isStart == 0)
		fsleep_microsec(30);		//延时30us
	// else
	// 	fsleep_microsec(2);		//延时30us
	// isStart = 1;
	// f_printk("[DHT11] Reset finished. \r\n");
}

/**
 * @brief 等待DHT11回应
 * 
 * @return u8 重试次数
 */
u8 DHT11Check(void)
{   
	u8 retry = 0;
	DHT11IoInitIn(); //输入模式
	// FError err = FT_SUCCESS;
	// FError err = FFreeRTOSSetupPin(dht11_gpio, &DHT11_DATA_config);
	// FASSERT_MSG(FT_SUCCESS == err, "Init input gpio pin failed.");
	// f_printk("[DHT11] Getting response!!! \r\n");
    while (FFreeRTOSPinRead (dht11_gpio, DHT11_IO_INDEX) && retry < 100)//DHT11会拉低40~80us
	{
		retry++;
		fsleep_microsec(1);
	};	 
	if (retry >= 100) return 1;
	else retry = 0;
    while (!FFreeRTOSPinRead(dht11_gpio,DHT11_IO_INDEX)&&retry<100)//DHT11拉低后会再次拉高40~80us
	{
		retry++;
		fsleep_microsec(1);
	};
	if (retry >= 100) return 1;	    
	return 0;
}

/**
 * @brief 读取一个位的数据
 * 
 * @return u8 错误码
 */
u8 DHT11ReadBit(void)
{
 	u8 retry=0;
	while(FFreeRTOSPinRead(dht11_gpio,DHT11_IO_INDEX)&&retry<100)//等待变为低电平
	{
		retry++;
		fsleep_microsec(1);
	}
	retry=0;
	while(!FFreeRTOSPinRead(dht11_gpio,DHT11_IO_INDEX)&&retry<100)//等待变高电平
	{
		retry++;
		fsleep_microsec(1);
	}
	fsleep_microsec(40);//等待40us
	if(FFreeRTOSPinRead(dht11_gpio,DHT11_IO_INDEX))return 1;
	else return 0;		   
}

/**
 * @brief 读取一个字节
 * 
 * @return u8 一个字节的数据
 */
u8 DHT11ReadByte(void)
{        
	u8 i,dat;
	dat=0;
	for (i=0;i<8;i++) 
	{
		dat<<=1; 
		dat|=DHT11ReadBit();
	}						    
	return dat;
}

/**
 * @brief 读取一次数据
 * 
 * @param temp 温度指针
 * @param humi 湿度指针
 * @return u8 错误码
 */
u8 DHT11ReadData(u8 *temp, u8 *humi)
{       
	u8 buf[5];
	u8 i; 
	// u8 check;
	DHT11Rst();
	// check = DHT11Check();

	// f_printk("check:%d\r\n",check);

	if (DHT11Check() == 0) {
		// f_printk("Reading......");
		for (i=0;i<5;i++) {//读取40位数据
			buf[i] = DHT11ReadByte();
		}
		if ((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4]) {
			*humi = buf[0];
			*temp = buf[2];
			f_printk("H:%d   T:%d\r\n",buf[0],buf[2]);
		}
		else
		{
			f_printk("data error!");
		}
		
	}
	else{
		f_printk("singal error!");
		return 1;
	}
	
 	return 0;
	

	
	/*output test*/

	
	// DHT11IoInitOut();
	// f_printk("1 microsecond \r\n");
	// for(;;){
	// 	FFreeRTOSPinWrite(dht11_gpio,DHT11_IO_INDEX,FGPIO_PIN_HIGH);
	// 	// f_printk("out:high\r\n");
	// 	fsleep_microsec(30);
	// 	FFreeRTOSPinWrite(dht11_gpio,DHT11_IO_INDEX,FGPIO_PIN_LOW);
	// 	// f_printk("out:low\r\n");
	// 	fsleep_microsec(30);
	// }
	

	/*input test*/
	/*
	FGpioPinVal val;
	DHT11IoInitIn();
	for(;;)
	{
		val = FFreeRTOSPinRead(dht11_gpio,DHT11_IO_INDEX);
		f_printk("in:%d\r\n",val);
	}
	*/
	// return 0;
	
}

/**
 * @brief 计算简单的校验码
 * 
 * @param temp 温度指针
 * @param humi 湿度指针
 * @return u8 校验码
 */
u8 CalculateCheck(u8 *temp, u8 *humi)
{
	return (*humi + *temp) % 256;
}

/**
 * @brief 打包数据
 * 一共5个字节，每个字节完成：
 * 1. 固定字头，1byte
 * 2. 湿度数据，1byte
 * 3. 温度数据，1byte
 * 4. 将湿度和温度数据取CRC校验码的末尾1byte；
 * 5. 固定字尾，1byte。
 * 
 * @param temp 温度
 * @param humi 湿度
 * @param data_frame 打包好数据存储位置的首字节指针
 */
void DHT11PackData(u8 *temp, u8 *humi, u8* data_frame)
{
	if (data_frame_lock == 0) {
		data_frame_lock = 1;
		data_frame[0] = 0xA0;	//固定字头
		data_frame[1] = *humi;	//湿度数据
		data_frame[2] = *temp;	//温度数据
		data_frame[3] = CalculateCheck(humi, temp);	//校验码
		data_frame[4] = 0x5A;	//固定字尾
		data_frame_lock = 0;
	}
	else {
		f_printk("Data Locked! \r\n");
	}

	// for debug only
	f_printk("Data sent: ");
    for (int i = 0; i < 5; i++) {
        f_printk("%02X ", data_frame[i]);
    }
    f_printk("\r\n");
}

/**
 * @brief DHT11 FreeRTOS 任务
 * 
 */
void DHT11CircularRead(void)
{
	while (1)
	{
		taskENTER_CRITICAL();
		DHT11ReadData (&temperature, &humidity);
		DHT11PackData (&temperature, &humidity, data_frame);
		taskEXIT_CRITICAL();
		f_printk("[DHT11] Running \r\n");
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

/**
 * @brief 创建DHT11的FreeRTOS任务
 * 
 * @return BaseType_t 错误码
 */
BaseType_t DHT11Task(void)
{
    BaseType_t ret = pdPASS;

	// humidity = 0;	//初始化
	// temperature = 0;
	// data_frame_lock = 0;
	// u8 i;
	// for (i = 0; i < 5; i++) {
	// 	data_frame[i] = 0;
	// }
	// f_printk("[SLAVE] Start Initializing DHT11Task \r\n");
	taskENTER_CRITICAL();	//需要保障无法被干扰的状态
    ret = xTaskCreate((TaskFunction_t)DHT11CircularRead,  /* task entry */
                      (const char *)"DHT11Task",/* task name */
                      (uint16_t)1024,  /* task stack size in words */
                      (void *)NULL, /* task params */
                      (UBaseType_t)3,  /* task priority */
                      NULL); /* task handler */
	if (ret != pdPASS) {
		f_printk("[SLAVE] Create DHT11 task failed. \r\n");
	}

	taskEXIT_CRITICAL();	
    return ret;
}

