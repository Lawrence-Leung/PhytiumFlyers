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
 * FilePath: portserial.c
 * Date: 2022-09-29 18:08:50
 * LastEditTime: 2022-09-29 18:08:50
 * Description:  This file is for serial port
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  liushengming 2022/09/29    first commit
 */
#include <stdio.h>
#include <string.h>
#include "port.h"
#include "fpl011.h"
#include "fpl011_hw.h"
#include "fassert.h"
#include "fparameters.h"
#include "finterrupt.h"
#include "fdebug.h"
#include "ferror_code.h"
#include "fcpu_info.h"
#include "fiopad.h"
/* ----------------------- modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- define -------------------------------------------*/
#define MODBUS_DEBUG_TAG "PORT_SERIAL"

#define MODBUS_DEBUG_I(format, ...) FT_DEBUG_PRINT_I(MODBUS_DEBUG_TAG, format, ##__VA_ARGS__)
#define MODBUS_DEBUG_W(format, ...) FT_DEBUG_PRINT_W(MODBUS_DEBUG_TAG, format, ##__VA_ARGS__)
#define MODBUS_DEBUG_E(format, ...) FT_DEBUG_PRINT_E(MODBUS_DEBUG_TAG, format, ##__VA_ARGS__)
/* ----------------------- static variables ---------------------------------*/
static FPl011 serial;
static FIOPadCtrl iopad_ctrl;
static FPl011Format format;
static CHAR data_buff[256];
volatile CHAR rx_data;
static FPl011Format format_default = {
    .baudrate = FPL011_BAUDRATE,
    .data_bits = FPL011_FORMAT_WORDLENGTH_8BIT,
    .parity = FPL011_FORMAT_NO_PARITY,
    .stopbits = FPL011_FORMAT_1_STOP_BIT};

/* ----------------------- static functions ---------------------------------*/
static void FUartISR(s32 vector, void *param);

/* ----------------------- Start implementation -----------------------------*/
void vMBPortSerialEnable( BOOL RxEnable, BOOL TxEnable )
{
    /* If xRXEnable enable serial receive interrupts. If xTxENable enable
     * transmitter empty interrupts.
     * !!! we close send interrupts,use poll mode.
     */
    u32 intr_temp = FPl011GetInterruptMask(&serial);
    FASSERT(!RxEnable || !TxEnable);
    if (RxEnable)
    {
        FPl011SetInterruptMask(&serial, intr_temp | FPL011IMSC_RXIM);
    }
    else
    {
        FPl011SetInterruptMask(&serial, (intr_temp & (~FPL011IMSC_RXIM)));
    }
    intr_temp = FPl011GetInterruptMask(&serial);
    MODBUS_DEBUG_I("Intr:0x%x.",intr_temp);
}

BOOL xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
    FPl011Config config_value;
    const FPl011Config *config_p;
    FError ret;
    config_p = FPl011LookupConfig(ucPORT);
    if (NULL == config_p)
    {
        MODBUS_DEBUG_E("Lookup ID is error.");
        return ERR_GENERAL;
    }
    memcpy(&config_value,config_p,sizeof(FPl011Config)) ;

    FIOPadCfgInitialize(&iopad_ctrl, FIOPadLookupConfig(FIOPAD0_ID));
#if defined(CONFIG_TARGET_E2000D)
    switch (ucPORT)
    {
        case FUART0_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_J33_REG0_OFFSET, FIOPAD_FUNC4);
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_J35_REG0_OFFSET, FIOPAD_FUNC4);
            break;
        case FUART1_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AW47_REG0_OFFSET, FIOPAD_FUNC0);
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AU47_REG0_OFFSET, FIOPAD_FUNC0);
            break;
        case FUART2_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_A43_REG0_OFFSET, FIOPAD_FUNC0);
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_A45_REG0_OFFSET, FIOPAD_FUNC0);
            break;
        case FUART3_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_L33_REG0_OFFSET, FIOPAD_FUNC2);
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_N31_REG0_OFFSET, FIOPAD_FUNC2);
            break;
        default:
            break;
    }
#elif defined(CONFIG_TARGET_E2000Q)
    switch (ucPORT)
    {
        case FUART0_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_J37_REG0_OFFSET, FIOPAD_FUNC4);
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_J39_REG0_OFFSET, FIOPAD_FUNC4);
            break;
        case FUART1_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AW51_REG0_OFFSET, FIOPAD_FUNC0);
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AU51_REG0_OFFSET, FIOPAD_FUNC0);
            break;
        case FUART2_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_A47_REG0_OFFSET, FIOPAD_FUNC0);
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_A49_REG0_OFFSET, FIOPAD_FUNC0);
            break;
        case FUART3_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_L37_REG0_OFFSET, FIOPAD_FUNC2);
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_N35_REG0_OFFSET, FIOPAD_FUNC2);
            break;
        default:
            break;
    }
#endif

    ret = FPl011CfgInitialize(&serial, &config_value);
    if(ret != FT_SUCCESS)
    {
        MODBUS_DEBUG_E("Uart FPl011CfgInitialize is error.");
        return ERR_GENERAL;
    }

    format.baudrate = ulBaudRate;
    switch(ucDataBits)
    {
        case 8:
            format.data_bits = FPL011_FORMAT_WORDLENGTH_8BIT;     
        break;
        case 7:
            format.data_bits = FPL011_FORMAT_WORDLENGTH_7BIT;     
        break;
        case 6:
            format.data_bits = FPL011_FORMAT_WORDLENGTH_6BIT;     
        break;
        case 5:
            format.data_bits = FPL011_FORMAT_WORDLENGTH_5BIT;     
        break;
        default:
            format.data_bits = FPL011_FORMAT_WORDLENGTH_8BIT;     
        break;
    }
    format.stopbits = FPL011_FORMAT_1_STOP_BIT;
    format.parity = FPL011_FORMAT_EVEN_PARITY;
    ret = FPl011SetDataFormat(&serial, &format);
    if(ret != FT_SUCCESS)
    {
        MODBUS_DEBUG_E("Uart FPl011SetDataFormat initialize is error ");
        return ERR_GENERAL;
    }

    FPl011SetRxFifoThreadhold(&serial,FPL011IFLS_RXIFLSEL_1_8);
    FPl011SetTxFifoThreadHold(&serial,FPL011IFLS_TXIFLSEL_1_8);
    /* Start Uart */
    FPl011SetOptions(&serial,FPL011_OPTION_UARTEN|FPL011_OPTION_RXEN| FPL011_OPTION_FIFOEN |FPL011_OPTION_TXEN);
    MODBUS_DEBUG_I("FTestUartInit baudrate:%d,data_bits:%d,parity:%d,stopbits:%d is ok.", format.baudrate, format.data_bits + 5, format.parity, format.stopbits + 1);

    u32 cpu_id;
    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(serial.config.irq_num, cpu_id);

    InterruptInstall(serial.config.irq_num, (IrqHandler)FUartISR, &serial, "modbus");
    InterruptUmask(serial.config.irq_num);
    MODBUS_DEBUG_I("Init OK.");
    return TRUE;
}

void xMBPortSerialClose( void )
{
    FError ret;
    /*load default farmat*/
    ret = FPl011SetDataFormat(&serial, &format_default);
    if(ret != FT_SUCCESS)
    {
        MODBUS_DEBUG_E("Pl011ResetDataFormat is error ");
        return;
    }
    /* Stop Uart */
    u32 reg = 0U;
    reg &= ~(FPL011_OPTION_UARTEN | FPL011_OPTION_RXEN | FPL011_OPTION_FIFOEN | FPL011_OPTION_TXEN);
    FPl011SetOptions(&serial,reg);
    /*set is_ready to no_ready*/
    serial.is_ready = 0U;
}

BOOL xMBPortSerialPutByte( CHAR ucByte )
{
    /* Put a byte in the UARTs transmit buffer. This function is called
     * by the protocol stack if pFModBusFrameCBTransmitterEmpty( ) has been
     * called. */
    FASSERT(serial.is_ready == FT_COMPONENT_IS_READY);
    MODBUS_DEBUG_I("Send ucByte:0x%x.", ucByte);
    if(!FUART_ISTRANSMITFULL(serial.config.base_address))
    {
        FUART_WRITEREG32(serial.config.base_address, FPL011DR_OFFSET, ucByte);
    }
    return TRUE;
}

BOOL xMBPortSerialGetByte( CHAR * pucByte )
{
    /* Return the byte in the UARTs receive buffer. This function is called
     * by the protocol stack after pFModBusFrameCBByteReceived( ) has been called.
     */
    *pucByte = rx_data;
    return TRUE;
}

void FUartISR(s32 vector, void *param)
{
    FPl011 *uart_p = (FPl011 *)param;
    u32 reg_value = 0;
    u8 times=0;
    FASSERT(uart_p != NULL);
    FASSERT(uart_p->is_ready == FT_COMPONENT_IS_READY);
    
    reg_value = FUART_READREG32(uart_p->config.base_address, FPL011IMSC_OFFSET);
    reg_value &= FUART_READREG32(uart_p->config.base_address, FPL011MIS_OFFSET);

    MODBUS_DEBUG_I("Uart intr reg:0x%x.",reg_value);
    if ((reg_value & ((u32)FPL011MIS_RXMIS)) != (u32)0)
    {
        /* Received data interrupt */
        while( FUART_RECEIVEDATAEMPTY(uart_p->config.base_address) == 0 )
        {
            rx_data = FUART_READREG32(uart_p->config.base_address, FPL011DR_OFFSET);
            pxMBFrameCBByteReceived();
        }
    }
}
