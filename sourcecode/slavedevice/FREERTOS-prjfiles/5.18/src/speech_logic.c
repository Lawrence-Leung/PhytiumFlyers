#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "ftypes.h"
#include "speech_i2c.h"
#include "speech_logic.h"

Object *objectList = NULL;
ZebraLine *zebraLineList = NULL;
TrafficLight *trafficLightList = NULL;
Stairs *stairsList = NULL;

unsigned char ObjectNum = 0;
unsigned char ZebraLineNum = 0;
unsigned char TrafficLightNum = 0;
unsigned char StairsNum = 0;

// 解析数据
int parse_data(char *data)
{
    size_t start_idx = 0;
    size_t length = sizeof(data);

    /* 完整帧 */
    if (data[0] == 0xAA && data[length - 1] == 0xBB)
    {
        ObjectNum = data[1];
        ZebraLineNum = data[2];
        TrafficLightNum = data[3];
        StairsNum = data[4];
    }
    else
    {
        return 0;
    }

    objectList = malloc(ObjectNum * sizeof(Object));
    zebraLineList = malloc(ZebraLineNum * sizeof(ZebraLine));
    trafficLightList = malloc(TrafficLightNum * sizeof(TrafficLight));
    stairsList = malloc(StairsNum * sizeof(Stairs));
    for (size_t i = 5; i < length; i++)
    {
        /* 物体帧 */
        if (data[i] == 0x1A)
        {
            if (data[i + 10] == 0x1B) // 判断包尾是否为0x1B
            {
                for (size_t j = 0; j < ObjectNum; j++)
                {
                    objectList[j].x = data[i + 1] << 8 | data[i + 2];
                    objectList[j].y = data[i + 3] << 8 | data[i + 4];
                    objectList[j].distance = (float)data[i + 6] / 10.0f;
                    objectList[j].ver = data[i + 7];
                    objectList[j].hor = (float)((data[i + 8] << 8) | data[i + 9]) / 10.0f;
                    i += 10; // 跳到第10个字节
                }
            }
        }
        /* 斑马线帧 */
        else if (data[i] == 0x2A)
        {
            for (size_t j = 0; j < ZebraLineNum; j++)
            {
                if (data[i + 7] == 0x2B)
                {
                    zebraLineList[j].x = (data[i + 1] << 8 | data[i + 2]);
                    zebraLineList[j].y = (data[i + 3] << 8 | data[i + 4]);
                    zebraLineList[j].deg = (data[i + 5] << 8 | data[i + 6]);
                    i += 7;
                }
            }
        }
        /* 交通灯帧 */
        else if (data[i] == 0x3A)
        {
            for (size_t j = 0; j < TrafficLightNum; j++)
            {
                if (data[i + 6] == 0x3B)
                {
                    trafficLightList[j].x = (data[i + 1] << 8 | data[i + 2]);
                    trafficLightList[j].y = (data[i + 3] << 8 | data[i + 4]);
                    trafficLightList[j].lightstatus = data[i + 5];
                    i += 6;
                }
            }
        }
        /* 楼梯帧 */
        else if (data[i] == 0x4A)
        {
            for (size_t j = 0; j < StairsNum; j++)
            {
                if (data[i + 6] == 0x4B)
                {
                    stairsList[j].x = (data[i + 1] << 8 | data[i + 2]);
                    stairsList[j].y = (data[i + 3] << 8 | data[i + 4]);
                    stairsList[j].stairs_numbers = data[i + 5];
                    i += 6;
                }
            }
        }
    }
    return 0;
}

int speechObject(Object *objectList)
{

    uint8_t dist1, dist2, dist3, dist4 = 0;
    bool is_total_crowded = false;
    uint8_t most_crowded = 0;

    if (objectList[0].distance >= 0 && objectList[0].distance <= 2.5)
        dist1 += 1;
    else if (objectList[0].distance >= 2.5 && objectList[0].distance <= 5)
        dist2 += 1;
    else if (objectList[0].distance >= 5 && objectList[0].distance <= 7.5)
        dist3 += 1;
    else if (objectList[0].distance >= 7.5 && objectList[0].distance <= 10)
        dist4 += 1;

    u8 total_objects = dist1 + dist2 + dist3 + dist4;
    if (total_objects >= 10)
        is_total_crowded = true;
    if (dist1 >= dist2 && dist1 >= dist3 && dist1 >= dist4)
    {
        most_crowded = 1;
    }
    else if (dist2 >= dist1 && dist2 >= dist3 && dist2 >= dist4)
    {
        most_crowded = 2;
    }
    else if (dist3 >= dist1 && dist3 >= dist2 && dist3 >= dist4)
    {
        most_crowded = 3;
    }
    else
    {
        most_crowded = 4;
    }

    if (is_total_crowded)
        speech("前方拥挤");
    else
        speech("前方宽松");
    switch (most_crowded)
    {
    case 1:
        speech("近距多障碍");
        break;
    case 2:
        speech("中距多障碍");
        break;
    case 3:
        speech("远距多障碍");
        break;
    case 4:
        speech("极远多障碍");
        break;
    default:
        break;
    }
    return 0;
}

int speechZebra(ZebraLine *zebraLineList, u8 length)
{
    unsigned char *text;
    if (ZebraLineNum > 0)
    {
        sprintf(text, "识别到%d条斑马线", ZebraLineNum);
        speech(text);
        if (zebraLineList[0].x < 320)
        {
            speech("脚下斑马线在您左侧");
        }
        else
        {
            speech("脚下斑马线在您右侧");
        }
        if (zebraLineList[0].deg < 0)
        {
            speech("指向左前方");
        }
        else
        {
            speech("指向右前方");
        }
    }
    return 0;
}

int speechTrafficLight(TrafficLight *trafficLightList, u8 length)
{
    unsigned char *text;
    if (TrafficLightNum > 0)
    {
        sprintf(text, "识别到%d个交通灯", TrafficLightNum);
        speech(text);
        if (trafficLightList[0].lightstatus == 1)
        {
            speech("最近距离交通灯为红灯");
        }
        else
        {
            speech("最近距离交通灯为绿灯");
        }
    }
    return 0;
}

int speechStairs(Stairs *stairsList, u8 length)
{
    unsigned char *text;
    if (StairsNum > 0)
    {
        sprintf(text, "识别到%d处阶梯", StairsNum);
        speech(text);
        if (stairsList[0].x < 213)
        {
            speech("最近阶梯在您左方");
        }
        else if (stairsList[0].x >= 213 && stairsList[0].x < 427)
        {
            speech("最近阶梯在您前方");
        }
        else
        {
            speech("最近阶梯在您右方");
        }
        sprintf(text, "阶梯数%d", stairsList[0].stairs_numbers);
        speech(text);
    }
}

void speechOut(void)
{
    speechObject(objectList);
    speechZebra(zebraLineList, ZebraLineNum);
    speechTrafficLight(trafficLightList, TrafficLightNum);
    speechStairs(stairsList, StairsNum);
}
