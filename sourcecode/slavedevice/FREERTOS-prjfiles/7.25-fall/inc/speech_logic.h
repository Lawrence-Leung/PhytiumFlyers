#ifndef SPEECH_LOGIC_H
#define SPEECH_LOGIC_H

typedef struct
{
    uint16_t x;
    uint16_t y;
    uint8_t lightstatus;
} TrafficLight;

// 楼梯
typedef struct
{
    uint16_t x;
    uint16_t y;
    uint8_t stairs_numbers;
} Stairs;

// 障碍物
typedef struct
{
    uint16_t x;
    uint16_t y;
    uint8_t obj_class;
    float distance;
    int8_t ver;
    float hor;
} Obstacle;

// //
// #define NUM_DISTANCE_RANGES 4
// #define NUM_CLASSES 5

// //
// typedef enum
// {
//     Class1 = 0,
//     Class2 = 1,
//     Class3 = 2,
//     Class4 = 3,
//     Class5 = 4
// } ObjectClass;

// 斑马线
typedef struct
{
    uint16_t x;
    uint16_t y;
    int deg;
} ZebraLine;

// 障碍物
typedef struct
{
    uint16_t x;
    uint16_t y;
    uint8_t class_type;
    float distance;
    int8_t ver;
    float hor;
} Object;

int parse_data(char *data);
int speechObject(Object *objectList);
int speechZebra(ZebraLine *zebraLineList, u8 length);
int speechTrafficLight(TrafficLight *trafficLightList, u8 length);
int speechStairs(Stairs *stairsList, u8 length);
void speechOut(void);
int speechVisual(void);
int speechFall(void);


#endif