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
#define DHT11_IO_INDEX FFREERTOS_GPIO_PIN_INDEX(3, 0, 2)
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

//定义引脚配置
static FFreeRTOSFGpio *dht11_gpio= NULL;
static FFreeRTOSGpioConfig dht11_in_cfg;
static FFreeRTOSGpioConfig dht11_out_cfg;
static xSemaphoreHandle init_locker = NULL;

static u32 in_pin = DHT11_IO_INDEX; 
static u32 out_pin = DHT11_IO_INDEX;

static FFreeRTOSGpioPinConfig DHT11_DATA_config =
{
    .pin_idx = DHT11_IO_INDEX,
    .mode = FGPIO_DIR_INPUT,
    .en_irq = FALSE,
    .irq_handler = NULL,
    .irq_args = NULL
};
//发送信号：配置为输出
static FFreeRTOSGpioPinConfig DHT11_IO_config =
{
    .pin_idx = DHT11_IO_INDEX,
    .mode = FGPIO_DIR_OUTPUT,
    .en_irq = FALSE
};



//将引脚设置为输出模式
void DHT11_IO_Init_OUT(void)
{
	FError err = FT_SUCCESS;
	FGPIO_INFO("out_pin: 0x%x", DHT11_IO_INDEX);
	dht11_gpio = FFreeRTOSGpioInit(FFREERTOS_GPIO_PIN_CTRL_ID(DHT11_IO_INDEX), &dht11_out_cfg);			//获取instance
	FIOPadSetGpioMux(FFREERTOS_GPIO_PIN_CTRL_ID(DHT11_IO_INDEX), FFREERTOS_GPIO_PIN_ID(DHT11_IO_INDEX)); /* set io pad */
	
	DHT11_IO_config.pin_idx = DHT11_IO_INDEX;
	err = FFreeRTOSSetupPin(dht11_gpio, &DHT11_IO_config);
    FASSERT_MSG(FT_SUCCESS == err, "Init output gpio pin failed.");
	// f_printk("io init success!\r\n");

    // vTaskDelete(NULL);
}

void DHT11_IO_Init_IN(void)
{
	FError err = FT_SUCCESS;
	// f_printk("in_pin: 0x%x", DHT11_IO_INDEX);
	dht11_gpio = FFreeRTOSGpioInit(FFREERTOS_GPIO_PIN_CTRL_ID(DHT11_IO_INDEX), &dht11_in_cfg);			//获取instance
	FIOPadSetGpioMux(FFREERTOS_GPIO_PIN_CTRL_ID(DHT11_IO_INDEX), FFREERTOS_GPIO_PIN_ID(DHT11_IO_INDEX)); /* set io pad */

	DHT11_DATA_config.pin_idx = DHT11_IO_INDEX;
    DHT11_DATA_config.mode = FGPIO_DIR_INPUT;
	DHT11_DATA_config.en_irq = FALSE;
	err = FFreeRTOSSetupPin(dht11_gpio, &DHT11_DATA_config);
	FASSERT_MSG(FT_SUCCESS == err, "Init input gpio pin failed.");
	// f_printk("data init success!\r\n");
	// vTaskDelete(NULL);
	
}


//向dht11发生开始信号
void DHT11_Rst(void)
{
	DHT11_IO_Init_OUT(); //输出模式
	
	FFreeRTOSPinWrite(dht11_gpio, DHT11_IO_INDEX, FGPIO_PIN_LOW); //拉低电平至少18us
	fsleep_millisec(20);		//延时20ms
	
	FFreeRTOSPinWrite(dht11_gpio, DHT11_IO_INDEX, FGPIO_PIN_HIGH); //拉高电平20~40us
	fsleep_microsec(30);

}

// 等待DHT11回应
u8 DHT11_Check(void)
{   
	u8 retry=0;
	DHT11_IO_Init_IN(); //输入模式
	FError err = FT_SUCCESS;
	err = FFreeRTOSSetupPin(dht11_gpio, &DHT11_DATA_config);
	FASSERT_MSG(FT_SUCCESS == err, "Init input gpio pin failed.");
	// f_printk("getting response!!!");
    while (FFreeRTOSPinRead(dht11_gpio,DHT11_IO_INDEX)&&retry<100)//DHT11会拉低40~80us
	{
		retry++;
		fsleep_microsec(1);
	};	 
	if(retry>=100)return 1;
	else retry=0;
    while (!FFreeRTOSPinRead(dht11_gpio,DHT11_IO_INDEX)&&retry<100)//DHT11拉低后会再次拉高40~80us
	{
		retry++;
		fsleep_microsec(1);
	};
	if(retry>=100)return 1;	    
	return 0;
}

// 读取一个位
u8 DHT11_Read_Bit(void)
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

//读取一个字节
u8 DHT11_Read_Byte(void)
{        
	u8 i,dat;
	dat=0;
	for (i=0;i<8;i++) 
	{
		dat<<=1; 
		dat|=DHT11_Read_Bit();
	}						    
	return dat;
}

//读取一次数据
u8 DHT11_Read_Data(u8 *temp,u8 *humi)
{       
	u8 buf[5];
	u8 i; 
	DHT11_Rst();
	// f_printk("check:%d\r\n",DHT11_Check());
	if(DHT11_Check()==0)
	{
		// f_printk("reading......");
		for(i=0;i<5;i++)//读取40位数据
		{
			buf[i]=DHT11_Read_Byte();
		}
		if((buf[0]+buf[1]+buf[2]+buf[3])==buf[4])
		{
			*humi=buf[0];
			*temp=buf[2];
			f_printk("H:%d   T:%d\r\n",buf[0],buf[2]);
			// vTaskDelay(1000);
		}
	}
	else return 1;
	
 	return 0;
	
	/*output test*/
	// DHT11_IO_Init_OUT();
	// for(;;){
	// 	FFreeRTOSPinWrite(dht11_gpio,DHT11_IO_INDEX,FGPIO_PIN_HIGH);
	// 	f_printk("out:high\r\n");
	// 	fsleep_millisec(1000);
	// 	FFreeRTOSPinWrite(dht11_gpio,DHT11_IO_INDEX,FGPIO_PIN_LOW);
	// 	f_printk("out:low\r\n");
	// }
	/*input test*/
	// FGpioPinVal val;
	// DHT11_IO_Init_IN();
	// for(;;)
	// {
	// 	val = FFreeRTOSPinRead(dht11_gpio,DHT11_IO_INDEX);
	// 	f_printk("in:%d\r\n",val);
	// }
	
	// return 0;
}

void dht11_Task(void)
{
	u8 humi;
	u8 temp;
	while (1)
	{
		DHT11_Read_Data(&temp,&humi);
	}
	
}
BaseType_t AppTask(void)
{
    BaseType_t ret = pdPASS;

    taskENTER_CRITICAL(); /* no schedule when create task */

    ret = xTaskCreate((TaskFunction_t)dht11_Task,  /* task entry */
                      (const char *)"dht11_Task",/* task name */
                      (uint16_t)1024,  /* task stack size in words */
                      NULL, /* task params */
                      (UBaseType_t)configMAX_PRIORITIES - 1,  /* task priority */
                      NULL); /* task handler */

    FASSERT_MSG(pdPASS == ret, "Create task failed.");
	// xTaskResumeAll();

	vTaskDelete(AppTask_Handle);

    taskEXIT_CRITICAL(); /* allow schedule since task created */

    // ret = xTimerStart(exit_timer, 0); /* start */

    // FASSERT_MSG(pdPASS == ret, "Start exit timer failed.");

    return ret;
}
