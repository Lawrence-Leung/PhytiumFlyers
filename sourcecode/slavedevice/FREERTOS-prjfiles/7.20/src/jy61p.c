#include <stdio.h>
#include "gpio_init.h"
#include "wit_c_sdk.h"
#include "jy61p.h"
#include "jy61p_i2c.h"
#include "fsleep.h"

#define ACC_UPDATE		0x01
#define GYRO_UPDATE		0x02
#define ANGLE_UPDATE	0x04
#define MAG_UPDATE		0x08
#define READ_UPDATE		0x80
static volatile char s_cDataUpdate = 0, s_cCmd = 0xff;

static void CmdProcess(void);
static void AutoScanSensor(void);
static void CopeSensorData(uint32_t uiReg, uint32_t uiRegNum);
static void Delayms(uint16_t ucMs);
static void ShowHelp(void);

float fAcc[3], fGyro[3], fAngle[3];
u8 jy61pLock = 0;
// float JY61P_datapack[12] = {0};
float JY61P_datapack1[6] = {0};
float JY61P_datapack2[6] = {0};
float JY61P_datapack3[6] = {0};

extern SemaphoreHandle_t xMutexJY61P;

void jy61pInit(void)
{
	IIC_Init();
}

void jy61pTask(void)
{
	while (1)
	{
		// if (xSemaphoreTake(xMutexJY61P, portMAX_DELAY) == pdTRUE)
		// {
			jy61pFunc(fAcc,fGyro,fAngle);
		// 	xSemaphoreGive(xMutexJY61P);
		// }
		vTaskDelay(900 / portTICK_PERIOD_MS);
	}
}

void jy61pFunc(float *fAcc ,float *fGyro ,float *fAngle)
{
    
	int i;
    WitInit(WIT_PROTOCOL_I2C, 0x50);
	WitI2cFuncRegister(IICwriteBytes, IICreadBytes);
	WitRegisterCallBack(CopeSensorData);
	WitDelayMsRegister(Delayms);
    AutoScanSensor();
    WitReadReg(AX, 12);
    // // delay_ms(500);
    fsleep_millisec(400);
    // CmdProcess();
    if(s_cDataUpdate)
    {
        for(i = 0; i < 3; i++)
        {
            fAcc[i] = sReg[AX+i] / 32768.0f * 16.0f;
            fGyro[i] = sReg[GX+i] / 32768.0f * 2000.0f;
            fAngle[i] = sReg[Roll+i] / 32768.0f * 180.0f;
        }
        if(s_cDataUpdate & ACC_UPDATE)
        {
            printf("acc:%.3f %.3f %.3f\r\n", fAcc[0], fAcc[1], fAcc[2]);
            s_cDataUpdate &= ~ACC_UPDATE;
        }
        if(s_cDataUpdate & GYRO_UPDATE)
        {
            printf("gyro:%.3f %.3f %.3f\r\n", fGyro[0], fGyro[1], fGyro[2]);
            s_cDataUpdate &= ~GYRO_UPDATE;
        }
        if(s_cDataUpdate & ANGLE_UPDATE)
        {
            printf("angle:%.3f %.3f %.3f\r\n", fAngle[0], fAngle[1], fAngle[2]);
            s_cDataUpdate &= ~ANGLE_UPDATE;
        }
    }
}

// void JY61PPackData(float *fAcc ,float *fGyro ,float *fAngl,float *pack)
// {
//     int i,j;
// 	if (jy61pLock == 0) {
// 		jy61pLock = 1;
// 		for(i = 0; i < 3; i++)
//         {
//             printf("acc:%.3f %.3f %.3f\r\n", fAcc[0], fAcc[1], fAcc[2]);
//             printf("gyro:%.3f %.3f %.3f\r\n", fGyro[0], fGyro[1], fGyro[2]);
//             printf("angle:%.3f %.3f %.3f\r\n", fAngle[0], fAngle[1], fAngle[2]);
//         }
// 		// pack[0] = 0xA0;	//固定字头
//         pack[0] = 160;
//         for(i=0;i<3;i++)
//             pack[1+i] = fAcc[i];
//         for (int i = 0; i < 3; i++)
//             pack[1+3+i] = fGyro[i];
//         for (int i = 0; i < 3; i++)
//             pack[1+3+3+i] = fAngle[i];
        
//         for(i=1;i<11;i++)
//             printf("pack[%d]:%.3f\r\n",i,pack[i]);
//         // pack[9] = GPSCalculateCheck(latitude, longitude); //校验码
//         // pack[11] = 0x5A; //固定字尾
//         pack[11] = 90;
// 		jy61pLock = 0;
// 	}
// 	else {
// 		f_printk("Data Locked! \r\n");
// 	}

// 	// for debug only
// 	f_printk("Data sent: ");
//     for (int i = 0; i < 12; i++) {
//         f_printk("%08X ", (int32_t)pack[i]);
//     }
//     f_printk("\r\n");
// }

/*
 *浮点数转十六进制
*/
static void FloatToByte(float floatNum,u8* byteoutputy)
{
    char* pchar = (char*)&floatNum;
    for(int i=0; i < sizeof(float); i++)
    {
        *byteoutputy = *pchar;
        pchar++;
        byteoutputy++;
    }
}


//三个数据包
void JY61PPackData(float *fAcc ,float *fGyro ,float *fAngl,float *pack1,float *pack2,float *pack3)
{
    int i,j;
	if (jy61pLock == 0) {
		jy61pLock = 1;
		// printf("acc:%.3f %.3f %.3f\r\n", fAcc[0], fAcc[1], fAcc[2]);
		// printf("gyro:%.3f %.3f %.3f\r\n", fGyro[0], fGyro[1], fGyro[2]);
		// printf("angle:%.3f %.3f %.3f\r\n", fAngl[0], fAngl[1], fAngl[2]);
		// pack[0] = 0xA0;	//固定字头
        pack1[0] = 160;
		pack2[0] = 160;
		pack3[0] = 160;
        for(i=0;i<3;i++)
            pack1[1+i] = fAcc[i];
        for (int i = 0; i < 3; i++)
            pack2[1+i] = fGyro[i];
        for (int i = 0; i < 3; i++)
            pack3[1+i] = fAngl[i];
		pack1[4] = 1;
		pack2[4] = 2;
		pack3[4] = 3;
        pack1[5] = 90;
		pack2[5] = 90;
		pack3[5] = 90;
        // for(i=1;i<11;i++)
        //     printf("pack[%d]:%.3f\r\n",i,pack[i]);
        // pack[9] = GPSCalculateCheck(latitude, longitude); //校验码
        // pack[11] = 0x5A; //固定字尾
        // pack[11] = 90;
		jy61pLock = 0;
	}
	else {
		f_printk("Data Locked! \r\n");
	}

	// for debug only
	// f_printk("Data sent: ");
    // for (int i = 0; i < 6; i++) {
    //     f_printk("%08X ", FloatToByte(*pack1[i]));
    // }
    // f_printk("\r\n");
}

/*
 * 发送16进制的数据包

*/
// void JY61PPackData(float *fAcc ,float *fGyro ,float *fAngl,u8 *pack1,u8 *pack2,u8 *pack3)
// {
//     int i,j;
// 	if (jy61pLock == 0) {
// 		jy61pLock = 1;
// 		for(i = 0; i < 3; i++)
//         {
//             printf("acc:%.3f %.3f %.3f\r\n", fAcc[0], fAcc[1], fAcc[2]);
//             printf("gyro:%.3f %.3f %.3f\r\n", fGyro[0], fGyro[1], fGyro[2]);
//             printf("angle:%.3f %.3f %.3f\r\n", fAngle[0], fAngle[1], fAngle[2]);
//         }
// 		// pack[0] = 0xA0;	//固定字头
//         pack1[0] = 160;
// 		pack2[0] = 160;
// 		pack3[0] = 160;
//         for(i=0;i<3;i++)
//             pack1[1+i] = fAcc[i]*1000;
//         for (int i = 0; i < 3; i++)
//             pack2[1+i] = fGyro[i]*1000;
//         for (int i = 0; i < 3; i++)
//             pack3[1+i] = fAngle[i]*1000;
// 		pack1[4] = 1;
// 		pack2[4] = 2;
// 		pack3[4] = 3;
//         pack1[5] = 90;
// 		pack2[5] = 90;
// 		pack3[5] = 90;
//         // for(i=1;i<11;i++)
//         //     printf("pack[%d]:%.3f\r\n",i,pack[i]);
//         // pack[9] = GPSCalculateCheck(latitude, longitude); //校验码
//         // pack[11] = 0x5A; //固定字尾
//         // pack[11] = 90;
// 		jy61pLock = 0;
// 	}
// 	else {
// 		f_printk("Data Locked! \r\n");
// 	}

// 	// for debug only
// 	// f_printk("Data sent: ");
//     // for (int i = 0; i < 12; i++) {
//     //     f_printk("%08X ", (int32_t)pack[i]);
//     // }
//     // f_printk("\r\n");
// }


static void CmdProcess(void)
{
	switch(s_cCmd)
	{
		case 'a':	
				if(WitStartAccCali() != WIT_HAL_OK) 
					printf("\r\nSet AccCali Error\r\n");
			break;
		case 'm':	
				if(WitStartMagCali() != WIT_HAL_OK) 
					printf("\r\nStart MagCali Error\r\n");
			break;
		case 'e':	
				if(WitStopMagCali() != WIT_HAL_OK) 
					printf("\r\nEnd MagCali Error\r\n");
			break;
		case 'u':	
				if(WitSetBandwidth(BANDWIDTH_5HZ) != WIT_HAL_OK) 
					printf("\r\nSet Bandwidth Error\r\n");
			break;
		case 'U':	
				if(WitSetBandwidth(BANDWIDTH_256HZ) != WIT_HAL_OK)
					printf("\r\nSet Bandwidth Error\r\n");
			break;
		case 'B':	
				if(WitSetUartBaud(WIT_BAUD_115200) != WIT_HAL_OK) 
					printf("\r\nSet Baud Error\r\n");
			break;
		case 'b':	
				if(WitSetUartBaud(WIT_BAUD_9600) != WIT_HAL_OK) 
					printf("\r\nSet Baud Error\r\n");
			break;
		case 'h':	
				ShowHelp();
			break;
		default : return ;
	}
	s_cCmd = 0xff;
}

static void ShowHelp(void)
{
	// printf("\r\n************************	 WIT_SDK_DEMO	************************");
	// printf("\r\n************************          HELP           ************************\r\n");
	// printf("UART SEND:a\\r\\n   Acceleration calibration.\r\n");
	// printf("UART SEND:m\\r\\n   Magnetic field calibration,After calibration send:   e\\r\\n   to indicate the end\r\n");
	// printf("UART SEND:U\\r\\n   Bandwidth increase.\r\n");
	// printf("UART SEND:u\\r\\n   Bandwidth reduction.\r\n");
	// printf("UART SEND:B\\r\\n   Baud rate increased to 115200.\r\n");
	// printf("UART SEND:b\\r\\n   Baud rate reduction to 9600.\r\n");
	// printf("UART SEND:h\\r\\n   help.\r\n");
	// printf("******************************************************************************\r\n");
}

// void CopeCmdData(unsigned char ucData)
// {
// 	static unsigned char s_ucData[50], s_ucRxCnt = 0;
	
// 	s_ucData[s_ucRxCnt++] = ucData;
// 	if(s_ucRxCnt<3)return;										//Less than three data returned
// 	if(s_ucRxCnt >= 50) s_ucRxCnt = 0;
// 	if(s_ucRxCnt >= 3)
// 	{
// 		if((s_ucData[1] == '\r') && (s_ucData[2] == '\n'))
// 		{
// 			s_cCmd = s_ucData[0];
// 			memset(s_ucData,0,50);//
// 			s_ucRxCnt = 0;
// 		}
// 		else 
// 		{
// 			s_ucData[0] = s_ucData[1];
// 			s_ucData[1] = s_ucData[2];
// 			s_ucRxCnt = 2;
			
// 		}
// 	}

// }

static void CopeSensorData(uint32_t uiReg, uint32_t uiRegNum)
{
	int i;
    for(i = 0; i < uiRegNum; i++)
    {
        switch(uiReg)
        {
//            case AX:
//            case AY:
            case AZ:
				s_cDataUpdate |= ACC_UPDATE;
            break;
//            case GX:
//            case GY:
            case GZ:
				s_cDataUpdate |= GYRO_UPDATE;
            break;
//            case HX:
//            case HY:
            case HZ:
				s_cDataUpdate |= MAG_UPDATE;
            break;
//            case Roll:
//            case Pitch:
            case Yaw:
				s_cDataUpdate |= ANGLE_UPDATE;
            break;
            default:
				s_cDataUpdate |= READ_UPDATE;
			break;
        }
		uiReg++;
    }
}

static void Delayms(uint16_t ucMs)
{
	fsleep_millisec(ucMs);
}

static void AutoScanSensor(void)
{
	int i, iRetry;
	
	for(i = 0; i < 0x7F; i++)
	{
		WitInit(WIT_PROTOCOL_I2C, i);
		iRetry = 2;
		do
		{
			s_cDataUpdate = 0;
			WitReadReg(AX, 3);
			fsleep_millisec(5);
			if(s_cDataUpdate != 0)
			{
				printf("find %02X addr sensor\r\n", i);
				// ShowHelp();
				return ;
			}
			iRetry--;
		}while(iRetry);		
	}
	printf("can not find sensor\r\n");
	printf("please check your connection\r\n");
}


