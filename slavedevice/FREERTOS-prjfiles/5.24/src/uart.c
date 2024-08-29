#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "ftypes.h"
// #include "shell.h"
// #include "shell_port.h"
#include "uart.h"
#include "fpl011_os.h"
#include "gps.h"

#define USART_REC_LEN  			200  	//定义最大接收字节数 200
#define EN_USART1_RX 			1		//使能（1）/禁止（0）串口1接收


#define TX_BUFFER   10000
#define RX_BUFFER   10000
static u8 send_buffer_i[TX_BUFFER] = "hello world!\r\n"; /*intr Buffer for Transmitting Data */
static u8 recv_buffer_i[RX_BUFFER]; /*intr Buffer for Receiving Data */
char rx[10];
u16 cnt = 0;
_SaveData Save_Data;

char USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
u16 USART_RX_STA;         		//接收状态标记	

extern double latitude;
extern double longitude;

FtFreertosUart uart;
FtFreertosUartConfig uart_config =
{
    .uart_instance = UART2_ID,              //TX pin8，RX pin10
    .isr_priority = IRQ_PRIORITY_VALUE_13,  /* irq Priority */
    .isr_event_mask = (RTOS_UART_ISR_OEIM_MASK | RTOS_UART_ISR_BEIM_MASK | RTOS_UART_ISR_PEIM_MASK | RTOS_UART_ISR_FEIM_MASK | RTOS_UART_ISR_RTIM_MASK | RTOS_UART_ISR_RXIM_MASK),
    .uart_baudrate = 9600
};

void GPSInit(void)
{
    FtFreertosUartInit(&uart,&uart_config );
}

void UartWaitLoop(void)
{
    u32 recive_length = 0;
    u32 i,j = 0;
    

    f_printk("uart ready!!");
    // while (1)
    // {
        // taskENTER_CRITICAL();
        FtFreertosUartReceiveBuffer(&uart, recv_buffer_i, sizeof(recv_buffer_i), &recive_length);
        // taskEXIT_CRITICAL();
        // taskENTER_CRITICAL();
        f_printk("GPS:recv!\r\n");
        for (i = 0; i < recive_length; i++)
        {
            printf("%c",recv_buffer_i[i]);
        }
        printf("\r\n");
        // taskEXIT_CRITICAL();
        // taskENTER_CRITICAL();
        for (i = 0; i < recive_length; i++)
        {
            // f_printk("%c",recv_buffer_i[i]);
            if(recv_buffer_i[i] == '$')
            {
                cnt = 0;
                // f_printk("/r/n");
            }
            USART_RX_BUF[cnt++] = recv_buffer_i[i];

            if(USART_RX_BUF[0] == '$' && USART_RX_BUF[4] == 'M' && USART_RX_BUF[5] == 'C')			//确定是否收到"GPRMC/GNRMC"这一帧数据
            {
                if(recv_buffer_i[i] == '\n')									   
                {
                    // for(j = 0;j<sizeof(USART_RX_BUF);j++)
                    // {
                    //     f_printk("%c",USART_RX_BUF[j]);
                    // }
                    // f_printk("\r\n");
                    memset(Save_Data.GPS_Buffer, 0, GPS_Buffer_Length);      //清空
                    // f_printk("cnt:%d\r\n",cnt);
                    memcpy(Save_Data.GPS_Buffer, USART_RX_BUF, cnt); 	//保存数据
                    // f_printk("Save_Data:");
                    // for(j = 0;j<sizeof(sizeof(Save_Data.GPS_Buffer));j++)
                    // {
                    //     f_printk("%c",Save_Data.GPS_Buffer[j]);
                    // }
                    // f_printk("\r\n");
                    Save_Data.isGetData = true;
                    cnt = 0;
                    memset(USART_RX_BUF, 0, USART_REC_LEN);      //清空	
                    
                }	
                        
            }
        }
        // taskEXIT_CRITICAL();

        // taskENTER_CRITICAL();
        parseGpsBuffer();
        // taskEXIT_CRITICAL();

        // taskENTER_CRITICAL();
        printGpsBuffer(&latitude,&longitude);
        // taskEXIT_CRITICAL();
        // vTaskDelay(2000 / portTICK_PERIOD_MS);
    // }
    // vTaskDelete(NULL);
}


// void UartTaskCreate(void *args)
// {
//     BaseType_t ret;
//     FtFreertosUartInit(&uart,&uart_config );


//     ret = xTaskCreate((TaskFunction_t)UartWaitLoop,  /* 任务入口函数 */
//                       (const char *)"UartWaitLoop",/* 任务名字 */
//                       (uint16_t)1024,  /* 任务栈大小 */
//                       (void *)NULL,/* 任务入口函数参数 */
//                       (UBaseType_t)2,  /* 任务的优先级 */
//                       NULL); /* 任务控制块指针 */

//     FASSERT_MSG(ret == pdPASS, "UartWaitLoop create is failed");

//     vTaskDelete(NULL);
// }

// BaseType_t AppTask(void)
// {
//     return xTaskCreate((TaskFunction_t)UartTaskCreate,  /* 任务入口函数 */
//                        (const char *)"LSUserShellTaskCreate",/* 任务名字 */
//                        (uint16_t)1024,  /* 任务栈大小 */
//                        (void *)NULL,/* 任务入口函数参数 */
//                        (UBaseType_t)2,  /* 任务的优先级 */
//                        NULL); /* 任务控制块指针 */
// }