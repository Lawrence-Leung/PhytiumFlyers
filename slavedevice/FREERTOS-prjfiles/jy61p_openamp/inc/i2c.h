#ifndef __I2C_H
#define __I2C_H

#include"stdio.h"
#include "ftypes.h"
#include "FreeRTOS.h"
#include "fgpio_os.h"
#include "gpio_init.h"



// #define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
// #define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
// #define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum))  


// #define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C 
// #define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C 
// #define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x4001100C 
// #define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x4001140C 
// #define GPIOE_ODR_Addr    (GPIOE_BASE+12) //0x4001180C 
// #define GPIOF_ODR_Addr    (GPIOF_BASE+12) //0x40011A0C    
// #define GPIOG_ODR_Addr    (GPIOG_BASE+12) //0x40011E0C    

// #define GPIOA_IDR_Addr    (GPIOA_BASE+8) //0x40010808 
// #define GPIOB_IDR_Addr    (GPIOB_BASE+8) //0x40010C08 
// #define GPIOC_IDR_Addr    (GPIOC_BASE+8) //0x40011008 
// #define GPIOD_IDR_Addr    (GPIOD_BASE+8) //0x40011408 
// #define GPIOE_IDR_Addr    (GPIOE_BASE+8) //0x40011808 
// #define GPIOF_IDR_Addr    (GPIOF_BASE+8) //0x40011A08 
// #define GPIOG_IDR_Addr    (GPIOG_BASE+8) //0x40011E08 

// #define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n) 
// #define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n) 

// #define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  
// #define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)   
   	   		   

// #define SDA_IN()  {GPIOB->CRH&=0XFFFF0FFF;GPIOB->CRH|=0x00008000;}
// #define SDA_OUT() {GPIOB->CRH&=0XFFFF0FFF;GPIOB->CRH|=0x00003000;}
void SDA_IN(void);

void SDA_OUT(void);
	 
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
