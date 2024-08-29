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
 * FilePath: fwdt_os.c
 * Date: 2022-07-14 13:42:19
 * LastEditTime: 2022-07-25 16:59:51
 * Description:  This file is for required function implementations of wdt driver used in FreeRTOS.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2022/08/09  first commit
 */
#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include "task.h"
#include "ftypes.h"
#include "fassert.h"
#include "fdebug.h"
#include "fwdt_os.h"
#include "fwdt.h"
#include "finterrupt.h"
#include "fwdt_hw.h"


#define FWDT_DEBUG_TAG "FFreeRTOSWdt"
#define FWDT_ERROR(format, ...)   FT_DEBUG_PRINT_E(FWDT_DEBUG_TAG, format, ##__VA_ARGS__)
#define FWDT_WARN(format, ...)   FT_DEBUG_PRINT_W(FWDT_DEBUG_TAG, format, ##__VA_ARGS__)
#define FWDT_INFO(format, ...) FT_DEBUG_PRINT_I(FWDT_DEBUG_TAG, format, ##__VA_ARGS__)
#define FWDT_DEBUG(format, ...) FT_DEBUG_PRINT_D(FWDT_DEBUG_TAG, format, ##__VA_ARGS__)

static FFreeRTOSWdt os_wdt[FWDT_NUM] = {0};

/**
 * @name: FFreeRTOSWdtInit
 * @msg:  init freeRTOS wdt instance, include init wdt and create mutex
 * @param {u32} instance_id, wdt instance id
 * @return {FFreeRTOSWdt *} pointer to os wdt instance
 */
FFreeRTOSWdt *FFreeRTOSWdtInit(u32 instance_id)
{
    FASSERT(instance_id < FWDT_NUM);
    FASSERT(FT_COMPONENT_IS_READY != os_wdt[instance_id].wdt_ctrl.is_ready);

    FWdtConfig pconfig;
    pconfig = *FWdtLookupConfig(instance_id);
    pconfig.irq_prority = FREERTOS_WDT_IRQ_PRIORITY;

    FASSERT(FWdtCfgInitialize(&os_wdt[instance_id].wdt_ctrl, &pconfig) == FT_SUCCESS);
    FASSERT((os_wdt[instance_id].wdt_semaphore = xSemaphoreCreateMutex()) != NULL);

    return (&os_wdt[instance_id]);
}

/**
 * @name: FFreeRTOSWdtDeinit
 * @msg:  deinit freeRTOS wdt instance, include stop wdt, deinit wdt and delete mutex
 * @param {FFreeRTOSWdt} *os_wdt_p, pointer to os wdt instance
 * @return err code information, FWDT_SUCCESS indicates success，others indicates failed
 */
FError FFreeRTOSWdtDeinit(FFreeRTOSWdt *os_wdt_p)
{
    FASSERT(os_wdt_p);
    FASSERT(os_wdt_p->wdt_semaphore != NULL);

    FWdtDeInitialize(&os_wdt_p->wdt_ctrl);
    vSemaphoreDelete(os_wdt_p->wdt_semaphore);
    memset(os_wdt_p, 0, sizeof(*os_wdt_p));

    return FWDT_SUCCESS;
}

/**
 * @name: FFreeRTOSWdtControl
 * @msg:  control freeRTOS wdt instance
 * @param {FFreeRTOSWdt} *os_wdt_p, pointer to os wdt instance
 * @param {int} cmd, control cmd
 * @param {void} *args, pointer to control cmd arguments
 * @return err code information, FWDT_SUCCESS indicates success，others indicates failed
 */
FError FFreeRTOSWdtControl(FFreeRTOSWdt *os_wdt_p, int cmd, void *args)
{
    FASSERT(os_wdt_p);
    FASSERT(os_wdt_p->wdt_semaphore != NULL);
    FError ret = FWDT_SUCCESS;

    FWdtCtrl *pctrl = &os_wdt_p->wdt_ctrl;
    FWdtConfig *pconfig = &os_wdt_p->wdt_ctrl.config;

    /* New contrl can be performed only after current one is finished */
    if (pdFALSE == xSemaphoreTake(os_wdt_p->wdt_semaphore, portMAX_DELAY))
    {
        FWDT_ERROR("Wdt xSemaphoreTake failed.");
        /* We could not take the semaphore, exit with 0 data received */
        return FREERTOS_WDT_SEM_ERROR;
    }

    switch (cmd)
    {
        case FREERTOS_WDT_CTRL_GET_TIMEOUT:
            *((u32 *)args) = os_wdt_p->timeout_value;
            break;

        case FREERTOS_WDT_CTRL_SET_TIMEOUT:
            os_wdt_p->timeout_value = *((u32 *)args);
            if (os_wdt_p->timeout_value >= FWDT_MAX_TIMEOUT)
            {
                goto control_exit;
            }
            ret = FWdtSetTimeout(pctrl, os_wdt_p->timeout_value);
            if (FWDT_SUCCESS != ret)
            {
                FWDT_ERROR("FFreeRTOSWdtControl FWdtSetTimeout failed.");
                goto control_exit;
            }
            break;

        case FREERTOS_WDT_CTRL_GET_TIMELEFT:
            *((u32 *)args) = FWdtGetTimeleft(pctrl);
            break;

        case FREERTOS_WDT_CTRL_KEEPALIVE:
            ret = FWdtRefresh(pctrl);
            if (FWDT_SUCCESS != ret)
            {
                FWDT_ERROR("FFreeRTOSWdtControl FWdtRefresh failed.");
                goto control_exit;
            }
            break;

        case FREERTOS_WDT_CTRL_START:
            ret = FWdtStart(pctrl);
            if (FWDT_SUCCESS != ret)
            {
                FWDT_ERROR("FFreeRTOSWdtControl FWdtStart failed.");
                goto control_exit;
            }
            break;

        case FREERTOS_WDT_CTRL_STOP:
            ret = FWdtStop(pctrl);
            if (FWDT_SUCCESS != ret)
            {
                FWDT_ERROR("FFreeRTOSWdtControl FWdtStop failed.");
                goto control_exit;
            }
            break;

        default:
            FWDT_ERROR("Control cmd is invalid.");
            break;
    }

control_exit:

    /* Enable next contrl. Current one is finished */
    if (pdFALSE == xSemaphoreGive(os_wdt_p->wdt_semaphore))
    {
        /* We could not post the semaphore, exit with error */
        FWDT_ERROR("Wdt xSemaphoreGive failed.");
        return FREERTOS_WDT_SEM_ERROR;
    }

    return ret;
}