#include "i2c.h"
#include "FreeRTOS.h"
#include "fgpio_os.h"
#include "gpio_init.h"
#include "fsleep.h"

// void Delay(u32 count)//400KHzIIC
// {
// 	unsigned int uiCnt = count*8;
// 	while (uiCnt --);
// }


void SDA_IN(void)
{
    JY61P_SDA_config.mode = FGPIO_DIR_INPUT;
    FFreeRTOSSetupPin(jy61p_sda_gpio, &JY61P_SDA_config);
}

void SDA_OUT(void)
{
    JY61P_SDA_config.mode = FGPIO_DIR_OUTPUT;
    FFreeRTOSSetupPin(jy61p_sda_gpio, &JY61P_SDA_config);
}


void IIC_Init(void)
{			
    Jy61pSclInitOut();
    Jy61pSdaInitOut();
	
	SDA_OUT();     
	// IIC_SDA=1;	  	  
	// IIC_SCL=1;
    JY61P_SDA_HIGH;
    JY61P_SCL_HIGH;
}

void IIC_Start(void)
{
	SDA_OUT();    
	JY61P_SDA_HIGH;	  	  
	JY61P_SCL_HIGH;
	
	// Delay(5);        12.5us
    fsleep_microsec(13);
 	JY61P_SDA_LOW;//START:when CLK is high,DATA change form high to low 
	
	// Delay(5);
    fsleep_microsec(13);
	JY61P_SCL_LOW;
}
	  
void IIC_Stop(void)
{
	SDA_OUT();
	JY61P_SCL_LOW;
	JY61P_SDA_LOW;//STOP:when CLK is high DATA change form low to high
 	
    // Delay(5);
    fsleep_microsec(13);
	JY61P_SCL_HIGH; 
	JY61P_SDA_HIGH;
	
    // Delay(5);
    fsleep_microsec(13);							   	
}

u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0; 
	// SDA_IN();
    SDA_OUT();              //先设置为输出
	JY61P_SDA_HIGH;
		// Delay(5);	
    fsleep_microsec(13);  
    SDA_IN();               //将上面的输入移到这里
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>50)
		{
			IIC_Stop();
			return 1;
		}
		// Delay(5);
	}  
	JY61P_SCL_HIGH;
	// Delay(5); 
    fsleep_microsec(13);  
	JY61P_SCL_LOW;
	return 0;  
} 

void IIC_Ack(void)
{
	JY61P_SCL_LOW;
	SDA_OUT();
	JY61P_SDA_LOW;
		// Delay(5);
    fsleep_microsec(13);  
	JY61P_SCL_HIGH;
		// Delay(5);
    fsleep_microsec(13);  
	JY61P_SCL_LOW;
}
	    
void IIC_NAck(void)
{
	JY61P_SCL_LOW;
	SDA_OUT();
	JY61P_SDA_HIGH;
	
		// Delay(5);
    fsleep_microsec(13);
	JY61P_SCL_HIGH;
		// Delay(5);
    fsleep_microsec(13);
	JY61P_SCL_LOW;
}					 				     
		  
void IIC_Send_Byte(u8 txd)
{                        
    u8 t; 
    SDA_OUT(); 	    
    JY61P_SCL_LOW;
    for(t=0;t<8;t++)
    {
        // IIC_SDA=(txd&0x80)>>7;
        if((txd&0x80)>>7 == 1)
        {
            JY61P_SDA_HIGH;
        }
        else
        {
            JY61P_SDA_LOW;
        }
        txd<<=1; 	  
			
		// Delay(2);  5us 
        fsleep_microsec(5);
		JY61P_SCL_HIGH;
		// Delay(5);
        fsleep_microsec(13);
		JY61P_SCL_LOW;	
		// Delay(3);    7.5us
        fsleep_microsec(8);
    }	 
} 	 
  
u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();
    for(i=0;i<8;i++ )
	{
        JY61P_SCL_LOW; 
        
		// Delay(5);
        fsleep_microsec(13);
		JY61P_SCL_HIGH;
        receive<<=1;
        if(READ_SDA)receive++;   
		
		// Delay(5); 
        fsleep_microsec(13);
    }					 
    if (ack)
        IIC_Ack(); 
    else
        IIC_NAck();
    return receive;
}

int32_t IICreadBytes(uint8_t dev, uint8_t reg, uint8_t *data, uint32_t length)
{
    uint32_t count = 0;

    IIC_Start();
    IIC_Send_Byte(dev);	
    if(IIC_Wait_Ack() == 1)return 0;
    IIC_Send_Byte(reg);
    if(IIC_Wait_Ack() == 1)return 0;
    IIC_Start();
    IIC_Send_Byte(dev+1); 
    if(IIC_Wait_Ack() == 1)return 0;

    for(count=0; count<length; count++)
    {
        if(count!=length-1)data[count]=IIC_Read_Byte(1);
        else  data[count]=IIC_Read_Byte(0);	 
    }
    IIC_Stop();
    return 1;
}


int32_t IICwriteBytes(uint8_t dev, uint8_t reg, uint8_t* data, uint32_t length)
{
    uint32_t count = 0;
    IIC_Start();
    IIC_Send_Byte(dev);	   
    if(IIC_Wait_Ack() == 1)return 0;
    IIC_Send_Byte(reg);   
    if(IIC_Wait_Ack() == 1)return 0;
    for(count=0; count<length; count++)
    {
        IIC_Send_Byte(data[count]);
        if(IIC_Wait_Ack() == 1)return 0;
    }
    IIC_Stop();

    return 1; 
}
