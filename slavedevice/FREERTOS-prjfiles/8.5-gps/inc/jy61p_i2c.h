#ifndef __JY61P_I2C_H
#define __JY61P_I2C_H


#include"stdio.h"
#include "ftypes.h"
#include "FreeRTOS.h"
#include "fgpio_os.h"
#include "gpio_init.h"

	 
extern FFreeRTOSFGpio *jy61p_sda_gpio;
extern FFreeRTOSFGpio *jy61p_scl_gpio;
extern FFreeRTOSGpioConfig jy61p_sda_cfg;
extern FFreeRTOSGpioConfig jy61p_scl_cfg;

extern FFreeRTOSGpioPinConfig JY61P_SCL_config;
//发送信号：配置为输出
extern FFreeRTOSGpioPinConfig JY61P_SDA_config;
#define JY61P_SCL_HIGH   FFreeRTOSPinWrite(jy61p_scl_gpio, JY61P_SCL_INDEX, FGPIO_PIN_HIGH);  //SCL
#define JY61P_SCL_LOW    FFreeRTOSPinWrite(jy61p_scl_gpio, JY61P_SCL_INDEX, FGPIO_PIN_LOW);   //SCL
#define JY61P_SDA_HIGH   FFreeRTOSPinWrite(jy61p_sda_gpio, JY61P_SDA_INDEX, FGPIO_PIN_HIGH);  //SDA
#define JY61P_SDA_LOW    FFreeRTOSPinWrite(jy61p_sda_gpio, JY61P_SDA_INDEX, FGPIO_PIN_LOW);   //SDA
#define READ_SDA   FFreeRTOSPinRead(jy61p_sda_gpio, JY61P_SDA_INDEX)   //SDA

void IIC_Init(void);                			 
void IIC_Start(void);				
void IIC_Stop(void);	  			
void IIC_Send_Byte(u8 txd);			
u8 IIC_Read_Byte(unsigned char ack);
u8 IIC_Wait_Ack(void); 				
void IIC_Ack(void);					
void IIC_NAck(void);				

int32_t IICreadBytes(uint8_t dev, uint8_t reg, uint8_t *data, uint32_t length);
int32_t IICwriteBytes(uint8_t dev, uint8_t reg, uint8_t* data, uint32_t length);





#endif