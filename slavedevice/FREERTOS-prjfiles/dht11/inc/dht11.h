#ifndef  DHT11_H
#define  DHT11_H

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include <stdio.h>
#include "fgpio_os.h"
/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
void DHT11_IO_Init_OUT(void);
void DHT11_IO_Init_IN(void);
void DHT11_Rst(void);
u8 DHT11_Check(void);
u8 DHT11_Read_Bit(void);
u8 DHT11_Read_Byte(void);
// u8 DHT11_Read_Data(u8 *temp,u8 *humi);
u8 DHT11_Read_Data(u8 *temp,u8 *humi);
void dht11_Task(void);
BaseType_t AppTask(void);

// u8 humi;
// u8 temp;

#ifdef __cplusplus
}
#endif

#endif
