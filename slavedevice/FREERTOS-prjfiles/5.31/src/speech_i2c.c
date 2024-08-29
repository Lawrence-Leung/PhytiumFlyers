#include "speech_i2c.h"
#include "FreeRTOS.h"
#include "fgpio_os.h"
#include "gpio_init.h"
#include "fsleep.h"
#include <string.h>

u8 ackflag = 0;            


static void IIC_Init(void)
{			
    // Jy61pSclInitOut();
    // Jy61pSdaInitOut();

	GpioInit(SPEECH_SDA_INDEX,&speech_sda_gpio, speech_sda_cfg, SPEECH_SDA_config);
	GpioInit(SPEECH_SCL_INDEX,&speech_scl_gpio, speech_scl_cfg, SPEECH_SCL_config);
	
	SDA_OUT(speech_sda_gpio,SPEECH_SDA_config);
    SPEECH_SDA_HIGH;
    SPEECH_SCL_HIGH;
}

void SpeechInit(void)
{
	IIC_Init();					//?????
	SetVolume(1);				//??
	SetReader(Reader_XiaoPing);	//???
}

static void IIC_Start(void)
{
	SDA_OUT(speech_sda_gpio,SPEECH_SDA_config); 
	SPEECH_SDA_HIGH;

    fsleep_microsec(1);

	SPEECH_SCL_HIGH;

    fsleep_microsec(5);

 	SPEECH_SDA_LOW;

    fsleep_microsec(5);

	SPEECH_SCL_LOW;

    fsleep_microsec(2);
}

// void IIC_Start(void)
// {
// 	SDA_OUT();    
// 	SPEECH_SDA_HIGH;

//     fsleep_microsec(1);

// 	SPEECH_SCL_HIGH;

//     fsleep_microsec(4);

//  	SPEECH_SDA_LOW;

//     fsleep_microsec(4);

// 	SPEECH_SCL_LOW;

//     fsleep_microsec(2);
// }
	  
static void IIC_Stop(void)
{
	SDA_OUT(speech_sda_gpio,SPEECH_SDA_config);
	// SPEECH_SCL_LOW;
	SPEECH_SDA_LOW;//STOP:when CLK is high DATA change form low to high
 	
    fsleep_microsec(1);
	SPEECH_SCL_HIGH; 
    fsleep_microsec(5);
	SPEECH_SDA_HIGH;
	
    // Delay(5);
    fsleep_microsec(4);							   	
}


static void IIC_Send_Byte(u8 txd)
{                        
    u8 t; 
    SDA_OUT(speech_sda_gpio,SPEECH_SDA_config);
    for(t=0;t<8;t++)
    {
        if((txd<<t)&0x80)
        {
            SPEECH_SDA_HIGH;
        }
        else
        {
            SPEECH_SDA_LOW;
        }

        fsleep_microsec(3);

		SPEECH_SCL_HIGH;

        fsleep_microsec(3);  

		SPEECH_SCL_LOW;
    }	 

	fsleep_microsec(4);

	SPEECH_SDA_HIGH;
	// SDA_IN();					//����Ϊ���� 1us
	SDA_IN(speech_sda_gpio,SPEECH_SDA_config);
	fsleep_microsec(3);			//�ܹ� 4us

	SPEECH_SCL_HIGH;

	fsleep_microsec(3);
	
	if(SPEECH_READ_SDA == 1)
	{
		ackflag = 0;
		// printf("sda read high\r\n");
	}
	else
	{
		ackflag = 1;
		// printf("sda read low\r\n");
	}
	
	SPEECH_SCL_LOW;	
	fsleep_microsec(1);
	SDA_OUT(speech_sda_gpio,SPEECH_SDA_config);
}


static u8 IIC_Read_Byte()
{
	unsigned char i,receive=0;
	SPEECH_SDA_HIGH;
	// SDA_IN();
	SDA_IN(speech_sda_gpio,SPEECH_SDA_config);
    for(i=0;i<8;i++ )
	{
        fsleep_microsec(1);
        SPEECH_SCL_LOW; 
        
		// Delay(5);
        fsleep_microsec(5);
		SPEECH_SCL_HIGH;
        fsleep_microsec(2);
        receive<<=1;
        if(SPEECH_READ_SDA)receive++;   
		
		// Delay(5); 
        fsleep_microsec(2);
    }					 
    SPEECH_SCL_LOW;
    fsleep_microsec(20);
    return receive;
}

static void IIC_Ack(u8 a)
{
	SDA_OUT(speech_sda_gpio,SPEECH_SDA_config);
    if (a == 0)
    {
        SPEECH_SDA_LOW;
    }
    else
    {
        SPEECH_SDA_HIGH;
    }
    fsleep_microsec(3);  
	SPEECH_SCL_HIGH;
    fsleep_microsec(5);  
	SPEECH_SCL_LOW;
	fsleep_microsec(2);  
}

static u8 I2C_ByteWrite(u8 dat)
{
   IIC_Start();              //坯动总线
   IIC_Send_Byte(I2C_ADDR);            //坑逝器件地址
   if(ackflag==0)return(0);
   IIC_Send_Byte(dat);            //坑逝数�? 
   if(ackflag==0)return(0);
   IIC_Stop();               //结束总线
   fsleep_millisec(10);
   return(1);
}

static void I2C_Writes_Bytes(u8 *buff,int number)
{
	int i;
	for(i = 0;i < number;i++)
	{
		I2C_ByteWrite(buff[i]);
	}
}

u8 GetChipStatus()
{
  	u8 dat = 0xff;
	u8* pBuffer = (u8*)0xff;
  	u8 AskState[4] = {0xFD,0x00,0x01,0x21};

	I2C_Writes_Bytes(AskState,4);
	fsleep_millisec(100);

   	IIC_Start();          			//坯动总线
   	IIC_Send_Byte(I2C_ADDR+1);      //坑逝器件地址
	if(ackflag==0)
	{
		return 0;
	}
	
	fsleep_millisec(5);//必须加一点延�?
   	dat=IIC_Read_Byte();          //读坖数杮

   	IIC_Ack(1);           //坑逝非应答信坷
   	IIC_Stop();           //结束总线
   	return(dat);  
}

void speech_text(u8 *str,u8 encoding_format)
{
	 u16 size = strlen(str) + 2;
	 
	 XFS_Protocol_TypeDef DataPack;
	
	 DataPack.DataHead = DATAHEAD;
	 DataPack.Length_HH = size >>8;
	 DataPack.Length_LL = size;
	 DataPack.Commond = 0x01;
	 DataPack.EncodingFormat = encoding_format;
	 DataPack.Text = str;
	 
	 I2C_ByteWrite(DataPack.DataHead );
	 I2C_ByteWrite(DataPack.Length_HH);	
	 I2C_ByteWrite(DataPack.Length_LL);
	 I2C_ByteWrite(DataPack.Commond);	
	 I2C_ByteWrite(DataPack.EncodingFormat);		
	
	 I2C_Writes_Bytes(DataPack.Text,strlen(DataPack.Text));	 
}

void speech(u8* data)
{
	if(GetChipStatus() == ChipStatus_Idle)  //等待芯片空闲
	{
		speech_text(data,GBK);
	}

}

void SetBase(u8 *str)
{
	 u16 size = strlen(str) + 2;
	 
	 XFS_Protocol_TypeDef DataPack;
	
	 DataPack.DataHead = DATAHEAD;
	 DataPack.Length_HH = size >>8;
	 DataPack.Length_LL = size;
	 DataPack.Commond = 0x01;
	 DataPack.EncodingFormat = 0x00;
	 DataPack.Text = str;
	 
	 I2C_ByteWrite(DataPack.DataHead );
	 I2C_ByteWrite(DataPack.Length_HH);	
	 I2C_ByteWrite(DataPack.Length_LL);
	 I2C_ByteWrite(DataPack.Commond);	
	 I2C_ByteWrite(DataPack.EncodingFormat);
	
	 I2C_Writes_Bytes(DataPack.Text,strlen(DataPack.Text));
}

void TextCtrl(char c, int d)
{
  char str[10];
  if (d != -1)
    sprintf(str, "[%c%d]", c, d);
  else
    sprintf(str, "[%c]", c);
  SetBase(str);
}

void SetStyle(Style_Type style)
{
  TextCtrl('f', style);
  	while(GetChipStatus() != ChipStatus_Idle)
	{
	  fsleep_microsec(10);
	}
}

void SetLanguage(Language_Type language)
{
	  TextCtrl('g', language);
	 while(GetChipStatus() != ChipStatus_Idle)
	{
	  fsleep_microsec(10);
	}
}

void SetArticulation(Articulation_Type articulation)
{
  TextCtrl('h', articulation);
  	while(GetChipStatus() != ChipStatus_Idle)
	{
	  fsleep_microsec(10);
	}
}

void SetSpell(Spell_Type spell)
{
  TextCtrl('i', spell);
  	while(GetChipStatus() != ChipStatus_Idle)
	{
	  fsleep_microsec(10);
	}
}

void SetReader(Reader_Type reader)
{
  TextCtrl('m', reader);
  	while(GetChipStatus() != ChipStatus_Idle)
	{
	  fsleep_microsec(10);
	}
}

void SetNumberHandle(NumberHandle_Type numberHandle)
{
  TextCtrl('n', numberHandle);
  	while(GetChipStatus() != ChipStatus_Idle)
	{
	  fsleep_microsec(10);
	}
}

void SetZeroPronunciation(ZeroPronunciation_Type zeroPronunciation)
{
  TextCtrl('o', zeroPronunciation);
  	while(GetChipStatus() != ChipStatus_Idle)
	{
	  fsleep_microsec(10);
	}
}



void SetNamePronunciation(NamePronunciation_Type namePronunciation)
{
  TextCtrl('r', namePronunciation);
  	while(GetChipStatus() != ChipStatus_Idle)
	{
	  fsleep_microsec(10);
	}
}

void SetSpeed(int speed)
{
  TextCtrl('s', speed);
}

void SetIntonation(int intonation)
{
  TextCtrl('t', intonation);
  	while(GetChipStatus() != ChipStatus_Idle)
	{
	  fsleep_microsec(10);
	}
}

void SetVolume(int volume)
{
  TextCtrl('v', volume);
  	while(GetChipStatus() != ChipStatus_Idle)
	{
	  fsleep_microsec(10);
	}
}

void SetOnePronunciation(OnePronunciation_Type onePronunciation)
{
  TextCtrl('y', onePronunciation);
  	while(GetChipStatus() != ChipStatus_Idle)
	{
	  fsleep_microsec(10);
	}
}

void SetRhythm(Rhythm_Type rhythm)
{
  TextCtrl('z', rhythm);
  	while(GetChipStatus() != ChipStatus_Idle)
	{
	  fsleep_microsec(10);
	}
}

void SetRestoreDefault()
{
  TextCtrl('d', -1);
  	while(GetChipStatus() != ChipStatus_Idle)
	{
	  fsleep_microsec(10);
	}
}



