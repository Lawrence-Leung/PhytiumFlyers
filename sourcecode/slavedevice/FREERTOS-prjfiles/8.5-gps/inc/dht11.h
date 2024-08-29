/*
    dht11.h
    DHT11 温度传感器 驱动程序
    by 周子琳, Lawrence Leung
    2024 飞腾风驰队
*/

#ifndef  DHT11_H
#define  DHT11_H

#include <stdio.h>
#include "fgpio_os.h"

void DHT11IoInitOut(void);
void DHT11IoInitIn(void);
void DHT11Rst(void);
u8 DHT11Check(void);
u8 DHT11ReadBit(void);
u8 DHT11ReadByte(void);
u8 DHT11ReadData(u8 *temp, u8 *humi);
u8 CalculateCheck(u8 *temp, u8 *humi);
void DHT11CircularRead(void);
void DHT11PackData (u8 *temp, u8 *humi, u8* data_frame);
BaseType_t DHT11Task(void);

#endif

