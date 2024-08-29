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
 * FilePath: fsdio_port.h
 * Date: 2022-02-10 14:53:44
 * LastEditTime: 2022-02-25 11:46:22
 * Description:  This file is for sdio port
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/12/5   init commit
 */

#include <string.h>

#include "fparameters.h"
#include "fassert.h"
#include "fcache.h"
#include "finterrupt.h"
#include "fdebug.h"
#include "fcpu_info.h"

#include "fsdio.h"
#include "fsdio_hw.h"

#include "fsdio_port.h"

/************************** Constant Definitions *****************************/
#define SDIO_MAX_BLK_TRANS 100U

/**************************** Type Definitions *******************************/
typedef struct
{
    FSdio ctrl;
    sdmmc_host_instance_t *instance;
    FSdioCmdData cmd_pkg;
    FSdioData dat_pkg;
    volatile FSdioIDmaDesc *rw_desc;
    volatile boolean cmd_done;
    volatile boolean data_done;
    volatile boolean err_occur;
    volatile boolean locked;
} sdmmc_host_slot_info;

/************************** Variable Definitions *****************************/
static sdmmc_host_slot_info host_slot_info[FSDIO_NUM];

/***************** Macros (Inline Functions) Definitions *********************/
#define FSDIO_DEBUG_TAG "FSDIO-HOST"
#define FSDIO_ERROR(format, ...)   FT_DEBUG_PRINT_E(FSDIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDIO_WARN(format, ...)    FT_DEBUG_PRINT_W(FSDIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDIO_INFO(format, ...)    FT_DEBUG_PRINT_I(FSDIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDIO_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FSDIO_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/
static inline sdmmc_host_slot_info *fsdio_get_slot_info(int slot)
{
    return &host_slot_info[slot];
}

static inline sdmmc_host_instance_t *fsdio_get_instance(int slot)
{
    return host_slot_info[slot].instance;
}

static inline FSdio *fsdio_get_ctrl(int slot)
{
    return &host_slot_info[slot].ctrl;
}

sdmmc_err_t sdmmc_cmd_set_block_count(sdmmc_card_t *card, u32 block_count);
sdmmc_err_t sdmmc_send_stop_transmission(sdmmc_card_t *card);
/*****************************************************************************/
static void fsdio_host_relax(void)
{
    sdmmc_sys_delay_ms(1);
}

static void fsdio_setup_interrupt(int slot)
{
    FSdio *ctrl_p = fsdio_get_ctrl(slot);
    uintptr base_addr = ctrl_p->config.base_addr;
    u32 cpu_id = 0;

    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(ctrl_p->config.irq_num, cpu_id);
    InterruptSetPriority(ctrl_p->config.irq_num, 0);

    /* register intr callback */
    InterruptInstall(ctrl_p->config.irq_num, 
                     FSdioInterruptHandler, 
                     ctrl_p, 
                     NULL);

    /* enable sdio irq */
    InterruptUmask(ctrl_p->config.irq_num);

    FSDIO_INFO("Sdio interrupt setup done !!!");
    return;
}

static void fsdio_card_detected(FSdio *const instance_p, void *args, u32 status, u32 dmac_status)
{
    FASSERT(instance_p);
    int slot_id = (int)instance_p->config.instance_id;
    sdmmc_host_slot_info *slot_info = fsdio_get_slot_info(slot_id);

    printf("Card-%d detected.\r\n", slot_id);
}

static void fsdio_card_cmd_done(FSdio *const instance_p, void *args, u32 status, u32 dmac_status)
{
    FASSERT(instance_p);
    int slot_id = (int)instance_p->config.instance_id;
    sdmmc_host_slot_info *slot_info = fsdio_get_slot_info(slot_id);
    FSDIO_INFO("Sdio cmd done.");

    slot_info->cmd_done = TRUE;
}

static void fsdio_card_data_done(FSdio *const instance_p, void *args, u32 status, u32 dmac_status)
{
    FASSERT(instance_p);
    int slot_id = (int)instance_p->config.instance_id;
    sdmmc_host_slot_info *slot_info = fsdio_get_slot_info(slot_id);
    FSDIO_INFO("Sdio data done .");

    u32 check_status = status & (FSDIO_INT_DTO_BIT | FSDIO_INT_RCRC_BIT |
					    FSDIO_INT_DCRC_BIT | FSDIO_INT_RE_BIT |
					    FSDIO_INT_DRTO_BIT | FSDIO_INT_EBE_BIT |
					    FSDIO_INT_SBE_BCI_BIT | FSDIO_INT_RTO_BIT);

	u32	check_dmac =  dmac_status & (FSDIO_DMAC_STATUS_AIS | FSDIO_DMAC_STATUS_DU);

    if (NULL == slot_info->cmd_pkg.data_p) /* no need to wait data done */
    {
        slot_info->data_done = TRUE;
    }
    else if (check_status | check_dmac)
    {
        if (check_status & FSDIO_INT_DTO_BIT)
        {
            slot_info->data_done = TRUE;
        }
        else
        {
            FSDIO_ERROR("Xfer data error, status: 0x%x, dmac status: 0x%x", 
                        check_status, check_dmac);
        }
    }
}

static void fsdio_card_error_occur(FSdio *const instance_p, void *args, u32 status, u32 dmac_status)
{
    FASSERT(instance_p);
    int slot_id = (int)instance_p->config.instance_id;
    sdmmc_host_slot_info *slot_info = fsdio_get_slot_info(slot_id);
    FSDIO_ERROR("Sdio error occur ...");
    FSDIO_ERROR("Status: 0x%x, dmac status: 0x%x.", status, dmac_status);

    if (status & FSDIO_INT_RE_BIT)
        FSDIO_ERROR("Response err.");

    if (dmac_status & FSDIO_DMAC_STATUS_DU)
        FSDIO_ERROR("Descriptor un-readable.");

    slot_info->cmd_done = TRUE;
    slot_info->err_occur = TRUE;
}

static sdmmc_err_t fsdio_host_set_bus_width(int slot, size_t width)
{
    FASSERT((slot >= 0) && (slot < FSDIO_NUM));
    FSdio *ctrl_p = fsdio_get_ctrl(slot);
    uintptr base_addr = ctrl_p->config.base_addr;
    FSDIO_INFO("Set bus width as %d.", width);
    FSdioSetBusWidth(base_addr, width);
    return SDMMC_OK;
}

static size_t fsdio_host_get_slot_width(int slot)
{
    FASSERT((slot >= 0) && (slot < FSDIO_NUM));
    return 4U; /* host support 4 bus width */
}

static sdmmc_err_t fsdio_host_set_bus_ddr_mode(int slot, bool ddr_enabled)
{
    FASSERT((slot >= 0) && (slot < FSDIO_NUM));
    FSDIO_ERROR("sdio_host_set_bus_ddr_mode not supported !!!");
    return SDMMC_OK;
}

static sdmmc_err_t fsdio_host_set_card_clk(int slot, uint32_t freq_khz)
{
    FASSERT((slot >= 0) && (slot < FSDIO_NUM));
    FSdio *ctrl_p = fsdio_get_ctrl(slot);
    sdmmc_host_slot_info *slot_info = fsdio_get_slot_info(slot);   
    FError err = FSdioSetClkFreq(ctrl_p, freq_khz * 1000);

    if (FSDIO_SUCCESS == err)
    {
        FSDIO_INFO("Set clk rate as %dKHz", freq_khz);
        return SDMMC_OK;
    }
    else
    {
        return SDMMC_FAIL;
    }
}

static sdmmc_err_t fsdio_host_io_int_enable(int slot)
{
    FASSERT((slot >= 0) && (slot < FSDIO_NUM));
    FSDIO_ERROR("sdio_host_io_int_enable not supported !!!");
    return SDMMC_OK;
}

static sdmmc_err_t fsdio_host_io_int_wait(int slot, tick_type_t timeout_ticks)
{
    FASSERT((slot >= 0) && (slot < FSDIO_NUM));
    FSDIO_ERROR("sdio_host_io_int_wait not supported !!!");
    return SDMMC_OK;
}

static sdmmc_err_t fsdio_host_get_real_freq(int slot, int* real_freq_khz)
{
    sdmmc_host_slot_info *slot_info = fsdio_get_slot_info(slot); 
    *real_freq_khz = FSdioGetClkFreq(&(slot_info->ctrl)) / 1000;
    return SDMMC_OK;
}

static void fsdio_convert_cmdinfo(sdmmc_command_t *cmdinfo, FSdioCmdData *const cmd_data)
{
    if (MMC_GO_IDLE_STATE == cmdinfo->opcode)
    {
        cmd_data->flag |= FSDIO_CMD_FLAG_NEED_INIT;
    }

    if (SCF_RSP_CRC & cmdinfo->flags)
    {
        cmd_data->flag |= FSDIO_CMD_FLAG_NEED_RESP_CRC;
    }

    if (SCF_RSP_PRESENT & cmdinfo->flags)
    {
        cmd_data->flag |= FSDIO_CMD_FLAG_EXP_RESP;

        if (SCF_RSP_136 & cmdinfo->flags)
        {
            cmd_data->flag |= FSDIO_CMD_FLAG_EXP_LONG_RESP;
        }
    }

    if (cmdinfo->data)
    {
        FASSERT_MSG(cmd_data->data_p, "Data buffer shall be assigned.");
        cmd_data->flag |= FSDIO_CMD_FLAG_EXP_DATA;

        if (SCF_CMD_READ & cmdinfo->flags)
        {
            cmd_data->flag |= FSDIO_CMD_FLAG_READ_DATA;
        }
        else
        {
            cmd_data->flag |= FSDIO_CMD_FLAG_WRITE_DATA;
        }

        cmd_data->data_p->buf = cmdinfo->data;
        cmd_data->data_p->buf_p = (uintptr)cmdinfo->data; /* physical address equals with virtual address */
        cmd_data->data_p->blksz = cmdinfo->blklen;
        cmd_data->data_p->datalen = cmdinfo->datalen;
        FSDIO_INFO("buf@%p, blksz: %d, datalen: %d", 
                    cmd_data->data_p->buf,
                    cmd_data->data_p->blksz,
                    cmd_data->data_p->datalen);
    }

    cmd_data->cmdidx = cmdinfo->opcode;
    cmd_data->cmdarg = cmdinfo->arg;

    /* FSdioDumpCmdInfo(cmd_data); */
    return;
}

static sdmmc_err_t fsdio_host_pre_command(int slot, sdmmc_command_t *cmdinfo)
{
    sdmmc_host_instance_t *instance_p = fsdio_get_instance(slot);
    sdmmc_host_slot_info *slot_info = fsdio_get_slot_info(slot);

    if (!(instance_p->config.flags & SDMMC_HOST_IO_SUPPORT)) /* filter function card only command */
    {
        FSDIO_INFO("opcode = %d", cmdinfo->opcode);
        if (SD_IO_SEND_OP_COND == cmdinfo->opcode)
        {
            FSDIO_INFO("Reject SD_IO_SEND_OP_COND for memory card.");
            return SDMMC_FAIL; /* just return a error code to bypass */
        }

        if (SD_IO_RW_DIRECT == cmdinfo->opcode)
        {
            FSDIO_INFO("Reject SD_IO_RW_DIRECT for memory card.");
            return SDMMC_ERR_TIMEOUT;            
        }
    }

    if (!(instance_p->config.flags & SDMMC_HOST_REMOVABLE_CARD)) /* filter SD memory card only command */
    {
        FSDIO_INFO("opcode = %d", cmdinfo->opcode);
        if ((SD_SEND_IF_COND == cmdinfo->opcode) && (NULL == cmdinfo->data))
        {
            /* MMC_SEND_EXT_CSD = 8 need to send so check cmdinfo->data to distinguish 
                SD_SEND_IF_COND from MMC_SEND_EXT_CSD  */
            FSDIO_INFO("Reject SD_SEND_IF_COND for eMMC.");
            return SDMMC_ERR_NOT_SUPPORTED;          
        }

        if (MMC_APP_CMD == cmdinfo->opcode)
        {
            FSDIO_INFO("Reject MMC_APP_CMD for eMMC.");
            return SDMMC_ERR_TIMEOUT;
        }        
    }

    if ((MMC_READ_BLOCK_MULTIPLE == cmdinfo->opcode) || 
        (MMC_WRITE_BLOCK_MULTIPLE == cmdinfo->opcode) && (cmdinfo->data))
    {
        u32 block_count = cmdinfo->datalen / cmdinfo->blklen;
        if (block_count > 1U)
        {
            return sdmmc_cmd_set_block_count(&instance_p->card, block_count);
        }
    }

    return SDMMC_OK;
}

static sdmmc_err_t fsdio_host_post_command(int slot, sdmmc_command_t *cmdinfo)
{
    sdmmc_host_instance_t *instance_p = fsdio_get_instance(slot);
    sdmmc_host_slot_info *slot_info = fsdio_get_slot_info(slot);

    if ((cmdinfo->error) && (cmdinfo->data))
    {
        (void)sdmmc_send_stop_transmission(&(instance_p->card));
    }

    return cmdinfo->error;
}

static sdmmc_err_t fsdio_host_do_transaction_irq(int slot, sdmmc_command_t *cmdinfo)
{
    FASSERT(cmdinfo);
    sdmmc_err_t ret = SDMMC_OK;
    sdmmc_host_instance_t *instance_p = fsdio_get_instance(slot);
    sdmmc_host_t *host_p = &instance_p->host;
    FSdio *ctrl_p = fsdio_get_ctrl(slot);
    FError err = FSDIO_SUCCESS;
    int timeout = host_p->command_timeout_ms;

    sdmmc_host_slot_info *slot_info = fsdio_get_slot_info(slot);
    FSdioCmdData *cmd_data = &(slot_info->cmd_pkg);
    FSdioData *trans_data = &(slot_info->dat_pkg);
    
    ret = fsdio_host_pre_command(slot, cmdinfo);
    if (SDMMC_OK != ret)
    {
        /* if not supported (bypass), return ok, if other error (reject), return fail */
        return ret;
    }

    memset(cmd_data, 0, sizeof(*cmd_data));
    if (cmdinfo->data)
    {
        memset(trans_data, 0, sizeof(*trans_data));
        cmd_data->data_p = trans_data;
        slot_info->data_done = FALSE;
    }
    else
    {
        cmd_data->data_p = NULL;
        slot_info->data_done = TRUE; /* no need to wait for data */
    }

    fsdio_convert_cmdinfo(cmdinfo, cmd_data);

    slot_info->cmd_done = FALSE;
    slot_info->err_occur = FALSE;

    if (SDMMC_HOST_WORK_MODE_DMA & instance_p->config.flags)
    {
        err = FSdioDMATransfer(ctrl_p, cmd_data);
        if (FSDIO_SUCCESS != err)
        {
            ret = SDMMC_FAIL;
            goto err_exit;
        }
    }
    else
    {
        err = FSdioPIOTransfer(ctrl_p, cmd_data);
        if (FSDIO_SUCCESS != err)
        {
            ret = SDMMC_FAIL;
            goto err_exit;
        }
    }
    
    while ((TRUE != slot_info->cmd_done) || (TRUE != slot_info->data_done) || (--timeout < 0))
    {
        fsdio_host_relax();
    }

    if (timeout <= 0)
    {
        FSDIO_ERROR("Wait command done timeout !!!");
        ret = SDMMC_FAIL;
        goto err_exit;
    }

    if (!slot_info->err_occur)
    {
        err = FSdioGetCmdResponse(ctrl_p, cmd_data);
        if (FSDIO_SUCCESS != err)
        {
            FSDIO_ERROR("Transfer cmd and data failed !!!");
            goto err_exit;
        }

        if (SCF_RSP_PRESENT & cmdinfo->flags)
        {
            memcpy(cmdinfo->response, cmd_data->response, sizeof(u32) * 4);
        }
    }
    else
    {
        cmdinfo->error = SDMMC_FAIL;
    }

    ret = fsdio_host_post_command(slot, cmdinfo);
    if (SDMMC_OK != ret)
    {
        goto err_exit;
    }

err_exit:
    return ret;
}

static sdmmc_err_t fsdio_host_do_transaction_poll(int slot, sdmmc_command_t *cmdinfo)
{
    FASSERT(cmdinfo);
    sdmmc_err_t ret = SDMMC_OK;
    sdmmc_host_instance_t *instance_p = fsdio_get_instance(slot);
    sdmmc_host_t *host_p = &instance_p->host;
    FSdio *ctrl_p = fsdio_get_ctrl(slot);
    FError err = FSDIO_SUCCESS;
    int timeout = host_p->command_timeout_ms;

    sdmmc_host_slot_info *slot_info = fsdio_get_slot_info(slot);
    FSdioCmdData *cmd_data = &(slot_info->cmd_pkg);
    FSdioData *trans_data = &(slot_info->dat_pkg);

    ret = fsdio_host_pre_command(slot, cmdinfo);
    if (SDMMC_OK != ret)
    {
        return ret;
    }

    memset(cmd_data, 0, sizeof(*cmd_data));
    if (cmdinfo->data)
    {
        memset(trans_data, 0, sizeof(*trans_data));
        cmd_data->data_p = trans_data;
    }

    fsdio_convert_cmdinfo(cmdinfo, cmd_data);
    if (SDMMC_HOST_WORK_MODE_DMA & instance_p->config.flags)
    {
        err = FSdioDMATransfer(ctrl_p, cmd_data);
        if (FSDIO_SUCCESS != err)
        {
            ret = SDMMC_FAIL;
            goto err_exit;
        }

        err = FSdioPollWaitDMAEnd(ctrl_p, cmd_data);
        if (FSDIO_SUCCESS != err)
        {
            ret = SDMMC_FAIL;
        }        
    }
    else
    {
        err = FSdioPIOTransfer(ctrl_p, cmd_data);
        if (FSDIO_SUCCESS != err)
        {
            ret = SDMMC_FAIL;
            goto err_exit;
        }

        err = FSdioPollWaitPIOEnd(ctrl_p, cmd_data);
        if (FSDIO_SUCCESS != err)
        {
            ret = SDMMC_FAIL;
        }
    }

    if (SDMMC_OK == ret)
    {
        if (SCF_RSP_PRESENT & cmdinfo->flags)
        {
            memcpy(cmdinfo->response, cmd_data->response, sizeof(u32) * 4);
        }
    }
    else
    {
        cmdinfo->error = SDMMC_FAIL;
    }

    ret = fsdio_host_post_command(slot, cmdinfo);
    if (SDMMC_OK != ret)
    {
        goto err_exit;
    }

err_exit:
    if (SDMMC_OK != ret)
    {
        FSdioDumpRegister(ctrl_p->config.base_addr);
        FSDIO_ERROR("Restart controller from error state !!!");
        (void)FSdioRestart(ctrl_p);
    }

    return ret;
}

static sdmmc_err_t fsdio_ctrl_init(int slot_id)
{
    sdmmc_err_t ret = SDMMC_OK;
    FSdio *ctrl_p = fsdio_get_ctrl(slot_id);
    sdmmc_host_instance_t *instance = fsdio_get_instance(slot_id);
    FSdioConfig ctrl_config;
    const FSdioConfig *def_ctrl_config;

    memset(&ctrl_config, 0, sizeof(ctrl_config));

    SDMMC_ASSERT((def_ctrl_config = FSdioLookupConfig(slot_id)) != NULL);
    ctrl_config = *def_ctrl_config;

    if (instance->config.flags & SDMMC_HOST_WORK_MODE_DMA)
        ctrl_config.trans_mode = FSDIO_IDMA_TRANS_MODE;
    else
        ctrl_config.trans_mode = FSDIO_PIO_TRANS_MODE;

    if (instance->config.flags & SDMMC_HOST_REMOVABLE_CARD)
        ctrl_config.non_removable = FALSE;
    else
        ctrl_config.non_removable = TRUE;

    if ((instance->config.flags & SDMMC_HOST_WORK_MODE_IRQ) && 
        !(instance->config.flags & SDMMC_HOST_WORK_MODE_DMA))
    {
        FSDIO_ERROR("Pio can only work in poll mode !!!");
        ret = SDMMC_FAIL;
        goto err_exit;        
    }

    ctrl_config.filp_resp_byte_order = FALSE; /* sdmmc will handle byte order filp */
    if (FSDIO_SUCCESS != FSdioCfgInitialize(ctrl_p, &ctrl_config))
    {
        FSDIO_ERROR("Sdio ctrl init failed.");
        ret = SDMMC_FAIL;
        goto err_exit;
    }

    FSdioRegisterRelaxHandler(ctrl_p, fsdio_host_relax);

    if (instance->config.flags & SDMMC_HOST_WORK_MODE_IRQ)
    {
        fsdio_setup_interrupt(slot_id);
        FSdioRegisterEvtHandler(ctrl_p, FSDIO_EVT_CARD_DETECTED, fsdio_card_detected, NULL);
        FSdioRegisterEvtHandler(ctrl_p, FSDIO_EVT_ERR_OCCURE, fsdio_card_error_occur, NULL);
        FSdioRegisterEvtHandler(ctrl_p, FSDIO_EVT_CMD_DONE, fsdio_card_cmd_done, NULL);
        FSdioRegisterEvtHandler(ctrl_p, FSDIO_EVT_DATA_DONE, fsdio_card_data_done, NULL);  
    }

    host_slot_info[slot_id].rw_desc = sdmmc_sys_heap_caps_malloc(SDIO_MAX_BLK_TRANS * sizeof(FSdioIDmaDesc), MALLOC_CAP_DESC);
    if (NULL == host_slot_info[slot_id].rw_desc)
    {
        ret = SDMMC_FAIL;
        goto err_exit;         
    }

    if ((instance->config.flags & SDMMC_HOST_WORK_MODE_DMA) && 
        (FSDIO_SUCCESS != FSdioSetIDMAList(ctrl_p, host_slot_info[slot_id].rw_desc, SDIO_MAX_BLK_TRANS)))
    {
        FSDIO_ERROR("Sdio ctrl setup DMA failed.");
        ret = SDMMC_FAIL;
        goto err_exit; 
    }

err_exit:
    return ret;
}

static sdmmc_err_t fsdio_card_init(int slot_id)
{
    sdmmc_err_t ret = SDMMC_OK;
    sdmmc_host_instance_t *instance = fsdio_get_instance(slot_id);
    sdmmc_host_t *host_p = &instance->host;
    sdmmc_card_t *card_p = &instance->card;

    ret = fsdio_host_set_card_clk(slot_id, SDMMC_FREQ_PROBING);
    if (SDMMC_OK != ret)
    {
        FSDIO_ERROR("Sdio ctrl setup 400kHz clock failed.");
        return ret;
    }

    ret = fsdio_host_set_bus_width(slot_id, 1U);
    if (SDMMC_OK != ret)
    {
        FSDIO_ERROR("Sdio ctrl set 1 bus width failed.");
        return ret;
    }

    ret = sdmmc_card_init(host_p, card_p);
    if (SDMMC_OK == ret)
    {
        sdmmc_card_print_info(card_p);
    }

    return ret;
}

sdmmc_err_t fsdio_host_init(sdmmc_host_instance_t *const instance, const sdmmc_host_config_t* config)
{
    SDMMC_ASSERT(instance && config);
    SDMMC_ASSERT(config->slot < FSDIO_NUM);
    int slot_id = config->slot;
    sdmmc_host_t *host_p = &instance->host;
    sdmmc_card_t *card_p = &instance->card;
    FSdio *ctrl_p = fsdio_get_ctrl(slot_id);
    sdmmc_err_t ret = SDMMC_OK;

    memset(host_p, 0, sizeof(*host_p));
    memset(card_p, 0, sizeof(*card_p));

    instance->private = &host_slot_info[slot_id];
    host_slot_info[slot_id].instance = instance;

    if (&(instance->config) != config)
        instance->config = *config;
    FSDIO_INFO("instance->config.flags = 0x%x.", instance->config.flags);

    /* config sdmmc host */
    host_p->flags |=  (SDMMC_HOST_FLAG_4BIT | SDMMC_HOST_FLAG_1BIT); /* support 1-bit and 4-bit dat line */
    host_p->flags &= ~SDMMC_HOST_FLAG_DDR; /* no DDR mode, support SDR only */
    host_p->flags &= ~SDMMC_HOST_FLAG_SPI; /* no SPI mode */
    host_p->slot = slot_id;
     /* SD 2.0, no UHS-I support, so support 3.3v only */
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

    /* register sdio host ops */
    host_p->set_bus_width = fsdio_host_set_bus_width;
    host_p->get_bus_width = fsdio_host_get_slot_width;
    host_p->set_bus_ddr_mode = fsdio_host_set_bus_ddr_mode;
    host_p->set_card_clk = fsdio_host_set_card_clk;
    host_p->get_real_freq = fsdio_host_get_real_freq;

    if (instance->config.flags & SDMMC_HOST_WORK_MODE_IRQ)
        host_p->do_transaction = fsdio_host_do_transaction_irq;
    else
        host_p->do_transaction = fsdio_host_do_transaction_poll;

    host_p->io_int_enable = fsdio_host_io_int_enable;
    host_p->io_int_wait = fsdio_host_io_int_wait;

    ret = fsdio_ctrl_init(slot_id);
    if (SDMMC_OK != ret)
    {
        return ret;
    }

    ret = fsdio_card_init(slot_id);
    if (SDMMC_OK != ret)
    {
        FSdioDumpRegister(ctrl_p->config.base_addr);
        return ret;
    }

    instance->is_ready = FT_COMPONENT_IS_READY;
    return ret;
}

sdmmc_err_t fsdio_host_deinit(sdmmc_host_instance_t *const instance)
{
    sdmmc_err_t ret = SDMMC_OK;

    return ret;   
}

sdmmc_err_t fsdio_host_lock(sdmmc_host_t *const host)
{
    int slot_id = host->slot;
    sdmmc_host_slot_info *slot_info = fsdio_get_slot_info(slot_id);

    while (slot_info->locked == TRUE) /* wait until other task/thread unlock */
    {
        fsdio_host_relax();
    }
    
    slot_info->locked = TRUE;
}

void fsdio_host_unlock(sdmmc_host_t *const host)
{
    int slot_id = host->slot;
    sdmmc_host_slot_info *slot_info = fsdio_get_slot_info(slot_id);

    FASSERT(TRUE == slot_info->locked);
    slot_info->locked = FALSE;
}