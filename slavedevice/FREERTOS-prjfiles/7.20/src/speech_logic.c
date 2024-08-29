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
bool isGetData = false;
extern float fAcc[3], fGyro[3], fAngle[3];
extern SemaphoreHandle_t xMutex;
extern SemaphoreHandle_t xMutexJY61P;

u8 parse_lock = 0;

// ��������
int parse_data(char *pdata)
{
    // if (parse_lock == 1)
    // {
    // parse_lock = 0;
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
    // printf("prased:");
    // for (size_t i = 0; i < length; i++)
    // {
    //     printf("%02x ", data[i]);
    // }
    // printf("\r\n");
    // printf("data0:%02x\tdatalength-1:%02x\r\n",data[0],data[length-1]);

    /* ����֡ */
    if (data[0] == 0xAA && data[length - 1] == 0xBB)
    {
        ObjectNum = data[1];
        ZebraLineNum = data[2];
        TrafficLightNum = data[3];
        StairsNum = data[4];
        printf("[NUM]object:%d\tzebra:%d\ttrafficlight:%d\tstairs:%d\r\n", ObjectNum, ZebraLineNum, TrafficLightNum, StairsNum);
        if (ObjectNum != 0 || ZebraLineNum != 0 || TrafficLightNum != 0 || StairsNum != 0)
            isGetData = true;
    }
    else
    {
        return 0;
    }
    if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
    {
        if (ObjectNum > 0)
        {
            objectList = malloc(ObjectNum * sizeof(Object));
        }

        if (ZebraLineNum > 0)
            zebraLineList = malloc(ZebraLineNum * sizeof(ZebraLine));
        if (TrafficLightNum > 0)
            trafficLightList = malloc(TrafficLightNum * sizeof(TrafficLight));
        if (StairsNum > 0)
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
                        // printf("%lf\t",objectList[j].distance);
                    }
                }
                printf("\r\n");
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
    }
    xSemaphoreGive(xMutex);

    // }

    return 0;
}

// int speechObject(Object *objectList)
int speechObject(void)
{
    static uint8_t dist1 = 0, dist2 = 0, dist3 = 0, dist4 = 0;
    bool is_total_crowded = false;
    uint8_t most_crowded = 0;
    // if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
    // {
    if (objectList == NULL)
    {
        printf("[object]None\r\n");
    }
    if (parse_lock == 1)
    {

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
        printf("1:%d\t2:%d\t3:%d\t4:%d\r\n");
        u8 total_objects = dist1 + dist2 + dist3 + dist4;
        if (total_objects >= 10)
        {
            speech("ǰ��ӵ����");
            printf("heavy\r\n");
        }
        else if (total_objects > 0)
        {
            speech("ǰ�����ɡ�");
            printf("loose\r\n");
        }

        if (dist1 > 0 || dist2 > 0 || dist3 > 0 || dist4 > 0)
        {

            if (dist1 >= dist2 && dist1 >= dist3 && dist1 >= dist4)
            {
                speech("������ϰ���");
                printf("close\r\n");
            }
            else if (dist2 >= dist1 && dist2 >= dist3 && dist2 >= dist4)
            {
                speech("�о���ϰ���");
                printf("middle\r\n");
            }
            else if (dist3 >= dist1 && dist3 >= dist2 && dist3 >= dist4)
            {
                speech("Զ����ϰ���");
                printf("far\r\n");
            }
            else
            {
                speech("��Զ���ϰ���");
                printf("very far\r\n");
            }
        }
        // }
        // xSemaphoreGive(xMutex);
        dist1 = 0, dist2 = 0, dist3 = 0, dist4 = 0;
        parse_lock = 0;
    }
    return 0;
}

int speechZebra(ZebraLine *zebraLineList, u8 length)
{
    unsigned char *text;
    if (zebraLineList == NULL)
    {
        printf("[zebraLine]None\r\n");
    }
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

int speechVisual(void)
{
    // printf("speech visual:%f\t%f\r\n",fAngle[0],fAngle[1]);
    // if (xSemaphoreTake(xMutexJY61P, portMAX_DELAY) == pdTRUE)
    // {
    if (abs(fAngle[0]) > 40 || abs(fAngle[1]) > 40)
    {
        speech("�ӽ��쳣��");
        printf("visual invaild\r\n");
    }
    //     xSemaphoreGive(xMutexJY61P);
    // }
    return 0;
}

int speechFall(void)
{
    // if (xSemaphoreTake(xMutexJY61P, portMAX_DELAY) == pdTRUE)
    // {
    if (fAcc[0] > 20 || fAcc[1] > 20 || fAcc[2] > 20)
    {
        speech("ˤ����");
    }
    //     xSemaphoreGive(xMutexJY61P);
    // }
    return 0;
}

void speechOut(void)
{
    while (1)
    {
        if (isGetData)
        {
            if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
            {
                if (objectList != NULL)
                {
                    // speechObject(objectList);
                    speechObject();
                    free(objectList);
                    objectList = NULL;
                }
                if (zebraLineList != NULL)
                {
                    speechZebra(zebraLineList, ZebraLineNum);
                    free(zebraLineList);
                    zebraLineList = NULL;
                }
                // if(trafficLightList != NULL)
                //     speechTrafficLight(trafficLightList, TrafficLightNum);
                // if(stairsList != NULL)
                //     speechStairs(stairsList, StairsNum);

                // free(zebraLineList);
                // free(trafficLightList);
                // free(stairsList);
                // memset(objectList,0,sizeof(objectList));
                // memset(zebraLineList,0,sizeof(zebraLineList));
                // memset(trafficLightList,0,sizeof(trafficLightList));
                // memset(stairsList,0,sizeof(stairsList));

                // zebraLineList = NULL;
                // trafficLightList = NULL;
                // stairsList = NULL;
                ZebraLineNum = 0;
                TrafficLightNum = 0;
                StairsNum = 0;
                isGetData = false;
                // �ͷŻ�����
                xSemaphoreGive(xMutex);
            }
        }
        // else
        // {
        //     printf("no data!\r\n");
        // }
        speechVisual();
        speechFall();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
