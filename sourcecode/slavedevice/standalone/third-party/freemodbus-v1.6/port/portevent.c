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
 * FilePath: portevent.c
 * Date: 2022-09-29 18:08:02
 * LastEditTime: 2022-09-29 18:08:02
 * Description:  This file is for port event function
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  liushengming 2022/09/29    first commit
 */

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- Variables ----------------------------------------*/
static eMBEventType queued_event;
static BOOL     event_in_queue;

/* ----------------------- Start implementation -----------------------------*/
BOOL xMBPortEventInit( void )
{
    event_in_queue = FALSE;
    return TRUE;
}

BOOL xMBPortEventPost( eMBEventType event )
{
    event_in_queue = TRUE;
    queued_event = event;
    return TRUE;
}

BOOL xMBPortEventGet( eMBEventType * event )
{
    BOOL            event_happened = FALSE;

    if( event_in_queue )
    {
        *event = queued_event;
        event_in_queue = FALSE;
        event_happened = TRUE;
    }
    return event_happened;
}
