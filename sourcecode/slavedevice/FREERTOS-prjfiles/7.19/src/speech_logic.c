#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "ftypes.h"
#include "speech_i2c.h"
#include "speech_logic.h"

#define MAX_DATA_LENGTH 256

Object *objectList = NULL;
ZebraLine *zebraLineList = NULL;
TrafficLight *trafficLightList = NULL;
Stairs *stairsList = NULL;

unsigned char ObjectNum = 0;
unsigned char ZebraLineNum = 0;
unsigned char TrafficLightNum = 0;
unsigned char StairsNum = 0;
bool isGetData = false;
extern float fAcc[3], fGyro[3], fAngle[3];

// ��������
int parse_data(char *pdata)
{
    // size_t start_idx = 0;
    size_t plength = strlen(pdata);
    uint8_t data[MAX_DATA_LENGTH];

    size_t length = 0;
    for (size_t i = 0; i < plength; i += 2)
    {
        //     if (pdata[i] == '0' && pdata[i + 1] == 'x') {
        sscanf(pdata + i, "%2hhx", &data[length]);
        length++;
        //     }
    }
    printf("prased:");
    for (size_t i = 0; i < length; i++)
    {
        printf("%02x ", data[i]);
    }
    printf("\r\n");

    /* ����֡ */
    if (data[0] == 0xAA && data[length - 1] == 0xBB)
    {
        ObjectNum = data[1];
        ZebraLineNum = data[2];
        TrafficLightNum = data[3];
        StairsNum = data[4];
        printf("[NUM]object:%d\tzebra:%d\ttrafficlight:%d\tstairs:%d\r\n", ObjectNum, ZebraLineNum, TrafficLightNum, StairsNum);
        isGetData = true;
    }
    else
    {
        return 0;
    }

    objectList = malloc(ObjectNum * sizeof(Object));
    zebraLineList = malloc(ZebraLineNum * sizeof(ZebraLine));
    trafficLightList = malloc(TrafficLightNum * sizeof(TrafficLight));
    stairsList = malloc(StairsNum * sizeof(Stairs));
    printf("start parse object!\r\n");
    for (size_t i = 5; i < length; i++)
    {
        /* ����֡ */
        if (data[i] == 0x1A)
        {
            for (size_t j = 0; j < ObjectNum; j++)
            {
                if (data[i + 10] == 0x1B) // �жϰ�β�Ƿ�Ϊ0x1B
                {
                    objectList[j].x = data[i + 1] << 8 | data[i + 2];
                    objectList[j].y = data[i + 3] << 8 | data[i + 4];
                    objectList[j].distance = (float)data[i + 6] / 10.0f;
                    objectList[j].ver = data[i + 7];
                    objectList[j].hor = (float)((data[i + 8] << 8) | data[i + 9]) / 10.0f;
                    printf("[objectList]x:%d\ty:%d\tclass_type:%d\tdistance:%.1f\tver:%d\thor:%.1f\r\n", objectList[j].x, objectList[j].y, objectList[j].class_type, objectList[j].distance, objectList[j].ver, objectList[j].hor);
                    i += 10; // ������10���ֽ�
                }
                else
                {
                    printf("[object]data error!\r\n");
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
                    printf("[zebraLineList]x:%d\ty:%d\tdeg:%d\r\n");
                    i += 7;
                }
                else
                {
                    printf("[zebraLine]data error!\r\n");
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
                    printf("[trafficLightList]x:%d\ty:%d\tlightstatus:%d\r\n");
                    i += 6;
                }
                else
                {
                    printf("[trafficLight]data error!\r\n");
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
                    printf("[stairsList]x:%d\ty:%d\tstairs_numbers:%d\r\n");
                    i += 6;
                }
                else
                {
                    printf("[stairs]data error!\r\n");
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

    for (int i = 0; i < ObjectNum; i++)
    {
        // �����5���ϰ����ʱû��
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
        speech("ǰ��ӵ����");
    else
        speech("ǰ�����ɡ�");

    if (dist1 >= dist2 && dist1 >= dist3 && dist1 >= dist4)
    {
        speech("������ϰ���");
        printf("������ϰ�\r\n");
    }
    else if (dist2 >= dist1 && dist2 >= dist3 && dist2 >= dist4)
    {
        speech("�о���ϰ���");
        printf("�о���ϰ�\r\n");
    }
    else if (dist3 >= dist1 && dist3 >= dist2 && dist3 >= dist4)
    {
        speech("Զ����ϰ���");
        printf("Զ����ϰ�\r\n");
    }
    else
    {
        speech("��Զ���ϰ���");
        printf("��Զ���ϰ�\r\n");
    }
    return 0;
}

int speechZebra(ZebraLine *zebraLineList, u8 length)
{
    unsigned char *text;
    printf("zebra:%d,", ZebraLineNum);
    printf("%d,", zebraLineList[0].x);
    printf("%d,", zebraLineList[0].deg);
    if (ZebraLineNum > 0)
    {
        sprintf(text, "ʶ��%d�������ߡ�", ZebraLineNum);
        printf(text);
        speech(text);
        printf("%s",text);
        if (zebraLineList[0].x < 320)
        {
            speech("���°�����������ࡣ");
            printf("���°������������\r\n");
        }
        else
        {
            speech("���°����������Ҳࡣ");
            printf("���°����������Ҳ�\r\n");
        }
        if (zebraLineList[0].deg < 0)
        {
            speech("ָ����ǰ����");
            printf("ָ����ǰ��\r\n");
        }
        else
        {
            speech("ָ����ǰ����");
            printf("ָ����ǰ��\r\n");
        }
    }
    printf("\r\n");
    return 0;
}

int speechTrafficLight(TrafficLight *trafficLightList, u8 length)
{
    unsigned char *text;
    if (TrafficLightNum > 0)
    {
        sprintf(text, "ʶ��%d����ͨ�ơ�", TrafficLightNum);
        speech(text);
        printf("%s\r\n",text);
        if (trafficLightList[0].lightstatus == 1)
        {
            speech("������뽻ͨ��Ϊ��ơ�");
            printf("������뽻ͨ��Ϊ���\r\n");
        }
        else
        {
            speech("������뽻ͨ��Ϊ�̵ơ�");
            printf("������뽻ͨ��Ϊ�̵�\r\n");
        }
    }
    return 0;
}

int speechStairs(Stairs *stairsList, u8 length)
{
    unsigned char *text;
    if (StairsNum > 0)
    {
        sprintf(text, "ʶ��%d�����ݡ�", StairsNum);
        speech(text);
        printf("%s",text);
        if (stairsList[0].x < 213)
        {
            speech("������������󷽡�");
            printf("�������������\r\n");
        }
        else if (stairsList[0].x >= 213 && stairsList[0].x < 427)
        {
            speech("�����������ǰ����");
            printf("�����������ǰ��\r\n");
        }
        else
        {
            speech("������������ҷ���");
            speech("������������ҷ�\r\n");
        }
        sprintf(text, "������%d", stairsList[0].stairs_numbers);
        speech(text);
        printf("%s",text);
    }
    return 0;
}

int speechVisual(void)
{
    if (fAngle[0] > 50 || fAngle[1] > 50 || fAngle[2] > 50)
    {
        speech("�ӽ��쳣��");
    }
    return 0;
}

int speechFall(void)
{
    if (fAcc[0] > 20 || fAcc[1] > 20 || fAcc[2] > 20)
    {
        speech("ˤ����");
    }
    return 0;
}

void speechOut(void)
{
    while (1)
    {
        if (isGetData)
        {
            speechObject(objectList);
            speechZebra(zebraLineList, ZebraLineNum);
            speechTrafficLight(trafficLightList, TrafficLightNum);
            speechStairs(stairsList, StairsNum);
            free(objectList);
            free(zebraLineList);
            free(trafficLightList);
            free(stairsList);
            objectList = NULL;
            zebraLineList = NULL;
            trafficLightList = NULL;
            stairsList = NULL;
            ZebraLineNum = 0;
            TrafficLightNum = 0;
            StairsNum = 0;
        }
        speechVisual();
        speechFall();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
