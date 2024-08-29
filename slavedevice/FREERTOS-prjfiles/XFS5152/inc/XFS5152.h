#ifndef XFS5152_H
#define XFS5152_H

#define XFS5152_MIO FMIO1_ID        //MIO1

// #define I2Cx_OWN_ADDRESS7      0X0A
#define I2C_ADDR 0x30   //语音识别模块地址,模块地址为0x50，由于最右边一位要做读写位，所以右移一位为0xa0
//旧版：0x50；新版：0x30

//芯片相关结构体
typedef enum
{
    GB2312 = 0x00,
    GBK = 0x01,
    BIG5 = 0x02,
    UNICODE = 0x03
} EncodingFormat_Type;//文本的编码格式

typedef struct
{
    u8 DataHead;
    u8 Length_HH;
    u8 Length_LL;
    u8 Commond;
    u8 EncodingFormat;
    u8* Text;
} XFS_Protocol_TypeDef;

typedef enum
{
		ChipStatus_InitSuccessful = 0x4A,//初始化成功回传
		ChipStatus_CorrectCommand = 0x41,//收到正确的命令帧回传
		ChipStatus_ErrorCommand = 0x45,//收到不能识别命令帧回传
		ChipStatus_Busy = 0x4E,//芯片忙碌状态回传
		ChipStatus_Idle = 0x4F//芯片空闲状态回传
} ChipStatus_Type;//芯片回传

typedef enum
{
		Style_Single,//？为 0，一字一顿的风格
		Style_Continue//？为 1，正常合成
} Style_Type; //合成风格设置 [f?]

typedef enum
{
		Language_Auto,//? 为 0，自动判断语种
		Language_Chinese,//? 为 1，阿拉伯数字、度量单位、特殊符号等合成为中文
		Language_English//? 为 2，阿拉伯数字、度量单位、特殊符号等合成为英文
} Language_Type; //合成语种设置 [g?]

typedef enum
{
		Articulation_Auto,//? 为 0，自动判断单词发音方式
		Articulation_Letter,//? 为 1，字母发音方式
		Articulation_Word//? 为 2，单词发音方式
} Articulation_Type; //设置单词的发音方式 [h?]

typedef enum
{
		Spell_Disable,//? 为 0，不识别汉语拼音
		Spell_Enable//? 为 1，将“拼音＋1 位数字（声调）”识别为汉语拼音，例如： hao3
} Spell_Type; //设置对汉语拼音的识别 [i?]

typedef enum
{
		Reader_XiaoYan = 3,//? 为 3，设置发音人为小燕(女声, 推荐发音人)
		Reader_XuJiu = 51,//? 为 51，设置发音人为许久(男声, 推荐发音人)
		Reader_XuDuo = 52,//? 为 52，设置发音人为许多(男声)
		Reader_XiaoPing = 53,//? 为 53，设置发音人为小萍(女声)
		Reader_DonaldDuck = 54,//? 为 54，设置发音人为唐老鸭(效果器)
		Reader_XuXiaoBao = 55//? 为 55，设置发音人为许小宝(女童声)
} Reader_Type;//选择发音人 [m?]

typedef enum
{
		NumberHandle_Auto,//? 为 0，自动判断
		NumberHandle_Number,//? 为 1，数字作号码处理
		NumberHandle_Value//? 为 2，数字作数值处理
} NumberHandle_Type; //设置数字处理策略 [n?]

FError FFreeRTOSI2cInitSet(uint32_t id, uint32_t work_mode, uint32_t slave_address);
void speech_text(u8 *str,u8 encoding_format);
void GetChipStatus(void);
void SetVolume(int volume);
void SetReader(Reader_Type reader);
BaseType_t FFreeRTOSI2cLoopbackCreate(void);

#endif