#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "ftypes.h"
#include "speech_i2c.h"
#include "speech_logic.h"

#define MAX_DATA_LENGTH 1024

Object *objectList = NULL;
ZebraLine *zebraLineList = NULL;
TrafficLight *trafficLightList = NULL;
Stairs *stairsList = NULL;

unsigned char ObjectNum = 0;
unsigned char ZebraLineNum = 0;
unsigned char TrafficLightNum = 0;
unsigned char StairsNum = 0;

extern SemaphoreHandle_t xMutex_speech;
extern SemaphoreHandle_t xMutex_isGetData;
bool isGetData = false;

// 解析数据
int parse_data(char *pdata)
{
    size_t plength = strlen(pdata);
    uint8_t data[MAX_DATA_LENGTH];

    size_t length = 0;
    for (size_t i = 0; i < plength; i += 2)
    {
        //     if (pdata[i] == '0' && pdata[i + 1] == 'x') {
        sscanf(pdata + i, "%2hhx", &data[length]);
        // printf("%2hhx",data[length]);
        length++;
        //     }
    }

    /* 完整帧 */
    if (data[0] == 0xAA && data[length - 1] == 0xBB)
    {
        ObjectNum = data[1];
        ZebraLineNum = data[2];
        TrafficLightNum = data[3];
        StairsNum = data[4];
        printf("[NUM]object:%d\tzebra:%d\ttrafficlight:%d\tstairs:%d\r\n", ObjectNum, ZebraLineNum, TrafficLightNum, StairsNum);
        if (xSemaphoreTake(xMutex_isGetData, portMAX_DELAY) == pdTRUE)
        {
            if (ObjectNum != 0 || ZebraLineNum != 0 || TrafficLightNum != 0 || StairsNum != 0)
                isGetData = true;
            else
            {
                isGetData = false;
            }
            xSemaphoreGive(xMutex_isGetData);
        }
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
                printf("trafficLightList:x:%d\ty:%d\tstatus:%d\r\n", trafficLightList[j].x, trafficLightList[j].y, trafficLightList[j].lightstatus);
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
    memset(data,0,sizeof(data));
    return 0;
}

int speechObject(Object *objectList)
{
    static uint8_t dist1, dist2, dist3, dist4 = 0;
    bool is_total_crowded = false;
    uint8_t most_crowded = 0;

    if (xSemaphoreTake(xMutex_speech, portMAX_DELAY) == pdTRUE)
    {
        for (int i = 0; i < ObjectNum; i++)
        {
            // 最近的5个障碍物，暂时没用
            //  if (abs(objectList[i].ver) < 45)
            //  {
            //      closest_class[j] = objectList[i].class_type;
            //      j++;
            //  }
            if (objectList[i].distance >= 0 && objectList[i].distance <= 2.5)
                dist1 += 1;
            else if (objectList[i].distance >= 2.5 && objectList[i].distance <= 5)
                dist2 += 1;
            else if (objectList[i].distance >= 5 && objectList[i].distance <= 7.5)
                dist3 += 1;
            else if (objectList[i].distance >= 7.5 && objectList[i].distance <= 10)
                dist4 += 1;
        }
        u8 total_objects = dist1 + dist2 + dist3 + dist4;
        if (total_objects >= 10)
        {
            speech("前方拥挤。");
            printf("heavy\r\n");
        }
        else if (total_objects > 0)
        {
            speech("前方宽松。");
            printf("loose\r\n");
        }

        if (dist1 > 0 || dist2 > 0 || dist3 > 0 || dist4 > 0)
        {
            printf("1:%d\t2:%d\t3:%d\t4:%d\r\n", dist1, dist2, dist3, dist4);
            if (dist1 >= dist2 && dist1 >= dist3 && dist1 >= dist4)
            {
                speech("近距多障碍。");
                printf("close\r\n");
            }
            else if (dist2 >= dist1 && dist2 >= dist3 && dist2 >= dist4)
            {
                speech("中距多障碍。");
                printf("middle\r\n");
            }
            else if (dist3 >= dist1 && dist3 >= dist2 && dist3 >= dist4)
            {
                speech("远距多障碍。");
                printf("far\r\n");
            }
            else
            {
                speech("极远多障碍。");
                printf("very far\r\n");
            }
        }
        // }
        // xSemaphoreGive(xMutex);
        dist1 = 0, dist2 = 0, dist3 = 0, dist4 = 0;
        xSemaphoreGive(xMutex_speech);
    }
    return 0;
}

int speechZebra(ZebraLine *zebraLineList, u8 length)
{
    unsigned char *text = (char *)malloc(50 * sizeof(char));
    if (text == NULL)
    {
        printf("Memory allocation failed.\n");
        return -1;
    }
    if (xSemaphoreTake(xMutex_speech, portMAX_DELAY) == pdTRUE)
    {
        if (ZebraLineNum > 0)
        {
            sprintf(text, "识别到%d条斑马线。", ZebraLineNum);
            speech(text);
            if (zebraLineList[0].x < 320)
            {
                speech("脚下斑马线在您左侧。");
            }
            else
            {
                speech("脚下斑马线在您右侧。");
            }
            if (zebraLineList[0].deg < 0)
            {
                speech("指向左前方。");
            }
            else
            {
                speech("指向右前方。");
            }
        }
        xSemaphoreGive(xMutex_speech);
    }
    free(text); // 释放text指向的内存
    return 0;
}

int speechTrafficLight(TrafficLight *trafficLightList, u8 length)
{
    unsigned char *text = (char *)malloc(50 * sizeof(char));
    if (text == NULL)
    {
        printf("Memory allocation failed.\n");
        return -1;
    }
    if (xSemaphoreTake(xMutex_speech, portMAX_DELAY) == pdTRUE)
    {
        if (TrafficLightNum > 0)
        {
            sprintf(text, "识别到%d个交通灯。", TrafficLightNum);
            speech(text);
            printf("trafficLightL%d\r\n", TrafficLightNum);
            if (trafficLightList[0].lightstatus == 1)
            {
                speech("最近距离交通灯为红灯。");
                printf("red\r\n");
            }
            else
            {
                speech("最近距离交通灯为绿灯。");
                printf("green\r\n");
            }
        }
        xSemaphoreGive(xMutex_speech);
    }
    free(text); // 释放text指向的内存
    return 0;
}

int speechStairs(Stairs *stairsList, u8 length)
{
    unsigned char *text = (char *)malloc(50 * sizeof(char));
    if (text == NULL)
    {
        printf("Memory allocation failed.\n");
        return -1;
    }
    if (xSemaphoreTake(xMutex_speech, portMAX_DELAY) == pdTRUE)
    {
        if (StairsNum > 0)
        {
            sprintf(text, "识别到%d处阶梯。", StairsNum);
            speech(text);
            if (stairsList[0].x < 213)
            {
                speech("最近阶梯在您左方。");
            }
            else if (stairsList[0].x >= 213 && stairsList[0].x < 427)
            {
                speech("最近阶梯在您前方。");
            }
            else
            {
                speech("最近阶梯在您右方。");
            }
            sprintf(text, "阶梯数%d。", stairsList[0].stairs_numbers);
            speech(text);
        }
        xSemaphoreGive(xMutex_speech);
    }
    free(text); // 释放text指向的内存
    return 0;
}

void speechOut(void)
{
    while (1)
    {
        if (xSemaphoreTake(xMutex_isGetData, portMAX_DELAY) == pdTRUE)
        {
            if (isGetData)
            {

                if (objectList != NULL)
                {
                    speechObject(objectList);
                    free(objectList);
                    objectList = NULL;
                }

                if (zebraLineList != NULL)
                {
                    speechZebra(zebraLineList, ZebraLineNum);
                    free(zebraLineList);
                    zebraLineList = NULL;
                    ZebraLineNum = 0;
                }

                if (trafficLightList != NULL)
                {
                    speechTrafficLight(trafficLightList, TrafficLightNum);
                    free(trafficLightList);
                    trafficLightList = NULL;
                    TrafficLightNum = 0;
                }

                if (stairsList != NULL)
                {
                    speechStairs(stairsList, StairsNum);
                    free(stairsList);
                    stairsList = NULL;
                    StairsNum = 0;
                }
                isGetData = false;
            }
            xSemaphoreGive(xMutex_isGetData);
        }
        vTaskDelay(800 / portTICK_PERIOD_MS);
    }
}
