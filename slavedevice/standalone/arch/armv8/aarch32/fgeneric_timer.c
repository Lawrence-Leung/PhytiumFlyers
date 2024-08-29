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
 * FilePath: fgeneric_timer.c
 * Date: 2022-02-10 14:53:41
 * LastEditTime: 2022-02-17 17:30:07
 * Description:  This file provides the common helper routines for the generic timer API's
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  huanghe	2021-11		initialization
 *  1.1  zhugengyu 2022/06/05     add tick api
 *  1.2  wangxiaodong 2023/05/29  modify api
 */


#include "fparameters.h"
#include "fgeneric_timer.h"
#include "faarch32.h"
#include "sdkconfig.h"
#include "fassert.h"
#include "fkernel.h"

#define CTL_ENABLE_MASK BIT(0)
#define CTL_INTERRUPT_MASK BIT(1)

/**
 * @name: GenericTimerSetTimerCompareValue
 * @msg:  Set generic timer CompareValue
 * @param {u32} id, id of generic timer, non-secoure physical timer or virtual timer
 * @param {u64} timeout, timeout value
 * @return {void}
 */
void GenericTimerSetTimerCompareValue(u32 id, u64 timeout)
{
    FASSERT_MSG((id == GENERIC_TIMER_ID0) || (id == GENERIC_TIMER_ID1), "Please use correct int id");

    if(id == GENERIC_TIMER_ID0)
    {
        AARCH32_WRITE_SYSREG_64(CNTP_CVAL_64, timeout);
    }
    else
    {
        AARCH32_WRITE_SYSREG_64(CNTV_CVAL_64, timeout);
    }
}

/**
 * @name: GenericTimerSetTimerValue
 * @msg:  Set generic timer TimerValue
 * @param {u32} id, id of generic timer, non-secoure physical timer or virtual timer
 * @param {u64} timeout, timeout value
 * @return {void}
 */
void GenericTimerSetTimerValue(u32 id, u32 timeout)
{
    FASSERT_MSG((id == GENERIC_TIMER_ID0) || (id == GENERIC_TIMER_ID1), "Please use correct int id");

    if(id == GENERIC_TIMER_ID0)
    {
        AARCH32_WRITE_SYSREG_32(CNTP_TVAL, timeout);
    }
    else
    {
        AARCH32_WRITE_SYSREG_32(CNTV_TVAL, timeout);
    }
}

/**
 * @name: GenericTimerInterruptEnable
 * @msg:  Unmask generic timer interrupt
 * @param {u32} id, id of generic timer, non-secoure physical timer or virtual timer
 * @return {void}
 */
void GenericTimerInterruptEnable(u32 id)
{

    FASSERT_MSG((id == GENERIC_TIMER_ID0) || (id == GENERIC_TIMER_ID1), "Please use correct int id");
    
    u32 ctrl = 0;
    if (id == GENERIC_TIMER_ID0)
    {
        ctrl = AARCH32_READ_SYSREG_32(CNTP_CTL);
        ctrl &= (~CTL_INTERRUPT_MASK);
        AARCH32_WRITE_SYSREG_32(CNTP_CTL, ctrl);
    }
    else
    {
        ctrl = AARCH32_READ_SYSREG_32(CNTV_CTL);
        ctrl &= (~CTL_INTERRUPT_MASK);
        AARCH32_WRITE_SYSREG_32(CNTV_CTL, ctrl);
    }

}

/**
 * @name: GenericTimerInterruptDisable
 * @msg:  Mask generic timer interrupt
 * @param {u32} id, id of generic timer, non-secoure physical timer or virtual timer
 * @return {void}
 */
void GenericTimerInterruptDisable(u32 id)
{
    FASSERT_MSG((id == GENERIC_TIMER_ID0) || (id == GENERIC_TIMER_ID1), "Please use correct int id");
    u32 ctrl = 0;
    if (id == GENERIC_TIMER_ID0)
    {
        ctrl = AARCH32_READ_SYSREG_32(CNTP_CTL);
        ctrl |= CTL_INTERRUPT_MASK;
        AARCH32_WRITE_SYSREG_32(CNTP_CTL, ctrl);
        
    }
    else
    {
        ctrl = AARCH32_READ_SYSREG_32(CNTV_CTL);
        ctrl |= CTL_INTERRUPT_MASK;
        AARCH32_WRITE_SYSREG_32(CNTV_CTL, ctrl);
    }
}

/**
 * @name: GenericTimerStart
 * @msg:  Enable generic timer
 * @param {u32} id, id of generic timer, non-secoure physical timer or virtual timer
 * @return {void}
 */
void GenericTimerStart(u32 id)
{
    FASSERT_MSG((id == GENERIC_TIMER_ID0) || (id == GENERIC_TIMER_ID1), "Please use correct int id");
    u32 ctrl = 0;
    if (id == GENERIC_TIMER_ID0)
    {
        ctrl = AARCH32_READ_SYSREG_32(CNTP_CTL);
        ctrl |= CTL_ENABLE_MASK;
        AARCH32_WRITE_SYSREG_32(CNTP_CTL, ctrl);
    }
    else
    {
        ctrl = AARCH32_READ_SYSREG_32(CNTV_CTL);
        ctrl |= CTL_ENABLE_MASK;
        AARCH32_WRITE_SYSREG_32(CNTV_CTL, ctrl);
    }

}

/**
 * @name: GenericTimerStop
 * @msg:  Disable generic timer
 * @param {u32} id, id of generic timer, non-secoure physical timer or virtual timer
 * @return {void}
 */
void GenericTimerStop(u32 id)
{
    FASSERT_MSG((id == GENERIC_TIMER_ID0) || (id == GENERIC_TIMER_ID1), "Please use correct int id");
    
    u32 ctrl = 0;
    if (id == GENERIC_TIMER_ID0)
    {
        ctrl = AARCH32_READ_SYSREG_32(CNTP_CTL);
        ctrl &= ~CTL_ENABLE_MASK;
        AARCH32_WRITE_SYSREG_32(CNTP_CTL, ctrl);
    }
    else
    {
        ctrl = AARCH32_READ_SYSREG_32(CNTV_CTL);
        ctrl &= ~CTL_ENABLE_MASK;
        AARCH32_WRITE_SYSREG_32(CNTV_CTL, ctrl);
    }
}

/**
 * @name: GenericTimerFrequecy
 * @msg:  Get generic timer frequency of the system counter
 * @param {null} 
 * @return {u32} frequency of the system counter
 */
u32 GenericTimerFrequecy(void)
{
    u32 rate = AARCH32_READ_SYSREG_32(CNTFRQ);
    return rate;
}

/**
 * @name: GenericTimerRead
 * @msg:  Get generic timer physical count value
 * @param {u32} id, id of generic timer, non-secoure physical timer or virtual timer
 * @return {u64} physical count value
 */
u64 GenericTimerRead(u32 id)
{
    FASSERT_MSG((id == GENERIC_TIMER_ID0) || (id == GENERIC_TIMER_ID1), "Please use correct int id");

    if(id == GENERIC_TIMER_ID0)
    {
	    return AARCH32_READ_SYSREG_64(CNTPCT_64);
    }
    else
    {
        return AARCH32_READ_SYSREG_64(CNTVCT_64);
    }
}
