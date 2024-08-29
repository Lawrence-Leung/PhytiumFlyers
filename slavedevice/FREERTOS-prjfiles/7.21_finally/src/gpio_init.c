#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "fdebug.h"
#include "fsleep.h"
#include "fgpio_os.h"
#include "fio_mux.h"
#include "gpio_init.h"
// #include "fi2c_os.h"


//GPIO3_2的配置
#define PIN_IRQ_OCCURED     (0x1 << 0)

#define GPIO_WORK_TASK_NUM  2U

/***************** Macros (Inline Functions) Definitions *********************/
#define FGPIO_DEBUG_TAG "GPIO-IO"
#define FGPIO_ERROR(format, ...) FT_DEBUG_PRINT_E(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGPIO_WARN(format, ...)  FT_DEBUG_PRINT_W(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGPIO_INFO(format, ...)  FT_DEBUG_PRINT_I(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGPIO_DEBUG(format, ...) FT_DEBUG_PRINT_D(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)

static EventGroupHandle_t event = NULL;
// static TaskHandle_t dht11_task_Handle = NULL;
static TaskHandle_t AppTask_Handle = NULL;

//定义引脚配置
FFreeRTOSFGpio *jy61p_sda_gpio= NULL;
FFreeRTOSFGpio *jy61p_scl_gpio= NULL;
FFreeRTOSGpioConfig jy61p_sda_cfg;
FFreeRTOSGpioConfig jy61p_scl_cfg;
xSemaphoreHandle init_locker = NULL;

FFreeRTOSGpioPinConfig JY61P_SCL_config =
{
    .pin_idx = JY61P_SCL_INDEX,
    .mode = FGPIO_DIR_OUTPUT,
    .en_irq = FALSE,
    // .irq_handler = NULL,
    // .irq_args = NULL
};

FFreeRTOSGpioPinConfig JY61P_SDA_config =
{
    .pin_idx = JY61P_SDA_INDEX,
    .mode = FGPIO_DIR_OUTPUT,
    .en_irq = FALSE
};

FFreeRTOSFGpio *speech_sda_gpio= NULL;
FFreeRTOSFGpio *speech_scl_gpio= NULL;
FFreeRTOSGpioConfig speech_sda_cfg;
FFreeRTOSGpioConfig speech_scl_cfg;

FFreeRTOSGpioPinConfig SPEECH_SCL_config =
{
    .pin_idx = SPEECH_SCL_INDEX,
    .mode = FGPIO_DIR_OUTPUT,
    .en_irq = FALSE,
    // .irq_handler = NULL,
    // .irq_args = NULL
};

FFreeRTOSGpioPinConfig SPEECH_SDA_config =
{
    .pin_idx = SPEECH_SDA_INDEX,
    .mode = FGPIO_DIR_OUTPUT,
    .en_irq = FALSE
};



// //将引脚设置为输出模式
// void Jy61pSclInitOut(void)
// {
// 	FError err = FT_SUCCESS;
//     f_printk("scl_pin: 0x%x", JY61P_SCL_INDEX);
// 	jy61p_scl_gpio = FFreeRTOSGpioInit(FFREERTOS_GPIO_PIN_CTRL_ID(JY61P_SCL_INDEX), &jy61p_scl_cfg);			//获取instance
// 	FIOPadSetGpioMux(FFREERTOS_GPIO_PIN_CTRL_ID(JY61P_SCL_INDEX), FFREERTOS_GPIO_PIN_ID(JY61P_SCL_INDEX)); /* set io pad */
	
// 	// DHT11_IO_config.pin_idx = DHT11_IO_INDEX;
// 	err = FFreeRTOSSetupPin(jy61p_scl_gpio, &JY61P_SCL_config);
//     FASSERT_MSG(FT_SUCCESS == err, "Init output gpio pin failed.");
// 	f_printk("scl init success!\r\n");

//     /*******************************************************/
//     // FIOPadSetMioMux(FMIO4_ID);
//     // f_printk("scl init success!\r\n");

// }

// void Jy61pSdaInitOut(void)
// {
// 	FError err = FT_SUCCESS;
//     f_printk("sda_pin: 0x%x", JY61P_SDA_INDEX);
// 	jy61p_sda_gpio = FFreeRTOSGpioInit(FFREERTOS_GPIO_PIN_CTRL_ID(JY61P_SDA_INDEX), &jy61p_sda_cfg);			//获取instance
// 	FIOPadSetGpioMux(FFREERTOS_GPIO_PIN_CTRL_ID(JY61P_SDA_INDEX), FFREERTOS_GPIO_PIN_ID(JY61P_SDA_INDEX)); /* set io pad */

// 	err = FFreeRTOSSetupPin(jy61p_sda_gpio, &JY61P_SDA_config);
// 	FASSERT_MSG(FT_SUCCESS == err, "Init input gpio pin failed.");
// 	f_printk("sda init success!\r\n");
//     /*******************************************************/
//     // FIOPadSetMioMux(FMIO4_ID);
//     // f_printk("sda init success!\r\n");
	
// }

void GpioInit(u32 pin_index,FFreeRTOSFGpio **gpio,const FFreeRTOSGpioConfig gpio_cfg,const FFreeRTOSGpioPinConfig gpio_config)
{
    FError err = FT_SUCCESS;
    f_printk("pin: 0x%x ", pin_index);
	*gpio = FFreeRTOSGpioInit(FFREERTOS_GPIO_PIN_CTRL_ID(pin_index), &gpio_cfg);			//获取instance
	FIOPadSetGpioMux(FFREERTOS_GPIO_PIN_CTRL_ID(pin_index), FFREERTOS_GPIO_PIN_ID(pin_index)); /* set io pad */

	err = FFreeRTOSSetupPin(*gpio, &gpio_config);
	FASSERT_MSG(FT_SUCCESS == err, " Init pin failed.");
	f_printk("init success!\r\n");
}

void SDA_IN(FFreeRTOSFGpio *gpio,FFreeRTOSGpioPinConfig gpio_config)
{
    gpio_config.mode = FGPIO_DIR_INPUT;
    FFreeRTOSSetupPin(gpio, &gpio_config);
}

// void SDA_OUT(void)
// {
//     SPEECH_SDA_config.mode = FGPIO_DIR_OUTPUT;
//     FFreeRTOSSetupPin(speech_sda_gpio, &SPEECH_SDA_config);
// }

void SDA_OUT(FFreeRTOSFGpio *gpio,FFreeRTOSGpioPinConfig gpio_config)
{
    gpio_config.mode = FGPIO_DIR_OUTPUT;
    FFreeRTOSSetupPin(gpio, &gpio_config);
}