
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
 * FilePath: sdmmc_host.c
 * Date: 2022-02-10 14:53:44
 * LastEditTime: 2022-02-25 11:46:22
 * Description:  This file is for sdmmc function implmentation
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/12/5   init commit
 */

#include "sdkconfig.h"
#include "sdmmc_host.h"

#ifdef CONFIG_SDMMC_USE_FSDIO
#include "fsdio_port.h"
#endif

#ifdef CONFIG_SDMMC_USE_FSDMMC
#include "fsdmmc_port.h"
#endif

sdmmc_err_t sdmmc_host_init(sdmmc_host_instance_t *const instance, const sdmmc_host_config_t* config)
{
    sdmmc_err_t ret = SDMMC_OK;
    SDMMC_ASSERT(config);
    if (SDMMC_HOST_TYPE_FSDIO == config->type)
    {
#if defined(CONFIG_SDMMC_USE_FSDIO)
        ret = fsdio_host_init(instance, config);       
#endif
    }
    else if (SDMMC_HOST_TYPE_FSDMMC == config->type)
    {
#if defined(CONFIG_SDMMC_USE_FSDMMC)
        ret = fsdmmc_host_init(instance, config);
#endif
    }
    else
    {
        return SDMMC_FAIL;
    }

    if (SDMMC_OK == ret)
    {
        instance->host.type = config->type;
    }
    else
    {
        ret = SDMMC_FAIL;
    }

    return ret;    
}

sdmmc_err_t sdmmc_host_deinit(sdmmc_host_instance_t *const instance)
{
    sdmmc_err_t ret = SDMMC_OK;
    SDMMC_ASSERT(instance);
    if (SDMMC_HOST_TYPE_FSDIO == instance->config.type)
    {
#if defined(CONFIG_SDMMC_USE_FSDIO)
        ret = fsdio_host_deinit(instance);       
#endif
    }
    else if (SDMMC_HOST_TYPE_FSDMMC == instance->config.type)
    {
#if defined(CONFIG_SDMMC_USE_FSDMMC)
        ret = fsdmmc_host_deinit(instance);
#endif
    }

    return ret;    
}

sdmmc_err_t sdmmc_host_lock(sdmmc_host_t *const host)
{
    SDMMC_ASSERT(host);
    if (SDMMC_HOST_TYPE_FSDIO == host->type)
    {
#if defined(CONFIG_SDMMC_USE_FSDIO)
        return fsdio_host_lock(host);
#endif
    }
    else if (SDMMC_HOST_TYPE_FSDMMC == host->type)
    {
#if defined(CONFIG_SDMMC_USE_FSDMMC)
        return fsdmmc_host_lock(host);
#endif
    }
    else
    {
        return SDMMC_FAIL;
    }

    return SDMMC_OK;     
}

void sdmmc_host_unlock(sdmmc_host_t *const host)
{
    SDMMC_ASSERT(host);
    if (SDMMC_HOST_TYPE_FSDIO == host->type)
    {
#if defined(CONFIG_SDMMC_USE_FSDIO)
        fsdio_host_unlock(host);
#endif
    }
    else if (SDMMC_HOST_TYPE_FSDMMC == host->type)
    {
#if defined(CONFIG_SDMMC_USE_FSDMMC)
        fsdmmc_host_unlock(host);
#endif
    }
    else
    {
        return;
    }

    return; 
}

sdmmc_err_t sdmmc_cmd_set_block_count(sdmmc_card_t *card, u32 block_count)
{
    sdmmc_command_t cmd = {
        .opcode = MMC_SET_BLOCK_COUNT,
        .arg = block_count & 0xffff,
        .flags = SCF_CMD_AC | SCF_RSP_R1};
    return sdmmc_send_cmd(card, &cmd);
}

sdmmc_err_t sdmmc_send_stop_transmission(sdmmc_card_t *card)
{
    sdmmc_err_t err;
    sdmmc_command_t cmd = {
        .opcode = MMC_STOP_TRANSMISSION,
        .arg = 0x0,
        .flags = SCF_RSP_R1B
    };
    err = sdmmc_send_app_cmd(card, &cmd);
    return err;
}

sdmmc_err_t sdmmc_send_cmd_switch(sdmmc_card_t *card)
{
    size_t datalen = 64;
    sdmmc_err_t err;
	
    uint32_t *buf = (uint32_t *)sdmmc_sys_heap_caps_malloc(datalen, MALLOC_CAP_DMA);
    if (NULL == buf)
    {
        return SDMMC_ERR_NO_MEM;
    }
    sdmmc_command_t cmd = {
        .opcode = MMC_SWITCH,
        .arg = 0x2,
        .flags = SCF_CMD_ADTC | SCF_RSP_R1
    };

    err = sdmmc_send_app_cmd(card, &cmd);
    sdmmc_sys_free(buf);
    return err;
}