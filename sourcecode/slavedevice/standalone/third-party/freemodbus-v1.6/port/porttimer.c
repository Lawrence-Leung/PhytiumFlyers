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
 * FilePath: porttimer.c
 * Date: 2022-09-29 18:09:10
 * LastEditTime: 2022-09-29 18:09:10
 * Description:  This file is for timer port
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  liushengming 2022/09/29    first commit
 */
/* ----------------------- Platform includes --------------------------------*/
#include <stdio.h>
#include <string.h>
#include "port.h"
#include "fgeneric_timer.h"
#include "finterrupt.h"
#include "sdkconfig.h"
#include "ferror_code.h"
#include "fdebug.h"
#include "fsleep.h"
#include "fassert.h"
#include "fcpu_info.h"
#if defined(CONFIG_USE_PER_TIMER)
#include "ftimer_tacho_hw.h"
#include "ftimer_tacho.h"
#endif
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- define -------------------------------------------*/
#define MODBUS_DEBUG_TAG "PORT_TIMER"

#define MODBUS_DEBUG_I(format, ...) FT_DEBUG_PRINT_I(MODBUS_DEBUG_TAG, format, ##__VA_ARGS__)
#define MODBUS_DEBUG_W(format, ...) FT_DEBUG_PRINT_W(MODBUS_DEBUG_TAG, format, ##__VA_ARGS__)
#define MODBUS_DEBUG_E(format, ...) FT_DEBUG_PRINT_E(MODBUS_DEBUG_TAG, format, ##__VA_ARGS__)

/* ----------------------- static functions ---------------------------------*/

static void FTimerExpiredISR(void);
static u64 tickrate_hz;
#if defined(CONFIG_USE_PER_TIMER)

#define TIMER_ID 33 /*0-37*/
#define US2TICKS(us) ((FTIMER_CLK_FREQ_HZ * (us) / 1000000ULL) + 1ULL)

u64 CntTick = 0;
typedef struct
{
    u32 id;              /* id of timer */
    boolean bits32;      /*otherwise 64 bit*/
    boolean restartmode; /*otherwise free-run*/
    boolean cyc_cmp;     /*otherwise once cmp*/
    boolean clear_cnt;   /*otherwise not clear*/
    boolean forceload;   /*otherwise not force-load*/
    u32 startcnt;        /*start cnt num*/
    u32 cmptick32;       /*32bit cnt num*/
    u64 cmptick64;       /*64bit cnt num*/
} TimerTestConfigs;

static FTimerTachoCtrl timer;
static TimerTestConfigs timercfg;

/**
 * @name: TimerEnableIntr
 * @msg: 设置并且使能中断
 * @return {void}
 * @param  {FTimerTachoCtrl} *instance_p 驱动控制数据结构
 */
void TimerEnableIntr(FTimerTachoCtrl *instance_p)
{
    FASSERT(instance_p);

    u32 irq_num = FTIMER_TACHO_IRQ_NUM(instance_p->config.id);

    u32 cpu_id;
    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(irq_num, cpu_id);
    /* disable timer irq */
    InterruptMask(irq_num);

    /* umask timer irq */
    InterruptSetPriority(irq_num, IRQ_PRIORITY_VALUE_11);
    InterruptInstall(irq_num, (IrqHandler)FTimerExpiredISR, instance_p, instance_p->config.name);

    FTimerTachoSetIntr(instance_p);
    /* enable irq */
    InterruptUmask(irq_num);

    return;
}

/**
 * @name: FTimerCfgInit
 * @msg: 加载转换后配置项，并完成初始化操作，定时器处于就绪状态
 * @return {FError} 驱动初始化的错误码信息，FTIMER_TACHO_SUCCESS 表示初始化成功，其它返回值表示初始化失败
 * @param {TimerTestConfigs} *timercfg_p 可操作的配置参数结构体
 */
static FError FTimerCfgInit(const TimerTestConfigs *timercfg_p)
{
    u32 ret = FT_SUCCESS;

    FTimerTachoConfig *pconfig = &timer.config;

    memset(&timer, 0, sizeof(timer));
    FTimerGetDefConfig(timercfg_p->id, pconfig);

    if (timercfg_p->restartmode)
    {
        pconfig->timer_mode = FTIMER_RESTART;
    }
    else
    {
        pconfig->timer_mode = FTIMER_FREE_RUN;
    }

    if (timercfg_p->bits32)
    {
        pconfig->timer_bits = FTIMER_32_BITS;
    }
    else
    {
        pconfig->timer_bits = FTIMER_64_BITS;
    }

    if (timercfg_p->cyc_cmp)
    {
        pconfig->cmp_type = FTIMER_CYC_CMP;
    }
    else
    {
        pconfig->cmp_type = FTIMER_ONCE_CMP;
    }
    ret = FTimerInit(&timer, pconfig);
    if (FTIMER_TACHO_SUCCESS != ret)
    {
        return ret;
    }

    /*将时间参数us装换成计时器的ticks，我们设置StartTick，将CmpTick设置为最大*/
    FTimerSetStartVal(&timer, timercfg.startcnt);

    if (timercfg_p->bits32)
        ret |= FTimerSetPeriod32(&timer, timercfg.cmptick32);
    else
        ret |= FTimerSetPeriod64(&timer, timercfg.cmptick64);
    return ret;
}

#endif

/* ----------------------- Start implementation -----------------------------*/
BOOL xMBPortTimersInit(USHORT usTim1Timerout50us)
{
    MODBUS_DEBUG_I("Timer init.");
#if defined(CONFIG_USE_PER_TIMER)
    timercfg.id = TIMER_ID;
    timercfg.cyc_cmp = FTIMER_ONCE_CMP;
    timercfg.bits32 = TRUE;
    timercfg.startcnt = (GENMASK(31, 0)) - US2TICKS(50 * usTim1Timerout50us);
    timercfg.cmptick32 = GENMASK(31, 0);
    timercfg.cmptick64 = GENMASK_ULL(63, 0);
    FTimerCfgInit(&timercfg);
    TimerEnableIntr(&timer);
    return TRUE;
#else
    /*20000 = 1s / 50 us*/
    tickrate_hz = (20000 / usTim1Timerout50us) - 1;

    /* stop timer */
    GenericTimerStop(GENERIC_TIMER_ID0);
    
    /* setup and enable interrupt */
    InterruptSetPriority(GENERIC_TIMER_NS_IRQ_NUM, IRQ_PRIORITY_VALUE_11);
    InterruptInstall(GENERIC_TIMER_NS_IRQ_NUM, FTimerExpiredISR, NULL, NULL);
    InterruptUmask(GENERIC_TIMER_NS_IRQ_NUM);

	GenericTimerSetTimerValue(GENERIC_TIMER_ID0, GenericTimerFrequecy() / tickrate_hz);

	GenericTimerStart(GENERIC_TIMER_ID0);

    GenericTimerInterruptDisable(GENERIC_TIMER_ID0);

    return TRUE;
    
#endif
}

void xMBPortTimersClose(void)
{
#if defined(CONFIG_USE_PER_TIMER)
    /* stop timer first */
    FTimerStop(&timer);
    /* reset reg*/
    FTimerSoftwareReset(&timer);

    timer.isready = 0;
    memset(&timer, 0, sizeof(timer));
#endif
}

void vMBPortTimersDelay(USHORT usTimeOutMS)
{
    fsleep_millisec(usTimeOutMS);
}

inline void
vMBPortTimersEnable()
{
    MODBUS_DEBUG_I("Enable timer.");
    /* Enable the timer with the timeout passed to xMBPortTimersInit( ) */
#if defined(CONFIG_USE_PER_TIMER)
    FTimerSetStartVal(&timer, timercfg.startcnt);
    FTimerStart(&timer);
#else
    GenericTimerSetTimerValue(GENERIC_TIMER_ID0, GenericTimerFrequecy() / tickrate_hz);
    GenericTimerInterruptEnable(GENERIC_TIMER_ID0);
#endif
}

inline void
vMBPortTimersDisable()
{
    /* Disable any pending timers. */
#if defined(CONFIG_USE_PER_TIMER)
    u32 intr_status = FTIMER_INTR_S_READ(&timer);
    FTIMER_INTR_S_CLEAR(&timer, intr_status);
    FTimerStop(&timer);
#endif
    GenericTimerInterruptDisable(GENERIC_TIMER_ID0);
}

/* Create an ISR which is called whenever the timer has expired. This function
 * must then call pxMBPortCBTimerExpired( ) to notify the protocol stack that
 * the timer has expired.
 */
static void FTimerExpiredISR(void)
{
    (void)pxMBPortCBTimerExpired();
    GenericTimerSetTimerValue(GENERIC_TIMER_ID0, GenericTimerFrequecy() / tickrate_hz);
}
