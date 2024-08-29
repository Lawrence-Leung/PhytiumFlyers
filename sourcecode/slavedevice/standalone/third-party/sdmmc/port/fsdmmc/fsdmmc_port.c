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
 * FilePath: fsdmmc_port.c
 * Date: 2022-08-17 10:11:48
 * LastEditTime: 2022-08-17 10:11:48
 * Description:  This file is for sdmmc port
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/12/5   init commit
 */
/***************************** Include Files *********************************/
#include <string.h>

#include "fparameters.h"
#include "fassert.h"
#include "ftypes.h"
#include "fsleep.h"
#include "fcache.h"
#include "finterrupt.h"

#include "fsdmmc.h"
#include "fsdmmc_hw.h"

#include "fsdmmc_port.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
typedef struct
{
    FSdmmc ctrl;
    sdmmc_host_instance_t *instance;
    FSdmmcCmd cmd_data;
    FSdmmcData trans_data;
    u32 freq_khz;
    volatile boolean cmd_done;
    volatile boolean data_done;
    volatile boolean cmd_error;
    volatile boolean data_error;
    volatile boolean locked;
} sdmmc_host_slot_info;

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FSDMMC_DEBUG_TAG            "FSMMC-PORT"
#define FSDMMC_ERROR(format, ...)   FT_DEBUG_PRINT_E(FSDMMC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDMMC_WARN(format, ...)    FT_DEBUG_PRINT_W(FSDMMC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDMMC_INFO(format, ...)    FT_DEBUG_PRINT_I(FSDMMC_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDMMC_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FSDMMC_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/
static sdmmc_host_slot_info host_slot_info[FSDMMC_NUM];

static inline sdmmc_host_slot_info *fsdmmc_get_slot_info(int slot)
{
    return &host_slot_info[slot];
}

static inline sdmmc_host_instance_t *fsdmmc_get_instance(int slot)
{
    return host_slot_info[slot].instance;
}

static inline FSdmmc *fsdmmc_get_ctrl(int slot)
{
    return &host_slot_info[slot].ctrl;
}

sdmmc_err_t sdmmc_cmd_set_block_count(sdmmc_card_t *card, u32 block_count);
sdmmc_err_t sdmmc_send_stop_transmission(sdmmc_card_t *card);
sdmmc_err_t sdmmc_send_cmd_switch(sdmmc_card_t *card);
void sdmmc_reverse(uint32_t *response, size_t length);
/*****************************************************************************/
static void fsdmmc_host_relax(void)
{
    sdmmc_sys_delay_ms(1);
}

static void fsdmmc_host_card_remove_cb(void *para)
{
    FASSERT(para);
    sdmmc_host_slot_info *slot_info = (sdmmc_host_slot_info *)para;

    FSDMMC_INFO("Card removed !!!");
}

static void fsdmmc_host_cmd_done_cb(void *para)
{
    FASSERT(para);
    sdmmc_host_slot_info *slot_info = (sdmmc_host_slot_info *)para;

    slot_info->cmd_done = TRUE;
}

static void fsdmmc_host_cmd_error_cb(void *para)
{
    FASSERT(para);
    sdmmc_host_slot_info *slot_info = (sdmmc_host_slot_info *)para;

    slot_info->cmd_done = TRUE;
    slot_info->cmd_error = TRUE;
}

static void fsdmmc_host_data_done_cb(void *para)
{
    FASSERT(para);
    sdmmc_host_slot_info *slot_info = (sdmmc_host_slot_info *)para;

    slot_info->data_done = TRUE;
}

static void fsdmmc_host_data_error_cb(void *para)
{
    FASSERT(para);
    sdmmc_host_slot_info *slot_info = (sdmmc_host_slot_info *)para;

    slot_info->data_done = TRUE;
    slot_info->data_error = TRUE;
}

static sdmmc_err_t fsdmmc_host_setup_interrupt(int slot)
{
    FSdmmc *instance_p = fsdmmc_get_ctrl(slot);
    sdmmc_host_slot_info *slot_info = fsdmmc_get_slot_info(slot);
    FSdmmcConfig *config_p = &instance_p->config;
    uintptr base_addr = config_p->base_addr;
    u32 reg_val;

    /* disable all interrupt */
    FSdmmcSetInterruptMask(base_addr, FSDMMC_CMD_INTR, FSDMCC_NORMAL_INT_ALL_BITS, FALSE);
    FSdmmcSetInterruptMask(base_addr, FSDMMC_ERROR_INTR, FSDMMC_ERROR_INT_ALL_BITS, FALSE);
    FSdmmcSetInterruptMask(base_addr, FSDMMC_DMA_BD_INTR, FSDMMC_BD_ISR_ALL_BITS, FALSE);

    /* clear interrupt status */
    FSdmmcClearNormalInterruptStatus(base_addr);
    FSdmmcClearErrorInterruptStatus(base_addr);
    FSdmmcClearBDInterruptStatus(base_addr);

    /* register intr, attach interrupt handler */
    InterruptSetPriority(config_p->irq_num[FSDMMC_CMD_INTR], 0);
    InterruptSetPriority(config_p->irq_num[FSDMMC_ERROR_INTR], 0);
    InterruptSetPriority(config_p->irq_num[FSDMMC_DMA_BD_INTR], 0);

    InterruptInstall(config_p->irq_num[FSDMMC_CMD_INTR], FSdmmcCmdInterrupHandler, instance_p, "FSDMMC-CMD");
    InterruptInstall(config_p->irq_num[FSDMMC_ERROR_INTR], FSdmmcErrInterrupHandler, instance_p, "FSDMMC-ERR");
    InterruptInstall(config_p->irq_num[FSDMMC_DMA_BD_INTR], FSdmmcDmaInterrupHandler, instance_p, "FSDMMC-DMA");

    /* umask and enable fsdio interrupt */
    InterruptUmask(config_p->irq_num[FSDMMC_CMD_INTR]);
    InterruptUmask(config_p->irq_num[FSDMMC_ERROR_INTR]);
    InterruptUmask(config_p->irq_num[FSDMMC_DMA_BD_INTR]);

    /* enable some interrupts */
    FSdmmcSetInterruptMask(base_addr, FSDMMC_CMD_INTR, FSDMCC_NORMAL_INT_ALL_BITS, TRUE);
    FSdmmcSetInterruptMask(base_addr, FSDMMC_ERROR_INTR, FSDMMC_ERROR_INT_ALL_BITS, TRUE);
    FSdmmcSetInterruptMask(base_addr, FSDMMC_DMA_BD_INTR, FSDMMC_BD_ISR_ALL_BITS, TRUE);

    /* register interrupt event handler */
    FSdmmcRegisterInterruptHandler(instance_p, FSDMMC_EVT_CARD_REMOVED, fsdmmc_host_card_remove_cb, slot_info);
    FSdmmcRegisterInterruptHandler(instance_p, FSDMMC_EVT_CMD_DONE, fsdmmc_host_cmd_done_cb, slot_info);
    FSdmmcRegisterInterruptHandler(instance_p, FSDMMC_EVT_CMD_ERROR, fsdmmc_host_cmd_error_cb, slot_info);
    FSdmmcRegisterInterruptHandler(instance_p, FSDMMC_EVT_CMD_RESP_ERROR, fsdmmc_host_cmd_error_cb, slot_info);
    FSdmmcRegisterInterruptHandler(instance_p, FSDMMC_EVT_DATA_READ_DONE, fsdmmc_host_data_done_cb, slot_info);
    FSdmmcRegisterInterruptHandler(instance_p, FSDMMC_EVT_DATA_WRITE_DONE, fsdmmc_host_data_done_cb, slot_info);
    FSdmmcRegisterInterruptHandler(instance_p, FSDMMC_EVT_DATA_ERROR, fsdmmc_host_data_error_cb, slot_info);

    return FSDMMC_SUCCESS;
}

static sdmmc_err_t fsdmmc_host_set_bus_width(int slot, size_t width)
{
    FASSERT(slot < FSDMMC_NUM); /* no need to switch bus width */
    return SDMMC_OK;
}

static size_t fsdmmc_host_get_slot_width(int slot)
{
    FASSERT(slot < FSDMMC_NUM);
    return 4; /* bus width is fix as 4 */
}

static sdmmc_err_t fsdmmc_host_set_bus_ddr_mode(int slot, bool ddr_enabled)
{
    FASSERT(slot < FSDMMC_NUM);
    FSDMMC_ERROR("fsdmmc_host_set_bus_ddr_mode is not supported !!!");
    return SDMMC_OK;
}

static sdmmc_err_t fsdmmc_host_set_card_clk(int slot, uint32_t freq_khz)
{
    FASSERT(slot < FSDMMC_NUM);
    FSdmmc *ctrl_p = fsdmmc_get_ctrl(slot);
    sdmmc_host_slot_info *slot_info = fsdmmc_get_slot_info(slot);
    u32 ret = FSdmmcSetCardClk(ctrl_p->config.base_addr, freq_khz * 1000);
    if (FSDMMC_SUCCESS == ret)
    {
        slot_info->freq_khz = freq_khz;
        return SDMMC_OK;
    }
    else
    {
        return SDMMC_FAIL;
    }
}

static sdmmc_err_t fsdmmc_host_io_int_enable(int slot)
{
    FASSERT(slot < FSDMMC_NUM);
    FSDMMC_ERROR("fsdmmc_host_io_int_enable is not supported !!!");
    return SDMMC_OK;
}

static sdmmc_err_t fsdmmc_host_io_int_wait(int slot, tick_type_t timeout_ticks)
{
    FASSERT(slot < FSDMMC_NUM);
    FSDMMC_ERROR("fsdmmc_host_io_int_wait is not supported !!!");
    return SDMMC_OK;
}

static sdmmc_err_t fsdmmc_host_get_real_freq(int slot, int* real_freq_khz)
{
    sdmmc_host_slot_info *slot_info = fsdmmc_get_slot_info(slot); 
    *real_freq_khz = slot_info->freq_khz;
    return SDMMC_OK;
}

static void fsdmmc_host_set_cmd_flag(const sdmmc_command_t *cmdinfo, FSdmmcCmd *cmd_p, FSdmmcData *data_p)
{
    FASSERT(cmdinfo && cmd_p);
    cmd_p->flag = 0;

    if (cmdinfo->flags & SCF_CMD_ADTC)
        cmd_p->flag |= FSDMMC_CMD_FLAG_ADTC;

    if (cmdinfo->flags & SCF_RSP_PRESENT)
        cmd_p->flag |= FSDMMC_CMD_FLAG_EXP_RESP;

    if (cmdinfo->flags & SCF_RSP_136)
        cmd_p->flag |= FSDMMC_CMD_FLAG_EXP_LONG_RESP;

    if (MMC_GO_IDLE_STATE == cmdinfo->opcode)
        cmd_p->flag |= FSDMMC_CMD_FLAG_NEED_INIT;

    if (cmdinfo->flags & SCF_CMD_ADTC)
        cmd_p->flag |= FSDMMC_CMD_FLAG_ADTC;

    cmd_p->cmdidx = cmdinfo->opcode;
    cmd_p->cmdarg = cmdinfo->arg;
    if (NULL != cmdinfo->data)
    {
        /* assign read/write flag */
        cmd_p->flag |= FSDMMC_CMD_FLAG_EXP_DATA;
        if (cmdinfo->flags & SCF_CMD_READ)
            cmd_p->flag |= FSDMMC_CMD_FLAG_READ_DATA;
        else
            cmd_p->flag |= FSDMMC_CMD_FLAG_WRITE_DATA;

        /* copy addr of in/out data buffer */
        data_p->buf = cmdinfo->data;
        data_p->blksz = FSDMMC_BLOCK_SIZE;
        data_p->datalen = cmdinfo->datalen;
        if (FSDMMC_BLOCK_SIZE != data_p->blksz)
        {
            FSDMMC_WARN( "Blk size is %d, data len is %d", 
                            data_p->blksz,
                            data_p->datalen);
        }

        /* save data info */
        cmd_p->data_p = data_p;
    }

    return;
}

static void fsdmmc_host_get_cmd_resp(uintptr base_addr, FSdmmcCmd *cmd_p, sdmmc_command_t *cmdinfo)
{
	if (cmd_p->flag & FSDMMC_CMD_FLAG_EXP_RESP)
	{
		if (cmd_p->flag & FSDMMC_CMD_FLAG_EXP_LONG_RESP)
		{
            /* in FT2004/D2000, byte-order of response no need filp, but array need reverse */
			cmd_p->response[3] = FSDMMC_READ_REG(base_addr, FSDMMC_CMD_RESP_1_REG_OFFSET);
			cmd_p->response[2] = FSDMMC_READ_REG(base_addr, FSDMMC_CMD_RESP_2_REG_OFFSET);
			cmd_p->response[1] = FSDMMC_READ_REG(base_addr, FSDMMC_CMD_RESP_3_REG_OFFSET);
			cmd_p->response[0] = FSDMMC_READ_REG(base_addr, FSDMMC_CMD_RESP_4_REG_OFFSET);
        }
        else
		{
			cmd_p->response[0] = FSDMMC_READ_REG(base_addr, FSDMMC_CMD_RESP_1_REG_OFFSET);
			cmd_p->response[1] = 0;
			cmd_p->response[2] = 0;
			cmd_p->response[3] = 0;			
		}

        memcpy((u32 *)cmdinfo->response, cmd_p->response, sizeof(u32) * 4); /* copy back response buffer */
	}
}

static sdmmc_err_t fsdmmc_host_pre_command(int slot, sdmmc_command_t *cmdinfo)
{
    sdmmc_host_instance_t *instance_p = fsdmmc_get_instance(slot);
    sdmmc_host_slot_info *slot_info = fsdmmc_get_slot_info(slot);

    if (!(instance_p->config.flags & SDMMC_HOST_IO_SUPPORT)) /* filter function card only command */
    {
        FSDMMC_INFO("opcode = %d", cmdinfo->opcode);
        if (SD_IO_SEND_OP_COND == cmdinfo->opcode)
        {
            FSDMMC_INFO("Reject SD_IO_SEND_OP_COND for memory card.");
            return SDMMC_FAIL; /* just return a error code to bypass */
        }

        if (SD_IO_RW_DIRECT == cmdinfo->opcode)
        {
            FSDMMC_INFO("Reject SD_IO_RW_DIRECT for memory card.");
            return SDMMC_ERR_TIMEOUT;            
        }        
    }

    if (!(instance_p->config.flags & SDMMC_HOST_REMOVABLE_CARD)) /* filter SD memory card only command */
    {
        FSDMMC_INFO("opcode = %d", cmdinfo->opcode);
        if ((SD_SEND_IF_COND == cmdinfo->opcode) && (NULL == cmdinfo->data))
        {
            /* MMC_SEND_EXT_CSD = 8 need to send so check cmdinfo->data to distinguish 
                SD_SEND_IF_COND from MMC_SEND_EXT_CSD  */
            FSDMMC_INFO("Reject SD_SEND_IF_COND for eMMC.");
            return SDMMC_ERR_NOT_SUPPORTED;          
        }

        if (MMC_APP_CMD == cmdinfo->opcode)
        {
            FSDMMC_INFO("Reject MMC_APP_CMD for eMMC.");
            return SDMMC_ERR_TIMEOUT;
        }        
    }

    /* FT20004/D2000 may need to send CMD23 to set block count even for single-block read/write */
    if ((MMC_READ_BLOCK_MULTIPLE == cmdinfo->opcode) || 
        (MMC_WRITE_BLOCK_MULTIPLE == cmdinfo->opcode) || 
        (MMC_READ_BLOCK_SINGLE == cmdinfo->opcode) || 
        (MMC_WRITE_BLOCK_SINGLE == cmdinfo->opcode) && (cmdinfo->data))
    {
        u32 block_count = cmdinfo->datalen / cmdinfo->blklen;
        return sdmmc_cmd_set_block_count(&instance_p->card, block_count);
    }

    return SDMMC_OK;    
}

static sdmmc_err_t fsdmmc_host_post_command(int slot, sdmmc_command_t *cmdinfo)
{
    sdmmc_host_instance_t *instance_p = fsdmmc_get_instance(slot);
    sdmmc_host_slot_info *slot_info = fsdmmc_get_slot_info(slot);

    if ((cmdinfo->error) && (cmdinfo->data))
    {
        (void)sdmmc_send_stop_transmission(&(instance_p->card));
    }

    /* FT2004/D200 need switch to 4-bit bus width before first DMA in CMD51 */
    if (MMC_SELECT_CARD == cmdinfo->opcode)
    {
        (void)sdmmc_send_cmd_switch(&instance_p->card);
    }

    return cmdinfo->error;
}

static sdmmc_err_t fsdmmc_host_do_transaction_irq(int slot, sdmmc_command_t *cmdinfo)
{
    FASSERT(slot < FSDMMC_NUM);
    FSdmmc *ctrl_p = fsdmmc_get_ctrl(slot);
    sdmmc_host_slot_info *slot_info = fsdmmc_get_slot_info(slot);
    sdmmc_host_instance_t *instance = fsdmmc_get_instance(slot);
    uintptr base_addr = ctrl_p->config.base_addr;
    sdmmc_err_t ret = SDMMC_OK;
    const u32 timeout = instance->host.command_timeout_ms;
    u32 loop;

    ret = fsdmmc_host_pre_command(slot, cmdinfo);
    if (SDMMC_OK != ret)
    {
        /* if not supported (bypass), return ok, if other error (reject), return fail */
        return ret;
    }

    memset(&slot_info->cmd_data, 0 , sizeof(slot_info->cmd_data));
    memset(&slot_info->trans_data, 0, sizeof(slot_info->trans_data));

    slot_info->cmd_done = FALSE;
    slot_info->data_done = FALSE;
    slot_info->cmd_error = FALSE;
    slot_info->data_error = FALSE;

    fsdmmc_host_set_cmd_flag(cmdinfo, &slot_info->cmd_data, &slot_info->trans_data);

    if (FSDMMC_SUCCESS != FSdmmcInterruptTransfer(ctrl_p, &slot_info->cmd_data))
    {
        FSDMMC_ERROR("Sdmmc transfer data/cmd failed.");
        ret = SDMMC_FAIL;
        goto err_ret;
    }

    /* wait cmd transfer done */
    loop = 0;
    while ((FALSE == slot_info->cmd_done) && (loop++ < timeout))
    {
        fsdmmc_host_relax();
    }

    /* wait cmd timeout */
    if (loop >= timeout)
    {
        FSDMMC_ERROR("Sdmmc transfer cmd timeout.");
        ret = SDMMC_ERR_TIMEOUT;
        goto err_ret;
    }

    /* cmd transfer end with error */
    if (TRUE == slot_info->cmd_error)
    {
        FSDMMC_ERROR("Sdmmc transfer cmd failed.");
        ret = SDMMC_FAIL;
        FSdmmcSoftwareReset(base_addr, FSDMMC_TIMEOUT); 
        goto err_ret;
    }

    FSDMMC_INFO( "CMD [%d] END: 0x%x.", slot_info->cmd_data.cmdidx, slot_info->cmd_error); 

    /* wait data transfer done */
    loop = 0;
    while ((slot_info->cmd_data.flag & FSDMMC_CMD_FLAG_EXP_DATA) && 
           (FALSE == slot_info->data_done) && 
           (loop++ < timeout))
    {
        fsdmmc_host_relax();
    }

    /* wait data timeout */
    if (loop >= timeout)
    {
        FSDMMC_ERROR("Sdmmc transfer data timeout.");
        ret = SDMMC_ERR_TIMEOUT;
        goto err_ret;
    }

    /* data transfer end with error */
    if (TRUE == slot_info->data_error)
    {
        FSDMMC_ERROR("Sdmmc transfer data failed");
        ret = SDMMC_FAIL;
        FSdmmcSoftwareReset(base_addr, FSDMMC_TIMEOUT);  
        goto err_ret;
    }

    fsdmmc_host_get_cmd_resp(base_addr, &slot_info->cmd_data, cmdinfo);

    if (slot_info->cmd_data.flag & FSDMMC_CMD_FLAG_EXP_DATA)
    {
        FSDMMC_INFO( "DATA [%d] END 0x%x.", slot_info->cmd_data.cmdidx, slot_info->data_error); 
        FCacheDCacheInvalidateRange((uintptr)slot_info->trans_data.buf, slot_info->trans_data.datalen);
    }

err_ret:
    if ((TRUE == slot_info->data_error) || (TRUE == slot_info->cmd_error))
    {
        cmdinfo->error = SDMMC_FAIL;
        ret = fsdmmc_host_post_command(slot, cmdinfo);
    }
    else
    {
        cmdinfo->error = SDMMC_OK;
        ret = fsdmmc_host_post_command(slot, cmdinfo);        
    }

    return ret;
}


static sdmmc_err_t fsdmmc_host_do_transaction_poll(int slot, sdmmc_command_t *cmdinfo)
{
    FASSERT(slot < FSDMMC_NUM);
    FSdmmc *ctrl_p = fsdmmc_get_ctrl(slot);
    sdmmc_host_slot_info *slot_info = fsdmmc_get_slot_info(slot);
    sdmmc_host_instance_t *instance = fsdmmc_get_instance(slot);
    uintptr base_addr = ctrl_p->config.base_addr;
    sdmmc_err_t ret = SDMMC_OK;

    ret = fsdmmc_host_pre_command(slot, cmdinfo);
    if (SDMMC_OK != ret)
    {
        /* if not supported (bypass), return ok, if other error (reject), return fail */
        return ret;
    }

    memset(&slot_info->cmd_data, 0 , sizeof(slot_info->cmd_data));
    memset(&slot_info->trans_data, 0, sizeof(slot_info->trans_data));

    fsdmmc_host_set_cmd_flag(cmdinfo, &slot_info->cmd_data, &slot_info->trans_data);

    if (FSDMMC_SUCCESS == FSdmmcPollTransfer(ctrl_p, &slot_info->cmd_data))
    {
        fsdmmc_host_get_cmd_resp(base_addr, &slot_info->cmd_data, cmdinfo);

        cmdinfo->error = SDMMC_OK;
        ret = fsdmmc_host_post_command(slot, cmdinfo);      
    }
    else
    {
        cmdinfo->error = SDMMC_FAIL;
        ret = fsdmmc_host_post_command(slot, cmdinfo);
    }

    return ret;
}

static sdmmc_err_t fsdmmc_ctrl_init(int slot_id)
{
    sdmmc_err_t ret = SDMMC_OK;
    FSdmmc *ctrl_p = fsdmmc_get_ctrl(slot_id);
    sdmmc_host_instance_t *instance = fsdmmc_get_instance(slot_id);
    FSdmmcConfig ctrl_config;
    const FSdmmcConfig *def_ctrl_config;

    memset(&ctrl_config, 0, sizeof(ctrl_config));

    SDMMC_ASSERT((def_ctrl_config = FSdmmcLookupConfig(slot_id)) != NULL);
    ctrl_config = *def_ctrl_config;

    if (!(instance->config.flags & SDMMC_HOST_WORK_MODE_DMA) || 
        !(instance->config.flags & SDMMC_HOST_REMOVABLE_CARD))
    {
        FSDMMC_ERROR("NO-DMA or eMMC do not support !!!");
        ret = SDMMC_FAIL;
        goto err_exit;         
    }

    /* init sdmmc ctrl */
    memset(ctrl_p, 0, sizeof(*ctrl_p));
    if (FSDMMC_SUCCESS != FSdmmcCfgInitialize(ctrl_p, &ctrl_config))
    {
        FSDMMC_ERROR("Sdmmc ctrl init failed.");
        ret = SDMMC_FAIL;
        goto err_exit;        
    }

    /* init sdmmc interrupt */
    if (instance->config.flags & SDMMC_HOST_WORK_MODE_IRQ)
    {
        ret = fsdmmc_host_setup_interrupt(slot_id);
        if (SDMMC_OK != ret)
        {
            FSDMMC_ERROR("Sdmmc ctrl init interrupt failed.");
            goto err_exit;        
        }        
    }

err_exit:
    return ret;
}

static sdmmc_err_t fsdmmc_card_init(int slot_id)
{
    sdmmc_err_t ret = SDMMC_OK;
    FSdmmc *ctrl_p = fsdmmc_get_ctrl(slot_id);
    sdmmc_host_instance_t *instance = fsdmmc_get_instance(slot_id);
    sdmmc_host_t *host_p = &instance->host;
    sdmmc_card_t *card_p = &instance->card;

    /* setup probing freq 400kHz */
    uintptr base_addr = ctrl_p->config.base_addr;
    if (FSDMMC_SUCCESS != FSdmmcSetCardClk(base_addr, SDMMC_FREQ_PROBING * 1000))
    {
        FSDMMC_ERROR("Sdmmc ctrl set probe clk failed.");
        return SDMMC_FAIL;        
    }

    /* init card */
    memset(card_p, 0, sizeof(*card_p));
    ret = sdmmc_card_init(&instance->host, card_p);
    if (SDMMC_OK != ret)
    {
        FSDMMC_ERROR("Card init failed: 0x%x.", ret);
        return ret;
    }

    /* detect card existence */
    if (FALSE == FSdmmcCheckIfCardExists(base_addr))
    {
        FSDMMC_ERROR("Card not exist.");
        return SDMMC_FAIL;
    }

    sdmmc_card_print_info(card_p);
    return SDMMC_OK;
}

sdmmc_err_t fsdmmc_host_init(sdmmc_host_instance_t *const instance, const sdmmc_host_config_t* config)
{
    SDMMC_ASSERT(instance && config);
    SDMMC_ASSERT(config->slot < FSDMMC_NUM);
    int slot_id = config->slot;
    sdmmc_host_t *host_p = &instance->host;
    sdmmc_card_t *card_p = &instance->card;
    FSdmmc *ctrl_p = fsdmmc_get_ctrl(slot_id);
    sdmmc_err_t ret = SDMMC_OK;    

    memset(host_p, 0, sizeof(*host_p));
    memset(card_p, 0, sizeof(*card_p));

    instance->private = &host_slot_info[slot_id];
    host_slot_info[slot_id].instance = instance;

    if (&(instance->config) != config)
        instance->config = *config;
    
    FSDMMC_INFO("instance->config.flags = 0x%x", instance->config.flags);

    /* config sdmmc host */    
    host_p->flags = SDMMC_HOST_FLAG_4BIT; /* only support 1-bit bus width */
    host_p->flags &= ~SDMMC_HOST_FLAG_DDR;
    host_p->flags &= ~SDMMC_HOST_FLAG_SPI;
    host_p->slot = slot_id;
    if (config->bus_speed_mode == SDMMC_SDR_12) /* SDR12 max freqency is 25MHz */
    {
        host_p->max_freq_khz = SDMMC_FREQ_DEFAULT;
        host_p->io_voltage = 1.8f;
    }
    else if (config->bus_speed_mode == SDMMC_SDR_25) /* SDR25 max freqency is 50MHz */
    {
        host_p->max_freq_khz = SDMMC_FREQ_50M;
        host_p->io_voltage = 1.8f;
    }

    host_p->command_timeout_ms = 5000; /* 5000ms */

    /* register sdmmc host ops */
    host_p->set_bus_width = fsdmmc_host_set_bus_width;
    host_p->get_bus_width = fsdmmc_host_get_slot_width;
    host_p->set_bus_ddr_mode = fsdmmc_host_set_bus_ddr_mode;
    host_p->set_card_clk = fsdmmc_host_set_card_clk;
    host_p->get_real_freq = fsdmmc_host_get_real_freq;

    if (instance->config.flags & SDMMC_HOST_WORK_MODE_IRQ)
        host_p->do_transaction = fsdmmc_host_do_transaction_irq;
    else
        host_p->do_transaction = fsdmmc_host_do_transaction_poll;    

    host_p->io_int_enable = fsdmmc_host_io_int_enable;
    host_p->io_int_wait = fsdmmc_host_io_int_wait; 

    ret = fsdmmc_ctrl_init(slot_id);
    if (SDMMC_OK != ret)
    {
        return ret;
    }

    ret = fsdmmc_card_init(slot_id);
    if (SDMMC_OK != ret)
    {
        return ret;
    }

    instance->is_ready = FT_COMPONENT_IS_READY;
    return ret;
}

sdmmc_err_t fsdmmc_host_deinit(sdmmc_host_instance_t *const instance)
{
    sdmmc_err_t ret = SDMMC_OK;

    return ret;     
}

sdmmc_err_t fsdmmc_host_lock(sdmmc_host_t *const host)
{
    int slot_id = host->slot;
    sdmmc_host_slot_info *slot_info = fsdmmc_get_slot_info(slot_id);

    while (slot_info->locked == TRUE) /* wait until other task/thread unlock */
    {
        fsdmmc_host_relax();
    }
    
    slot_info->locked = TRUE;
}

void fsdmmc_host_unlock(sdmmc_host_t *const host)
{
    int slot_id = host->slot;
    sdmmc_host_slot_info *slot_info = fsdmmc_get_slot_info(slot_id);

    FASSERT(TRUE == slot_info->locked);
    slot_info->locked = FALSE;
}