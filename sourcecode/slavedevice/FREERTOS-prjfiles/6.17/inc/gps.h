
#ifndef  SERIAL_INTR_EXAMPLE_H
#define  SERIAL_INTR_EXAMPLE_H
/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>

#include "fcpu_info.h"
#include "finterrupt.h"
#include "ftypes.h"
#if defined(CONFIG_TARGET_E2000)
#endif
#include "fparameters.h"

#include "fpl011.h"

#ifdef __cplusplus
extern "C"
{
#endif
/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
/* entry function for serial interrupt example */
// FError FSerialIntrInit(u32 id, FPl011Format *format, FPl011 *uart_p);
// int FSerialIntrExample(void);

#define false 0
#define true 1

//定义数组长度
#define GPS_Buffer_Length 80
#define UTCTime_Length 11
#define latitude_Length 11
#define N_S_Length 2
#define longitude_Length 12
#define E_W_Length 2 

typedef struct SaveData 
{
	char GPS_Buffer[GPS_Buffer_Length];
	char isGetData;		//是否获取到GPS数据
	char isParseData;	//是否解析完成
	char UTCTime[UTCTime_Length];		//UTC时间
	char latitude[latitude_Length];		//纬度
	char N_S[N_S_Length];		//N/S
	char longitude[longitude_Length];		//经度
	char E_W[E_W_Length];		//E/W
	char isUsefull;		//定位信息是否有效
	char isPackable;	//是否可以打包数据
} _SaveData;


extern char rxdatabufer;
extern u16 point1;
extern _SaveData Save_Data;

void CLR_Buf(void);
u8 Hand(char *a);
void clrStruct(void);
void parseGpsBuffer();
void printGpsBuffer(double *latitude, double *longitude);
int GPSCalculateCheck(int latitude, int longitude);
void GPSPackData(double *latitude, double *longitude, u8 *pack);

#ifdef __cplusplus
}
#endif

#endif