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
 * FilePath: ftimer_tacho_os.c
 * Date: 2022-08-23 17:20:51
 * LastEditTime: 2022-08-23 17:20:51
 * Description:  This file is for required function implementations of timer tacho driver used in FreeRTOS.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 liushengming 2022/11/25  first commit
 */

#include <string.h>
#include "fkernel.h"
#include "ftimer_tacho.h"
#include "ftimer_tacho_os.h"
#include "fparameters.h"
#include "finterrupt.h"
#include "fsleep.h"
#include "fassert.h"



#define FTACHO_OS_DEBUG_TAG "FFreeRTOSTacho"
#define FTACHO_OS_ERROR(format, ...) FT_DEBUG_PRINT_E(FTACHO_OS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FTACHO_OS_WARN(format, ...)  FT_DEBUG_PRINT_W(FTACHO_OS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FTACHO_OS_INFO(format, ...)  FT_DEBUG_PRINT_I(FTACHO_OS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FTACHO_OS_DEBUG(format, ...) FT_DEBUG_PRINT_D(FTACHO_OS_DEBUG_TAG, format, ##__VA_ARGS__)

#define FTIMER_OS_DEBUG_TAG "FFreeRTOSTimer"
#define FTIMER_OS_ERROR(format, ...) FT_DEBUG_PRINT_E(FTIMER_OS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FTIMER_OS_WARN(format, ...)  FT_DEBUG_PRINT_W(FTIMER_OS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FTIMER_OS_INFO(format, ...)  FT_DEBUG_PRINT_I(FTIMER_OS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FTIMER_OS_DEBUG(format, ...) FT_DEBUG_PRINT_D(FTIMER_OS_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Constant Definitions *****************************/
/*Notice:timer are 38U,tacho nums only FTACHO_NUM = 16U,but they use the same controller num 0~15. */
static FFreeRTOSTimerTacho os_timer_tacho[38] = {0};

#define MAX_32_VAL GENMASK(31, 0)
#define MAX_64_VAL GENMASK_ULL(63, 0)
#define TACHO_MAX   10000
#define TACHO_MIN   10
#define TACHO_PERIOD 1000000 /* 1000000/50000000 = 0.02s  20ms ticks period at 50Mhz pclk*/

#define US2TICKS(us) ((FTIMER_CLK_FREQ_HZ * (us) / 1000000ULL ) + 1ULL)
#define MS2TICKS(ms) (US2TICKS(1000ULL) * (ms))

/************************** Variable Definitions *****************************/

typedef struct
{
    u32     id;             /* id of tacho */
    boolean work_mode;      /*tacho or capture*/
    boolean bits32;         /*otherwise 64 bit*/
    boolean restart_mode;    /*otherwise free-run*/
    u8      edge_mode;      /* rising falling or double edge*/
    u8      jitter_level;       /* anti_jitter_number 0~3 */
    u32     plus_num;  /* plus_num of period to calculate rpm */
} FFreeRTOSTachoConfigs;

typedef struct
{
    u32     id;             /* id of timer */
    boolean bits32;         /*otherwise 64 bit*/
    boolean restartmode;    /*otherwise free-run*/
    boolean cyc_cmp;         /*otherwise once cmp*/
    boolean clear_cnt;       /*otherwise not clear*/
    boolean forceload;      /*otherwise not force-load*/
    u32 startcnt;           /*start cnt num*/
    u32 cmptick32;          /*32bit cnt num*/
    u64 cmptick64;          /*64bit cnt num*/
} FFreeRTOSTimerConfigs;

static FFreeRTOSTimerConfigs timercfg;
static FFreeRTOSTachoConfigs tachocfg;

/************************** Function Prototypes ******************************/

/**
 * @name: FTimerCfgInit
 * @msg: 加载转换后配置项，并完成初始化操作，定时器处于就绪状态
 * @return {FError} 驱动初始化的错误码信息，FTIMER_TACHO_SUCCESS 表示初始化成功，其它返回值表示初始化失败
 * @param {TimerTestConfigs} *timercfg_p 可操作的配置参数结构体
 */
static FError FTimerCfgInit(const FFreeRTOSTimerConfigs *timercfg_p, FTimerTachoCtrl *timer)
{
    FASSERT(timercfg_p);
    FASSERT(timer);
    FError ret = FREERTOS_TIMER_TACHO_SUCCESS;

    FTimerTachoConfig *pconfig = &timer->config;

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

    ret = FTimerInit(timer, pconfig);
    if (FREERTOS_TIMER_TACHO_SUCCESS != ret)
    {
        return ret;
    }

    /*将时间参数us装换成计时器的ticks，我们设置StartTick，将CmpTick设置为最大*/
    ret = FTimerSetStartVal(timer, timercfg.startcnt);
    if (FREERTOS_TIMER_TACHO_SUCCESS != ret)
    {
        return ret;
    }

    if (timercfg_p->bits32)
    {
        ret |= FTimerSetPeriod32(timer, timercfg.cmptick32);
    }
    else
    {
        ret |= FTimerSetPeriod64(timer, timercfg.cmptick64);
    }

    FTIMER_OS_INFO("Timer Init finished.");

    return ret;
}

/**
 * @name: FTimerFunctionInit
 * @msg:  timer init.
 * @param {u8}id :use 0~37 timer
 * @param {boolean}timer_mode:单次定时还是循环定时
 * @param {u64}times:定时时间，单位us
 * @return {FFreeRTOSSpim *} return
 */
FFreeRTOSTimerTacho *FFreeRTOSTimerInit(u32 id, boolean timer_mode, u64 times)
{
    FASSERT_MSG(id < FTIMER_NUM, "Invalid timer id.");
    FASSERT_MSG(FT_COMPONENT_IS_READY != os_timer_tacho[id].ctrl.isready, "timer_tacho ready.");
    FFreeRTOSTimerTacho *instance = &os_timer_tacho[id];

    FASSERT(FT_COMPONENT_IS_READY != os_timer_tacho[id].ctrl.isready);
    FASSERT((instance->locker = xSemaphoreCreateMutex()) != NULL);

    u64 cnttick = 0;
    FTimerTachoCtrl *timer = &instance->ctrl;
    timercfg.id = id;
    timercfg.cyc_cmp = timer_mode;
    cnttick = US2TICKS(times);
    FTIMER_OS_INFO("\n***cnttick:%llu.", cnttick);
    if (cnttick > 0xffffffff)
    {
        timercfg.bits32 = FALSE;
        timercfg.startcnt = MAX_64_VAL - cnttick;
    }
    else
    {
        timercfg.bits32 = TRUE;
        timercfg.startcnt = MAX_32_VAL - cnttick;
    }
    /* Set CmpTick max value ,that we can easy to trigger RolloverIntr. */
    timercfg.cmptick32 = MAX_32_VAL;
    timercfg.cmptick64 = MAX_64_VAL;

    if (FREERTOS_TIMER_TACHO_SUCCESS != FTimerCfgInit(&timercfg, timer))
    {
        FTIMER_OS_ERROR("Timer config init failed.");
        return NULL;
    }
    return (&os_timer_tacho[id]);
}

/**
 * @name: FFTimerStartTest
 * @msg:  start timer.
 * @param {u64 times,boolean forceLoad}
 * @return {FError}
 */
FError FFreeRTOSTimerStart(FFreeRTOSTimerTacho *os_timer_p)
{
    FASSERT(NULL != os_timer_p->locker);
    FError ret = FREERTOS_TIMER_TACHO_SUCCESS;

    FTimerTachoCtrl *timer = &os_timer_p->ctrl;

    if (pdFALSE == xSemaphoreTake(os_timer_p->locker, portMAX_DELAY))
    {
        FTIMER_OS_ERROR("Timer xSemaphoreTake failed.");
        /* We could not take the semaphore, exit with 0 data received */
        return FREERTOS_TIMER_TACHO_SEMA_ERROR;
    }

    ret = FTimerStart(timer);
    if (FREERTOS_TIMER_TACHO_SUCCESS != ret)
    {
        return ret;
    }
    return ret;
}


/**
 * @name: FFreeRTOSTimerStop
 * @msg:
 * @return {*}
 * @param {FFreeRTOSTimerTacho} *os_timer_p
 */
FError FFreeRTOSTimerStop(FFreeRTOSTimerTacho *os_timer_p)
{
    FASSERT(NULL != os_timer_p->locker);

    if (pdFALSE == xSemaphoreGive(os_timer_p->locker))
    {
        /* We could not post the semaphore, exit with error */
        FTIMER_OS_ERROR("Timer xSemaphoreGive failed.");
        return FREERTOS_TIMER_TACHO_SEMA_ERROR;
    }
    FTimerTachoCtrl *timer = &os_timer_p->ctrl;

    return FTimerStop(timer);
}


/**
 * @name: FFreeRTOSTimerDeinit
 * @msg:
 * @return {*} void
 * @param {FFreeRTOSTimerTacho} *os_timer_p
 */
void FFreeRTOSTimerDeinit(FFreeRTOSTimerTacho *os_timer_p)
{
    FASSERT(NULL != os_timer_p->locker);

    FTimerDeInit(&os_timer_p->ctrl);
    vSemaphoreDelete(os_timer_p->locker);
    memset(os_timer_p, 0, sizeof(*os_timer_p));
    return;
}


/**
 * @name: FFreeRTOSTimerDebug
 * @msg: Dump timer reg message
 * @return {*}
 * @param {FFreeRTOSTimerTacho} *os_timer_p
 */
void FFreeRTOSTimerDebug(FFreeRTOSTimerTacho *os_timer_p)
{
    FASSERT(NULL != os_timer_p);
    FTimeSettingDump(&os_timer_p->ctrl);
    return;
}



/**********************************************************************************************************/
/***********************************************tacho******************************************************/
/**********************************************************************************************************/

/**
 * @name: FTachoCfgInit
 * @msg: 添加配置
 * @return {*}
 * @param {FFreeRTOSTachoConfigs} *tachocfg_p
 * @param {FTimerTachoCtrl} *tacho
 */
static FError FTachoCfgInit(const FFreeRTOSTachoConfigs *tachocfg_p, FTimerTachoCtrl *tacho)
{
    FASSERT(tachocfg_p);
    FASSERT(tacho);
    FError ret = FREERTOS_TIMER_TACHO_SUCCESS;

    FTimerTachoConfig *pconfig = &tacho->config;
    memset(tacho, 0, sizeof(tacho));
    /* tacho  */
    FTachoGetDefConfig(tachocfg_p->id, pconfig);

    if (tachocfg_p->work_mode == FTIMER_WORK_MODE_TACHO)
    {
        pconfig->work_mode = FTIMER_WORK_MODE_TACHO;

        if (tachocfg_p->bits32 == FTIMER_32_BITS)
        {
            pconfig->timer_bits = FTIMER_32_BITS;
        }
        else
        {
            pconfig->timer_bits = FTIMER_64_BITS;
        }

        if (tachocfg_p->restart_mode)
        {
            pconfig->timer_mode = FTIMER_RESTART;
        }
        else
        {
            pconfig->timer_mode = FTIMER_FREE_RUN;
        }
    }
    else
    {
        pconfig->work_mode = FTIMER_WORK_MODE_CAPTURE;
        pconfig->captue_cnt = 0x7f;/* 边沿检测计数默认值 */
    }

    if (tachocfg_p->edge_mode == FTACHO_RISING_EDGE)
    {
        pconfig->edge_mode = FTACHO_RISING_EDGE;
    }
    else if (tachocfg_p->edge_mode == FTACHO_FALLING_EDGE)
    {
        pconfig->edge_mode = FTACHO_FALLING_EDGE;
    }
    else
    {
        pconfig->edge_mode = FTACHO_DOUBLE_EDGE;
    }

    switch (tachocfg_p->jitter_level)
    {
        case FTACHO_JITTER_LEVEL0:
            pconfig->jitter_level = FTACHO_JITTER_LEVEL0;
            break;
        case FTACHO_JITTER_LEVEL1:
            pconfig->jitter_level = FTACHO_JITTER_LEVEL1;
            break;
        case FTACHO_JITTER_LEVEL2:
            pconfig->jitter_level = FTACHO_JITTER_LEVEL2;
            break;
        case FTACHO_JITTER_LEVEL3:
            pconfig->jitter_level = FTACHO_JITTER_LEVEL3;
            break;
        default:
            pconfig->jitter_level = FTACHO_JITTER_LEVEL0;
            break;
    }

    if (tachocfg_p->plus_num != 0)
    {
        pconfig->plus_num = tachocfg_p->plus_num;
    }

    ret = FTachoInit(tacho, pconfig);
}

/**
 * @name: FFreeRTOSTachoInit
 * @msg: tacho or capture init function
 * @return {*}
 * @param {u32} id
 * @param {boolean} tacho_mode
 */
FFreeRTOSTimerTacho *FFreeRTOSTachoInit(u32 id, boolean tacho_mode)
{
    FASSERT_MSG(id < FTACHO_NUM, "Invalid timer id.");
    FASSERT_MSG(FT_COMPONENT_IS_READY != os_timer_tacho[id].ctrl.isready, "timer_tacho ready.");

    FFreeRTOSTimerTacho *instance = &os_timer_tacho[id];
    FTimerTachoCtrl *tacho = &os_timer_tacho[id].ctrl;

    FASSERT((instance->locker = xSemaphoreCreateMutex()) != NULL);

    FError ret = FREERTOS_TIMER_TACHO_SUCCESS;
    tachocfg.id = id;
    tachocfg.edge_mode = FTACHO_RISING_EDGE;/* Not open operation interface for cmd */
    tachocfg.jitter_level = FTACHO_JITTER_LEVEL0;/* Not open operation interface for cmd */
    tachocfg.bits32 = FTIMER_32_BITS;/* Use capture mode, Not open operation interface for cmd.*/
    tachocfg.restart_mode = FTIMER_RESTART;/* Use capture mode, Not open operation interface for cmd.*/
    tachocfg.plus_num = TACHO_PERIOD;
    if (tacho_mode == FTIMER_WORK_MODE_TACHO)
    {
        tachocfg.work_mode = FTIMER_WORK_MODE_TACHO;
    }
    else
    {
        tachocfg.work_mode = FTIMER_WORK_MODE_CAPTURE;
    }

    ret = FTachoCfgInit(&tachocfg, tacho);
    if (ret != FREERTOS_TIMER_TACHO_SUCCESS)
    {
        FTACHO_OS_ERROR("Tacho config init failed.");
        return NULL;
    }

    if (tacho_mode == FTIMER_WORK_MODE_TACHO)
    {
        /* Not open operation interface for cmd */
        FTachoSetOverLimit(tacho, TACHO_MAX);
        FTachoSetUnderLimit(tacho, TACHO_MIN);
    }

    return (&os_timer_tacho[id]);
}

/**
 * @name: FFreeRTOSTachoGetRPM
 * @msg: get tacho RPM
 * @return {*}
 * @param {FFreeRTOSTimerTacho} *os_tacho_p
 * @param {u32} *rpm
 */
FError FFreeRTOSTachoGetRPM(FFreeRTOSTimerTacho *os_tacho_p, u32 *rpm)
{
    FASSERT(NULL != os_tacho_p->locker);
    FError ret = FREERTOS_TIMER_TACHO_SUCCESS;

    FTimerTachoCtrl *tacho = &os_tacho_p->ctrl;


    ret = FTachoGetFanRPM(tacho, rpm);
    if (ret != FREERTOS_TIMER_TACHO_SUCCESS)
    {
        FTIMER_OS_ERROR("Tachometer get error,please check init.");
        return ret;
    }
    return ret;
}

/**
 * @name: FFreeRTOSTachoGetCNT
 * @msg: get capture value
 * @return {*}
 * @param {FFreeRTOSTimerTacho} *os_tacho_p
 */
u32 FFreeRTOSTachoGetCNT(FFreeRTOSTimerTacho *os_tacho_p)
{
    FASSERT(NULL != os_tacho_p->locker);

    FTimerTachoCtrl *tacho = &os_tacho_p->ctrl;

    return FTachoGetCaptureCnt(tacho);
}

/**
 * @name: FFreeRTOSTachoDeinit
 * @msg: deinit tacho or capture
 * @return {*}
 * @param {FFreeRTOSTimerTacho} *os_tacho_p
 */
void FFreeRTOSTachoDeinit(FFreeRTOSTimerTacho *os_tacho_p)
{
    FASSERT(NULL != os_tacho_p->locker);

    FTachoDeInit(&os_tacho_p->ctrl);
    vSemaphoreDelete(os_tacho_p->locker);
    memset(os_tacho_p, 0, sizeof(*os_tacho_p));
    return;
}

