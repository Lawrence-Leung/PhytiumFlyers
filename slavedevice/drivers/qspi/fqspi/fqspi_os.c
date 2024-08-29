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
 * FilePath: fqspi_os.c
 * Date: 2022-02-24 13:42:19
 * LastEditTime: 2022-03-21 16:59:51
 * Description:  This file is for required function implementations of qspi driver used in FreeRTOS.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2022/08/09  first commit
 * 1.1 wangxiaodong 2022/11/09  qspi sfud perfection
 */
#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include "task.h"
#include "ftypes.h"
#include "fdebug.h"
#include "fassert.h"
#include "fqspi_os.h"
#include "fqspi.h"
#include "fqspi_hw.h"
#include "fqspi_flash.h"

#define FQSPI_DEBUG_TAG "FFreeRTOSQspi"
#define FQSPI_ERROR(format, ...)   FT_DEBUG_PRINT_E(FQSPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FQSPI_WARN(format, ...)   FT_DEBUG_PRINT_W(FQSPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FQSPI_INFO(format, ...) FT_DEBUG_PRINT_I(FQSPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FQSPI_DEBUG(format, ...) FT_DEBUG_PRINT_D(FQSPI_DEBUG_TAG, format, ##__VA_ARGS__)

static FFreeRTOSQspi os_qspi[FQSPI_NUM] = {0};

/**
 * @name: FFreeRTOSQspiInit
 * @msg:  init freertos qspi instance, include init qspi and create mutex
 * @param {u32} instance_id, qspi instance id, such as FQSPI0_ID
 * @return {FFreeRTOSQspi *} pointer to os qspi instance
 */
FFreeRTOSQspi *FFreeRTOSQspiInit(u32 instance_id)
{
    FASSERT(instance_id < FQSPI_NUM);
    FASSERT(FT_COMPONENT_IS_READY != os_qspi[instance_id].qspi_ctrl.is_ready);

    /* qspi initialize */
    FQspiConfig qspi_config;
    qspi_config = *FQspiLookupConfig(instance_id);
    FASSERT(FQspiCfgInitialize(&os_qspi[instance_id].qspi_ctrl, &qspi_config) == FT_SUCCESS);
    /* detect connected flash infomation */
    FASSERT_MSG(FQspiFlashDetect(&os_qspi[instance_id].qspi_ctrl) == FT_SUCCESS, "Flash detect failed.");

    /* qspi wr_semaphore initialize */
    FASSERT((os_qspi[instance_id].wr_semaphore = xSemaphoreCreateMutex()) != NULL);

    return (&os_qspi[instance_id]);
}

/**
 * @name: FFreeRTOSQspiDeinit
 * @msg:  deinit freertos qspi instance, include deinit qspi and delete mutex
 * @param {FFreeRTOSQspi} *os_qspi_p, pointer to os qspi instance
 * @return void
 */
void FFreeRTOSQspiDeinit(FFreeRTOSQspi *os_qspi_p)
{
    FASSERT(os_qspi_p);
    FASSERT(os_qspi_p->wr_semaphore != NULL);
    FQspiDeInitialize(&os_qspi_p->qspi_ctrl);
    vSemaphoreDelete(os_qspi_p->wr_semaphore);
    memset(os_qspi_p, 0, sizeof(*os_qspi_p));
}

/**
 * @name: FFreeRTOSQspiTransfer
 * @msg:  tranfer qspi mesage, include read and write data, read flash id oparetion.
 * @param {FFreeRTOSQspi} *os_qspi_p, pointer to os qspi instance
 * @param {FFreeRTOSQspiMessage} *message, qspi transfer message
 * @return err code information, FQSPI_SUCCESS indicates success，others indicates failed
 */
FError FFreeRTOSQspiTransfer(FFreeRTOSQspi *os_qspi_p, FFreeRTOSQspiMessage *message)
{
    FASSERT(os_qspi_p);
    FASSERT(os_qspi_p->wr_semaphore != NULL);
    FError ret = FQSPI_SUCCESS;
    FQspiCtrl *pctrl = &os_qspi_p->qspi_ctrl;

    const u8 *write_buf = message->write_buf;
    u8 *read_buf = message->read_buf;
    u32 flash_addr = message->addr;
    size_t length = message->length;
    u8 cmd = message->cmd;
    u8 cs = message->cs;
    FASSERT(cs < FQSPI_CS_NUM);

    static u8 cs_bak = 0;
    static u8 read_cmd_bak = 0;
    size_t read_len = 0;

    /* New transfer can be performed only after current one is finished */
    if (pdFALSE == xSemaphoreTake(os_qspi_p->wr_semaphore, portMAX_DELAY))
    {
        FQSPI_ERROR("Qspi xSemaphoreTake failed.");
        /* We could not take the semaphore, exit with 0 data received */
        return FREERTOS_QSPI_SEM_ERROR;
    }
    /* set qspi cs number if the value is changed */
    if (cs != cs_bak)
    {
        FQspiChannelSet(pctrl, cs);
        cs_bak = cs;
    }
    switch (cmd)
    {
        case FQSPI_FLASH_CMD_PP:
        {
            if (NULL != write_buf)
            {
                /* write norflash data */
                ret = FQspiFlashWriteData(pctrl, cmd, flash_addr, write_buf, length);
                if (FQSPI_SUCCESS != ret)
                {
                    FQSPI_ERROR("Qspi failed to write mem, result 0x%x", ret);
                    goto transfer_exit;
                }
            }
            else
            {
                FQSPI_ERROR("Qspi Transfer cmd %x write_buf is null.", cmd);
                ret = FQSPI_INVAL_PARAM;
                goto transfer_exit;
            }
        }
        break;

        case FQSPI_FLASH_CMD_READ:
        case FQSPI_FLASH_CMD_DUAL_READ:
        case FQSPI_FLASH_CMD_QIOR:
        case FQSPI_FLASH_CMD_4QIOR:
        {
            if (NULL != read_buf)
            {
                /* read norflash data */
                if (cmd != read_cmd_bak)
                {
                    ret |= FQspiFlashReadDataConfig(pctrl, cmd);
                    read_cmd_bak = cmd;
                    if (FQSPI_SUCCESS != ret)
                    {
                        FQSPI_ERROR("Qspi read config failed.");
                        goto transfer_exit;
                    }
                }

                read_len = FQspiFlashReadData(pctrl, flash_addr, read_buf, length);
                if (read_len != length)
                {
                    FQSPI_ERROR("Qspi failed to read mem, read len = %d", read_len);
                    ret = FQSPI_NOT_SUPPORT;
                    goto transfer_exit;
                }

            }
            else
            {
                FQSPI_ERROR("Qspi Transfer cmd %x read_buf is null.", cmd);
                ret = FQSPI_INVAL_PARAM;
                goto transfer_exit;
            }
        }
        break;

        case FQSPI_FLASH_CMD_RDID:
        case FQSPI_FLASH_CMD_RDSR1:
            ret = FQspiFlashSpecialInstruction(pctrl, cmd, read_buf, length);
            break;

        case FQSPI_FLASH_CMD_SFDP:
            ret = FQspiFlashReadSfdp(pctrl, flash_addr, read_buf, length);
            break;

        case FQSPI_CMD_ENABLE_RESET:
        case FQSPI_CMD_RESET:
        case FQSPI_FLASH_CMD_WRR:
            ret = FQspiFlashWriteReg(pctrl, cmd, NULL, 0);
            break;

        case FQSPI_FLASH_CMD_WREN:
            ret = FQspiFlashEnableWrite(pctrl);
            break;

        case FQSPI_FLASH_CMD_WRDI:
            ret = FQspiFlashDisableWrite(pctrl);
            break;

        case FQSPI_FLASH_CMD_SE:
        case FQSPI_FLASH_CMD_4SE:
        case FQSPI_FLASH_CMD_4BE:
        case FQSPI_FLASH_CMD_P4E:
            ret = FQspiFlashErase(pctrl, cmd, flash_addr);
            break;

        default:
            FQSPI_ERROR("Qspi Transfer cmd invalid.");
            ret = FQSPI_INVAL_PARAM;
            goto transfer_exit;

    }

transfer_exit:
    /* Enable next transfer. Current one is finished */
    if (pdFALSE == xSemaphoreGive(os_qspi_p->wr_semaphore))
    {
        /* We could not post the semaphore, exit with error */
        FQSPI_ERROR("Qspi xSemaphoreGive failed.");
        ret |= FREERTOS_QSPI_SEM_ERROR;
    }

    return ret;
}