#ifndef __GPIO_INIT_H
#define __GPIO_INIT_H

#define SPEECH_SCL_INDEX FFREERTOS_GPIO_PIN_INDEX(1,0,11)
#define SPEECH_SDA_INDEX FFREERTOS_GPIO_PIN_INDEX(1,0,12)

#define JY61P_SCL_INDEX FFREERTOS_GPIO_PIN_INDEX(3,0,2)
#define JY61P_SDA_INDEX FFREERTOS_GPIO_PIN_INDEX(3,0,1)

#include "ftypes.h"
#include "FreeRTOS.h"
#include "fgpio_os.h"
// void Jy61pSclInitOut(void);
// void Jy61pSdaInitOut(void);


void GpioInit(u32 pin_index,FFreeRTOSFGpio **gpio,const FFreeRTOSGpioConfig gpio_cfg,const FFreeRTOSGpioPinConfig gpio_config);
void SDA_IN(FFreeRTOSFGpio *gpio,FFreeRTOSGpioPinConfig gpio_config);
void SDA_OUT(FFreeRTOSFGpio *gpio,FFreeRTOSGpioPinConfig gpio_config);

#endif