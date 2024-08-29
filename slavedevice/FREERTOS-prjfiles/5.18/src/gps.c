#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fsleep.h"
#include "ftypes.h"

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#ifdef CONFIG_TARGET_E2000
#include "fio_mux.h"
#endif

#include "gps.h"

/***************** Macros (Inline Functions) Definitions *********************/
#define INTR_TEST_BUFFER_SIZE   10000
/************************** Constant Definitions *****************************/
static u8 send_buffer_i[INTR_TEST_BUFFER_SIZE]; /*intr Buffer for Transmitting Data */
static u8 recv_buffer_i[INTR_TEST_BUFFER_SIZE]; /*intr Buffer for Receiving Data */
static volatile int total_received_count;
static volatile int total_sent_count;
static volatile int total_error_count;

double latitude;
double longitude;
u8 GPS_data_frame_lock;	 // 打包好的数据使用的锁
u8 GPS_datapack[14] = {0};

void errorLog(int num)
{
	while (1)
	{
	  	printf("ERROR%d\r\n",num);
	}
}

//数据解析
void parseGpsBuffer()
{
	char *subString;
	char *subStringNext;
	char i = 0;
	if (Save_Data.isGetData)
	{
		Save_Data.isGetData = false;
		printf("**************\r\n");
		printf(Save_Data.GPS_Buffer);

		
		for (i = 0 ; i <= 6 ; i++)
		{
			if (i == 0)
			{
				if ((subString = strstr(Save_Data.GPS_Buffer, ",")) == NULL)
					errorLog(1);	//解析错误
			}
			else
			{
				subString++;
				if ((subStringNext = strstr(subString, ",")) != NULL)
				{
					char usefullBuffer[2]; 
					switch(i)
					{
						case 1:memcpy(Save_Data.UTCTime, subString, subStringNext - subString);break;	//获取UTC时间
						case 2:memcpy(usefullBuffer, subString, subStringNext - subString);break;	//获取UTC时间
						case 3:memcpy(Save_Data.latitude, subString, subStringNext - subString);break;	//获取纬度信息
						case 4:memcpy(Save_Data.N_S, subString, subStringNext - subString);break;	//获取N/S
						case 5:memcpy(Save_Data.longitude, subString, subStringNext - subString);break;	//获取经度信息
						case 6:memcpy(Save_Data.E_W, subString, subStringNext - subString);break;	//获取E/W

						default:break;
					}

					subString = subStringNext;
					Save_Data.isParseData = true;
					if(usefullBuffer[0] == 'A')
						Save_Data.isUsefull = true;
					else if(usefullBuffer[0] == 'V')
						Save_Data.isUsefull = false;

				}
				else
				{
					errorLog(2);	//解析错误
				}
			}
		}
	}
}

// GPS数据转化单位为度。
double Convert_to_degrees(char* data)
{
	double temp_data = atof(data);
	int degree = (int)(temp_data / 100);
	double f_degree = (temp_data / 100.0 - degree)*100/60.0;
	double result = degree + f_degree;
	return result;
}


void printGpsBuffer(double *latitude, double *longitude)
{
	double f_latitude = 0.0;
	double f_longitude = 0.0;
	
	if (Save_Data.isParseData)
	{
		Save_Data.isParseData = false;
		
		printf("Save_Data.UTCTime = ");
		printf(Save_Data.UTCTime);
		printf("\r\n");

		if(Save_Data.isUsefull)
		{
			Save_Data.isUsefull = false;
			f_printk("Save_Data.latitude = ");
			// printf(Save_Data.latitude);
			// printf("--");
			f_latitude = Convert_to_degrees(Save_Data.latitude);
			printf("%f %s", f_latitude, Save_Data.N_S);
			printf("\r\n");

			printf("Save_Data.N_S = ");
			printf(Save_Data.N_S);
			printf("\r\n");

			printf("Save_Data.longitude = ");
			// printf(Save_Data.longitude);
			// printf("--");
			f_longitude = Convert_to_degrees(Save_Data.longitude);
			printf("%f %s", f_longitude, Save_Data.E_W);
			printf("\r\n");

			printf("Save_Data.E_W = ");
			printf(Save_Data.E_W);
			printf("\r\n");
			
			*latitude = f_latitude;
			*longitude = f_longitude;
			// strcpy(Save_Data.N_S,"S");
			// if(strcmp(Save_Data.N_S,"S") == 0)
			// 	*latitude = -*latitude;
			// if(strcmp(Save_Data.E_W,"W") == 0)
			// 	*longitude = -*longitude;
			printf("%f\t%f\r\n",*latitude,*longitude);
		}
		else
		{
			printf("GPS DATA is not usefull!\r\n");
		}
		
	}
}

// u8 GPSCalculateCheck(double *latitude, double *longitude)
// {
// 	return (*latitude + *longitude) % 256;
// }

#define VALID_RANGE_BIT 0x01
#define VALID_LONGITUDE_BIT 0x02
#define VALID_LATITUDE_BIT 0x04
#define NAN_LONGITUDE_BIT 0x08
#define NAN_LATITUDE_BIT 0x10
#define POS_INF_LONGITUDE_BIT 0x20
#define POS_INF_LATITUDE_BIT 0x40
#define NEG_INF_LONGITUDE_BIT 0x80
#define NEG_INF_LATITUDE_BIT 0x100

// u8 GPSCalculateCheck(double *latitude, double *longitude) {
//     u8 code = 0;

//     if (longitude >= -180.0 && longitude <= 180.0) {
//         code |= VALID_RANGE_BIT;
//         if (!isnan(longitude)) code |= VALID_LONGITUDE_BIT;
//         if (isinf(longitude)) {
//             if (longitude > 0) code |= POS_INF_LONGITUDE_BIT;
//             else code |= NEG_INF_LONGITUDE_BIT;
//         }
//     }

//     if (latitude >= -90.0 && latitude <= 90.0) {
//         code |= VALID_RANGE_BIT;
//         if (!isnan(latitude)) code |= VALID_LATITUDE_BIT;
//         if (isinf(latitude)) {
//             if (latitude > 0) code |= POS_INF_LATITUDE_BIT;
//             else code |= NEG_INF_LATITUDE_BIT;
//         }
//     }

//     return code;
// }

int GPSCalculateCheck(int latitude, int longitude) {
	int code = latitude + longitude;
	if(latitude >= -90*1000000 && latitude <= 90*1000000)
		if(latitude > 0)
				code += 100;
	if(longitude >= -180*1000000 && longitude <= 180*1000000)
		if(longitude > 0)
			code += 10;
	return code;
}


void GPSPackData(double *latitude, double *longitude, u8 *pack)
{
	if (GPS_data_frame_lock == 0) {
		GPS_data_frame_lock = 1;
		printf("lat:%lf\tlon:%lf",latitude,longitude);
		f_printk("lat:%lf\tlon:%lf",latitude,longitude);
		pack[0] = 0xA0;	//固定字头
		 // Convert latitude and longitude to integer representation
        int lat = (int)(*latitude * 1000000); // Assuming latitude is in decimal degrees
        int lon = (int)(*longitude * 1000000); // Assuming longitude is in decimal degrees
		int code = 0;
        // Store integer representations in pack
        pack[1] = (u8)(lat >> 24); // Most significant byte of latitude
        pack[2] = (u8)(lat >> 16);
        pack[3] = (u8)(lat >> 8);
        pack[4] = (u8)lat; // Least significant byte of latitude

        pack[5] = (u8)(lon >> 24); // Most significant byte of longitude
        pack[6] = (u8)(lon >> 16);
        pack[7] = (u8)(lon >> 8);
        pack[8] = (u8)lon; // Least significant byte of longitude

        code = GPSCalculateCheck(lat, lon); //校验码
		pack[9] = (u8)(code >> 24); // Most significant byte of longitude
        pack[10] = (u8)(code >> 16);
        pack[11] = (u8)(code >> 8);
        pack[12] = (u8)code; // Least significant byte of longitude

        pack[13] = 0x5A; //固定字尾
		GPS_data_frame_lock = 0;
	}
	else {
		f_printk("Data Locked! \r\n");
	}

	// for debug only
	f_printk("Data sent: ");
    for (int i = 0; i < 14; i++) {
        f_printk("%02X ", (u8)pack[i]);
    }
    f_printk("\r\n");
}