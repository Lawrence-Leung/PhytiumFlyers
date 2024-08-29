#include <string.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "fi2c.h"
#include "fi2c_hw.h"
#include "fi2c_os.h"
#include "timers.h"
#include "fcpu_info.h"
// #include "i2c_example.h"
#include "fparameters.h"
#include "fio_mux.h"
#include "sdkconfig.h"
#include "fdebug.h"
#include "ftypes.h"
#include "finterrupt.h"
#include "XFS5152.h"
#include "fsleep.h"

// #define I2C_ADDR 0x01   //语音识别模块地址,模块地址为0x30，由于最右边一位要做读写位，所以右移一位为0x60

#define DATAHEAD 0xFD   //帧头

#define DAT_LENGTH  15

static char data_r0[DAT_LENGTH];

FFreeRTOSI2c *os_i2c_master;
FFreeRTOSI2c *os_i2c_slave;
// static data_r0[DAT_LENGTH];
FFreeRTOSI2cMessage message = {
    .buf = 0,
    .buf_length = 0,
    .mem_addr = 0x60,
    .mem_byte_len = 1,
    .slave_addr = I2C_ADDR,
    .mode = FI2C_WRITE_DATA_POLL
};

FFreeRTOSI2cMessage messageread = {
    .buf = 0,
    .buf_length = 0,
    .mem_addr = 0x61,
    .mem_byte_len = 1,
    .slave_addr = I2C_ADDR,
    .mode = FI2C_READ_DATA_POLL
};
// FFreeRTOSI2c *os_i2c_write_p = os_i2c_master;
// FFreeRTOSI2c *os_i2c_read_p = os_i2c_master;



//i2c结构体
typedef struct data
{
    boolean first_write;/*IIC首次写入，在初始化时置位，用来指示当前传输的首个字节数据是用户需要读写的地址偏移*/
    u32 buff_idx;/* PC 指向的地址偏移 */
    u8 buff[IO_BUF_LEN];/*虚拟内存块*/
} FI2cSlaveData;

/* Slave mode for virtual eeprom memory ，size: IO_BUF_LEN in fi2c_os.h*/
static FI2cSlaveData slave;

FError FFreeRTOSI2cInitSet(uint32_t id, uint32_t work_mode, uint32_t slave_address)
{
    FError err;
    FIOPadSetMioMux(id);    
    /* init i2c controller */
    if (work_mode == FI2C_MASTER) /* 主机初始化默认使用poll模式 */
    {
        os_i2c_master = FFreeRTOSI2cInit(id, work_mode, slave_address, FI2C_SPEED_STANDARD_RATE);

        f_printk("[DEBUG] slave_addr:0x%x \r\n",os_i2c_master->i2c_device.config.slave_addr);
        f_printk("[DEBUG] base_addr:0x%x \r\n",os_i2c_master->i2c_device.config.base_addr);
        fsleep_seconds(5);  //debug
    
        /* register intr callback */
        // InterruptInstall(os_i2c_master->i2c_device.config.irq_num, FI2cMasterIntrHandler, &os_i2c_master->i2c_device, "fi2cmaster");
        /* register intr handler func */
        // FI2cMasterRegisterIntrHandler(&os_i2c_master->i2c_device, FI2C_EVT_MASTER_TRANS_ABORTED, FI2cIntrTxAbrtcallback);
        // FI2cMasterRegisterIntrHandler(&os_i2c_master->i2c_device, FI2C_EVT_MASTER_READ_DONE, FI2cIntrRxDonecallback);
        // FI2cMasterRegisterIntrHandler(&os_i2c_master->i2c_device, FI2C_EVT_MASTER_WRITE_DONE, FI2cIntrTxDonecallback);
    }
    else
    {
        printf("error_work_mode!\r\n");
        return FREERTOS_I2C_INVAILD_PARAM_ERROR;
    }
    return FREERTOS_I2C_SUCCESS;
}


void speech_text(u8 *str,u8 encoding_format)
{
    FError ret = FREERTOS_I2C_SUCCESS;

    u16 size = strlen(str) + 2;
    XFS_Protocol_TypeDef DataPack;

    DataPack.DataHead = DATAHEAD;
    DataPack.Length_HH = size >>8;
    DataPack.Length_LL = size;
    DataPack.Commond = 0x01;
    DataPack.EncodingFormat = encoding_format;
    DataPack.Text = str;

    message.mode = FI2C_WRITE_DATA_POLL;
    message.slave_addr = I2C_ADDR;
    //  message.slave_addr = os_i2c_write_p->i2c_device.config.slave_addr;
    message.mem_addr = 0x0;/* 地址偏移0x0的位置poll方式写入数据 */
    message.mem_byte_len = 1;
    u8 buf_len = 5;
    u8 buf[buf_len];
    buf[0] = DataPack.DataHead;
    buf[1] = DataPack.Length_HH;
    buf[2] = DataPack.Length_LL;
    buf[3] = DataPack.Commond;
    buf[4] = DataPack.EncodingFormat;
    // strcat(buf,str);
	message.buf = buf;
    message.buf_length = buf_len;
    ret = FFreeRTOSI2cTransfer(os_i2c_master, &message);
    if (ret != FREERTOS_I2C_SUCCESS)
    {
        vPrintf("FFreeRTOSI2cTransfer write poll task error,i2c id:%d.\r\n", os_i2c_master->i2c_device.config.instance_id);
    }

    message.buf = str;
    message.buf_length = strlen(str);
    ret = FFreeRTOSI2cTransfer(os_i2c_master, &message);
    if (ret != FREERTOS_I2C_SUCCESS)
    {
        vPrintf("FFreeRTOSI2cTransfer write poll task error,i2c id:%d.\r\n", os_i2c_master->i2c_device.config.instance_id);
    }
}

/**
 * @brief 获取语音模块的当前芯片状态
 * for debug only
 */
void GetChipStatus(void) 
{  
    FError ret;

	// u16 NumByteToRead = 1;
	u8 pBuffer = 0xff;
    u8 AskState[3] = {0x00,0x01,0x21};
	
    f_printk("\r\n");

    memset(data_r0, 0, sizeof(data_r0));

    // 注意：
    // base_addr：I2C控制器的基地址，目前飞腾派有控制器1、控制器8两个I2C控制器
    // slave_addr：I2C的slave目标地址
    // mem_addr：I2C slave设备需要写入的偏移值

    for (uint32_t i = 0; i < 10; i++)
    {
        /************************* 测试1 *************************/
        FFreeRTOSI2cInitSet(XFS5152_MIO, FI2C_MASTER, 0x30);
        f_printk("Inited as address 0x30 \r\n");
        // 初始化设备：写进程
        message.slave_addr = os_i2c_master->i2c_device.config.slave_addr;
        message.mem_byte_len = 1;
        message.buf = AskState;
        message.buf_length = strlen(AskState);
        message.mode = FI2C_WRITE_DATA_POLL;
        message.mem_addr = 0xFD;
        f_printk("[DEBUG] mem_addr = %x \r\n Start Writing \r\n", message.mem_addr);

        // 开始传输
        ret = FFreeRTOSI2cTransfer(os_i2c_master, &message);
        if (ret != FREERTOS_I2C_SUCCESS) {
            vPrintf("FFreeRTOSI2cTransfer write poll task error,i2c id:%d.\r\n", os_i2c_master->i2c_device.config.instance_id);
        }
        //fsleep_millisec(3000);

        // 初始化设备：读进程
        messageread.slave_addr = os_i2c_master->i2c_device.config.slave_addr;
        messageread.mem_byte_len = 1;
        messageread.buf = data_r0;
        messageread.buf_length = strlen(data_r0);
        messageread.mode = FI2C_READ_DATA_POLL;
        messageread.mem_addr = 0xFD; 
        f_printk("[DEBUG] mem_addr = %x \r\n Start Reading \r\n", message.mem_addr);

        // 开始传输
        ret = FFreeRTOSI2cTransfer(os_i2c_master, &messageread);
        if (ret != FREERTOS_I2C_SUCCESS) {
            vPrintf("FFreeRTOSI2cTransfer read poll task error,i2c id:%d.\r\n", os_i2c_master->i2c_device.config.instance_id);
        } else {
            f_printk("Get Status Finished! \r\n");
        }
        f_printk("data_r0: %s\r\n", data_r0);
        f_printk("messageread.buf: %s\r\n", messageread.buf);

        /************************* 测试2 *************************/
        FFreeRTOSI2cDeinit(os_i2c_master);
        FFreeRTOSI2cInitSet(XFS5152_MIO, FI2C_MASTER, 0x60);
        f_printk("Inited as address 0x60 \r\n");
        // 初始化设备：写进程
        message.slave_addr = os_i2c_master->i2c_device.config.slave_addr;
        message.mem_byte_len = 1;
        message.buf = AskState;
        message.buf_length = strlen(AskState);
        message.mode = FI2C_WRITE_DATA_POLL;
        message.mem_addr = 0xFD;
        f_printk("[DEBUG] mem_addr = %x \r\n Start Writing \r\n", message.mem_addr);

        // 开始传输
        ret = FFreeRTOSI2cTransfer(os_i2c_master, &message);
        if (ret != FREERTOS_I2C_SUCCESS) {
            vPrintf("FFreeRTOSI2cTransfer write poll task error,i2c id:%d.\r\n", os_i2c_master->i2c_device.config.instance_id);
        }
        //fsleep_millisec(3000);

        // 初始化设备：读进程
        messageread.slave_addr = os_i2c_master->i2c_device.config.slave_addr;
        messageread.mem_byte_len = 1;
        messageread.buf = data_r0;
        messageread.buf_length = strlen(data_r0);
        messageread.mode = FI2C_READ_DATA_POLL;
        messageread.mem_addr = 0xFD; 
        f_printk("[DEBUG] mem_addr = %x \r\n Start Reading \r\n", message.mem_addr);

        // 开始传输
        ret = FFreeRTOSI2cTransfer(os_i2c_master, &messageread);
        if (ret != FREERTOS_I2C_SUCCESS) {
            vPrintf("FFreeRTOSI2cTransfer read poll task error,i2c id:%d.\r\n", os_i2c_master->i2c_device.config.instance_id);
        } else {
            f_printk("Get Status Finished! \r\n");
        }
        f_printk("data_r0: %s\r\n", data_r0);
        f_printk("messageread.buf: %s\r\n", messageread.buf);
    
        FFreeRTOSI2cDeinit(os_i2c_master);
    }
}

void SetBase(u8 *str)
{
    FError ret;
	 u16 size = strlen(str) + 2;
	 
	 XFS_Protocol_TypeDef DataPack;
	
	 DataPack.DataHead = DATAHEAD;
	 DataPack.Length_HH = size >>8;
	 DataPack.Length_LL = size;
	 DataPack.Commond = 0x01;
	 DataPack.EncodingFormat = 0x00;
	 DataPack.Text = str;

    u8 buf_len = 5;
    u8 buf[buf_len];
    buf[0] = DataPack.DataHead;
    buf[1] = DataPack.Length_HH;
    buf[2] = DataPack.Length_LL;
    buf[3] = DataPack.Commond;
    buf[4] = DataPack.EncodingFormat;
	message.buf = buf;
    message.buf_length = buf_len;
    ret = FFreeRTOSI2cTransfer(os_i2c_master, &message);
    if (ret != FREERTOS_I2C_SUCCESS)
    {
        vPrintf("FFreeRTOSI2cTransfer write poll task error,i2c id:%d.\r\n", os_i2c_master->i2c_device.config.instance_id);
    }

    message.buf = str;
    message.buf_length = strlen(str);
    ret = FFreeRTOSI2cTransfer(os_i2c_master, &message);
	 
}

void TextCtrl(char c, int d)
{
  char str[10];
  if (d != -1)
    sprintf(str, "[%c%d]", c, d);
  else
    sprintf(str, "[%c]", c);
  SetBase(str);
	
}

// void SetVolume(int volume)
// {
//     f_printk("SetVolume");
//     TextCtrl('v', volume);
// 	while(GetChipStatus() != ChipStatus_Idle)
// 	{
// 	//  delay(10);
//         f_printk("%x\r\n",GetChipStatus());
//         fsleep_millisec(10);
// 	}
//     f_printk("SetVolume finish!");
// }

// void SetReader(Reader_Type reader)
// {
//   TextCtrl('m', reader);
// 	while(GetChipStatus() != ChipStatus_Idle)
// 	{
// 	//  delay(10);
//         fsleep_millisec(10);
// 	}
// }

BaseType_t FFreeRTOSI2cLoopbackCreate(void)
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */
    BaseType_t xTimerStarted = pdPASS;

    taskENTER_CRITICAL(); //进入临界区

    xReturn = xTaskCreate((TaskFunction_t)GetChipStatus,  /* 任务入口函数 */
                          (const char *)"GetChipStatus",/* 任务名字 */
                          (uint16_t)1024,  /* 任务栈大小 */
                          (void *)NULL,/* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 1,  /* 任务的优先级 */
                          NULL); /* 任务控制 */
    FASSERT_MSG(xReturn == pdPASS, "I2cInitTask creation is failed.");

    taskEXIT_CRITICAL(); //退出临界区
    printf("I2c task is created successfully.\r\n");
    return xReturn;
}