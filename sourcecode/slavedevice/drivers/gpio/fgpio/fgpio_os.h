/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
 * All Rights Reserved.
 *
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,
 * either version 1.0 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details.
 *
 *
 * FilePath: fgpio_os.h
 * Date: 2022-07-22 11:33:45
 * LastEditTime: 2022-07-22 11:33:45
 * Description:  This file is for providing function related definitions of gpio driver used in FreeRTOS.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/7/27   init commit
 */

#ifndef  FGPIO_OS_H
#define  FGPIO_OS_H
/***************************** Include Files *********************************/
#include <FreeRTOS.h>
#include <semphr.h>
#include <event_groups.h>

#include "fkernel.h"
#include "fparameters.h"
#include "fgpio.h"
/************************** Constant Definitions *****************************/
#ifdef __cplusplus
extern "C"
{
#endif

#define FFREERTOS_GPIO_OK                   FT_SUCCESS
#define FFREERTOS_GPIO_NOT_INIT             FT_CODE_ERR(ErrModPort, ErrBspGpio, 0)
#define FFREERTOS_GPIO_SEMA_ERR             FT_CODE_ERR(ErrModPort, ErrBspGpio, 1)
#define FFREERTOS_GPIO_ALREADY_INIT         FT_CODE_ERR(ErrModPort, ErrBspGpio, 2)
#define FFREERTOS_GPIO_NOT_SUPPORT          FT_CODE_ERR(ErrModPort, ErrBspGpio, 3)
#define FFREERTOS_GPIO_WAIT_EVT_TIMOUT      FT_CODE_ERR(ErrModPort, ErrBspGpio, 4)

#define FFREERTOS_GPIO_IRQ_PRIORITY         IRQ_PRIORITY_VALUE_12

/* format gpio pin index */
#define FFREERTOS_GPIO_PIN_INDEX(ctrl, port, pin) SET_REG32_BITS(ctrl, 31, 16) | \
                                                  SET_REG32_BITS(port, 15, 12) | \
                                                  SET_REG32_BITS(pin, 11, 0)
#define FFREERTOS_GPIO_PIN_CTRL_ID(pin_idx)       GET_REG32_BITS(pin_idx, 31, 16)
#define FFREERTOS_GPIO_PIN_PORT_ID(pin_idx)       GET_REG32_BITS(pin_idx, 15, 12)
#define FFREERTOS_GPIO_PIN_ID(pin_idx)            GET_REG32_BITS(pin_idx, 11, 0)


#define FFREERTOS_GPIO_LEVEL_LOW            0U
#define FFREERTOS_GPIO_LEVEL_HIGH           1U
/**************************** Type Definitions *******************************/

typedef struct
{
    FGpio ctrl; /* driver instance */
    FGpioPin pins[FGPIO_PORT_NUM][FGPIO_PIN_NUM]; /* pin instance */
    SemaphoreHandle_t locker; /* locker of freertos instance */
} FFreeRTOSFGpio; /* freertos gpio instance */

typedef void (*FFreeRTOSFGpioPinIrqHandler)(s32 vector, void *param);

typedef struct
{

} FFreeRTOSGpioConfig; /* freertos gpio config, reserved for future use */

typedef struct
{
    u32 pin_idx; /* use FFREERTOS_GPIO_PIN_INDEX to define */
    FGpioDirection mode; /* pin direction */
    boolean en_irq; /* TRUE: enable irq */
    FGpioIrqType irq_type; /* pin irq type */
    FFreeRTOSFGpioPinIrqHandler irq_handler;
    void *irq_args;
} FFreeRTOSGpioPinConfig; /* freertos pin config */

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/* init and get gpio instance */
FFreeRTOSFGpio *FFreeRTOSGpioInit(u32 instance_id, const FFreeRTOSGpioConfig *config);

/* deinit gpio instance */
FError FFreeRTOSGpioDeInit(FFreeRTOSFGpio *const instance);

/* config and setup pin */
FError FFreeRTOSSetupPin(FFreeRTOSFGpio *const instance, const FFreeRTOSGpioPinConfig *config);

/* enable/disable interrupt of pin */
FError FFreeRTOSSetIRQ(FFreeRTOSFGpio *const instance, u32 pin, boolean en_irq);

/* set output pin value */
FError FFreeRTOSPinWrite(FFreeRTOSFGpio *const instance, u32 pin, u32 value);

/* get input pin value */
u32 FFreeRTOSPinRead(FFreeRTOSFGpio *const instance, u32 pin);

#ifdef __cplusplus
}
#endif

#endif