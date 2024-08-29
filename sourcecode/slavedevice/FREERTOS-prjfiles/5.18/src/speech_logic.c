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

// ��������
int parse_data(char *data)
{
    size_t start_idx = 0;
    size_t length = sizeof(data);

    /* ����֡ */
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
        /* ����֡ */
        if (data[i] == 0x1A)
        {
            if (data[i + 10] == 0x1B) // �жϰ�β�Ƿ�Ϊ0x1B
            {
                for (size_t j = 0; j < ObjectNum; j++)
                {
                    objectList[j].x = data[i + 1] << 8 | data[i + 2];
                    objectList[j].y = data[i + 3] << 8 | data[i + 4];
                    objectList[j].distance = (float)data[i + 6] / 10.0f;
                    objectList[j].ver = data[i + 7];
                    objectList[j].hor = (float)((data[i + 8] << 8) | data[i + 9]) / 10.0f;
                    i += 10; // ������10���ֽ�
                }
            }
        }
        /* ������֡ */
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
        /* ��ͨ��֡ */
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
        /* ¥��֡ */
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
        speech("ǰ��ӵ��");
    else
        speech("ǰ������");
    switch (most_crowded)
    {
    case 1:
        speech("������ϰ�");
        break;
    case 2:
        speech("�о���ϰ�");
        break;
    case 3:
        speech("Զ����ϰ�");
        break;
    case 4:
        speech("��Զ���ϰ�");
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
        sprintf(text, "ʶ��%d��������", ZebraLineNum);
        speech(text);
        if (zebraLineList[0].x < 320)
        {
            speech("���°������������");
        }
        else
        {
            speech("���°����������Ҳ�");
        }
        if (zebraLineList[0].deg < 0)
        {
            speech("ָ����ǰ��");
        }
        else
        {
            speech("ָ����ǰ��");
        }
    }
    return 0;
}

int speechTrafficLight(TrafficLight *trafficLightList, u8 length)
{
    unsigned char *text;
    if (TrafficLightNum > 0)
    {
        sprintf(text, "ʶ��%d����ͨ��", TrafficLightNum);
        speech(text);
        if (trafficLightList[0].lightstatus == 1)
        {
            speech("������뽻ͨ��Ϊ���");
        }
        else
        {
            speech("������뽻ͨ��Ϊ�̵�");
        }
    }
    return 0;
}

int speechStairs(Stairs *stairsList, u8 length)
{
    unsigned char *text;
    if (StairsNum > 0)
    {
        sprintf(text, "ʶ��%d������", StairsNum);
        speech(text);
        if (stairsList[0].x < 213)
        {
            speech("�������������");
        }
        else if (stairsList[0].x >= 213 && stairsList[0].x < 427)
        {
            speech("�����������ǰ��");
        }
        else
        {
            speech("������������ҷ�");
        }
        sprintf(text, "������%d", stairsList[0].stairs_numbers);
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
