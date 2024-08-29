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
 * FilePath: fspim_sfud_core.c
 * Date: 2022-02-10 14:53:44
 * LastEditTime: 2022-03-01 09:01:56
 * Description:  This file is for providing sfud func based on spi.
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu    2021/12/10   first commit
 * 1.0.1 wangxiaodong 2022/12/1    parameter naming change
 */

#include "fspim_sfud_core.h"
#include "fparameters.h"
#include "sfud_def.h"
#include "finterrupt.h"
#include "fspim_hw.h"
#include "sdkconfig.h"
#include "fcpu_info.h"
#include "fio_mux.h"

#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif
#ifdef FSPIM_VERSION_1
#include "fgpio.h"
#endif
 
/* ../port/sfup_port.c */
extern void sfud_log_debug(const char *file, const long line, const char *format, ...);
extern void sfud_log_info(const char *format, ...);

#define SFUD_SPI_CS_ON       TRUE
#define SFUD_SPI_CS_OFF      FALSE 
#define SFUD_TX_BUF_LEN      256

typedef struct 
{
    FSpim spim;
#ifdef FSPIM_VERSION_1
    FGpio gpio;
    FGpioPin cs_pin;
#endif
} FSpimCore;

static u32 device_select_mask ; /* 每一位用于指示那个设备被选择，如0x3 ,则 fspim0 ，fspim1 被选择 */
static FSpimCore fspim[FSPI_NUM] = {0} ;

#if defined(FSPIM_VERSION_1)
/* D2000/FT2000-4 使用GPIO引脚控制片选信号 */
static FGpioPinId cs_pin_id = 
{
    .ctrl = FGPIO1_ID,
    .port = FGPIO_PORT_A,
    .pin = FGPIO_PIN_5
};

static int SfudSpiPortSetupCs(FSpimCore *core_p)
{
    FGpioConfig input_cfg = *FGpioLookupConfig(FGPIO1_ID);
    FGpio *gpio_p = &core_p->gpio;
    FGpioPin *cs_p = &core_p->cs_pin;
    FError ret = FSPIM_SUCCESS;

    (void)FGpioCfgInitialize(gpio_p, &input_cfg);
    (void)FGpioPinInitialize(gpio_p, cs_p, cs_pin_id);
    FGpioSetDirection(cs_p, FGPIO_DIR_OUTPUT);

    return SFUD_SUCCESS;
}

static void SfudSpiPortCsOnOff(FSpimCore *core_p, boolean on)
{
    FGpioPin *cs_p = &core_p->cs_pin;
    if (on)
        FGpioSetOutputValue(cs_p, FGPIO_PIN_LOW);
    else
        FGpioSetOutputValue(cs_p, FGPIO_PIN_HIGH);
}

#elif defined(FSPIM_VERSION_2) || defined (TARDIGRADE)
/* E2000 使用FSpimSetChipSelection控制片选信号 */
static int SfudSpiPortSetupCs(FSpimCore *core_p)
{
    return SFUD_SUCCESS;
}

static void SfudSpiPortCsOnOff(FSpimCore *core_p, boolean on)
{
    FSpim * spim_p = &core_p->spim;
    FSpimSetChipSelection(spim_p, on);
}
#endif

#ifdef CONFIG_SFUD_TRANS_MODE_POLL_FIFO
static sfud_err SfudSpiPortPollFifoTransfer(FSpim *spim_p, const uint8_t *write_buf, 
                                                size_t write_size, uint8_t *read_buf,
                                                size_t read_size)
{
    sfud_err result = SFUD_SUCCESS;

    SFUD_DEBUG("spim@%p beg+++++++++++++++++++++++++++++++++++++++++++++++++++", spim_p);
    SFUD_DEBUG("++++  Write %d Bytes @%p: 0x%x, 0x%x, 0x%x",
               write_size, write_buf,
               ((NULL != write_buf) && (write_size > 0)) ? write_buf[0] : 0xff,
               ((NULL != write_buf) && (write_size > 1)) ? write_buf[1] : 0xff,
               ((NULL != write_buf) && (write_size > 2)) ? write_buf[2] : 0xff);
    SFUD_DEBUG("++++  Read %d Bytes @%p: 0x%x, 0x%x, 0x%x", 
               read_size, read_buf, 
               ((NULL != read_buf) && (read_size > 0)) ? read_buf[0] : 0xff, 
               ((NULL != read_buf) && (read_size > 1)) ? read_buf[1] : 0xff,
               ((NULL != read_buf) && (read_size > 2)) ? read_buf[2] : 0xff);
    SFUD_DEBUG("spim@%p end+++++++++++++++++++++++++++++++++++++++++++++++++++", spim_p);
    if (write_size && read_size)
    {
        result = FSpimTransferPollFifo(spim_p, write_buf, NULL, write_size);
        if (SFUD_SUCCESS != result)
            goto err_ret;

        result = FSpimTransferPollFifo(spim_p, NULL, read_buf, read_size);
    }
    else if (write_size)
    {
        result = FSpimTransferPollFifo(spim_p, write_buf, NULL, write_size);
    }
    else if (read_size)
    {
        result = FSpimTransferPollFifo(spim_p, NULL, read_buf, read_size);
    }

err_ret:
    return result;      
}

#endif

#ifdef CONFIG_SFUD_TRANS_MODE_INTRRUPT
boolean sfud_spi_rx_done = FALSE;
static void SfudSpiRxDoneHandler(void *instance_p, void *param)
{
    FASSERT(instance_p && param);
    FSpim *spim_p = (FSpim *)instance_p;
    boolean *done_flag = (boolean *)param;

    *done_flag = TRUE;
    return;
}

static FError SfudSpiSetupInterrupt(FSpim *instance_p)
{
	FASSERT(instance_p);
    FSpimConfig *config_p = &instance_p->config;
    uintptr base_addr = config_p->base_addr;
    u32 evt;
    u32 mask;
    u32 cpu_id;

	if (FT_COMPONENT_IS_READY != instance_p->is_ready)
	{
		SFUD_DEBUG("device is already initialized!!!");
		return FSPIM_ERR_NOT_READY;
	}

    GetCpuId(&cpu_id);
    SFUD_DEBUG("cpu_id is cpu_id %d", cpu_id);
    InterruptSetTargetCpus(config_p->irq_num, cpu_id);

    InterruptSetPriority(config_p->irq_num, config_p->irq_prority);

    /* register intr callback */
    InterruptInstall(config_p->irq_num, 
                     FSpimInterruptHandler, 
                     instance_p, 
                     NULL);

    /* enable tx fifo overflow / rx overflow / rx full */
    FSpimMaskIrq(base_addr, FSPIM_IMR_ALL_BITS);

    /* enable irq */
    InterruptUmask(config_p->irq_num);

    return FSPIM_SUCCESS;    
}

static int SfudSpiPortWaitRxDone(int timeout)
{
    while (TRUE != sfud_spi_rx_done)
    {
        fsleep_microsec(10);
        if (0 >= --timeout)
            break;
    }

    if (0 >= timeout)
    {
        SFUD_DEBUG("wait rx timeout \r\n");
        return -1;
    }

    return 0;
}

static sfud_err SfudSpiPortInterruptRx(FSpim *spim_p, uint8_t *read_buf, size_t read_size)
{
    sfud_err result = SFUD_SUCCESS;

    sfud_spi_rx_done = FALSE;
    if (FSPIM_SUCCESS != FSpimTransferByInterrupt(spim_p, NULL, read_buf, read_size)){
        SFUD_ERROR("interrupt read by fifo failed !!!");
        return SFUD_ERR_READ;
    }

    if (0 != SfudSpiPortWaitRxDone(50000000)){
        SFUD_ERROR("wait timeout 500 seconds used up !!!");
        return SFUD_ERR_TIMEOUT;
    }

    return result;
}

static sfud_err SfudSpiPortInterruptTx(FSpim *spim_p, const uint8_t *write_buf, size_t write_size)
{
    sfud_err result = SFUD_SUCCESS;

    sfud_spi_rx_done = FALSE;
    if (FSPIM_SUCCESS != FSpimTransferByInterrupt(spim_p, write_buf, NULL, write_size)){
        SFUD_ERROR("interrupt write by fifo failed !!!");
        return SFUD_ERR_WRITE;
    }

    /* timeout 10 us * 50000000 = 500s */
    if (0 != SfudSpiPortWaitRxDone(50000000)){
        SFUD_ERROR("wait timeout 500 seconds used up !!!");
        return SFUD_ERR_TIMEOUT;
    }

    return result;
}

static sfud_err SfudSpiPortInterruptTransfer(FSpim *spim_p, const uint8_t *write_buf, 
                                                size_t write_size, uint8_t *read_buf,
                                                size_t read_size)
{
    sfud_err result = SFUD_SUCCESS;
    SFUD_DEBUG("spim@%p beg+++++++++++++++++++++++++++++++++++++++++++++++++++", spim_p);
    SFUD_DEBUG("++++  Write %d Bytes @%p: 0x%x, 0x%x, 0x%x", 
               write_size, write_buf, 
               ((NULL != write_buf) && (write_size > 0)) ? write_buf[0] : 0xff, 
               ((NULL != write_buf) && (write_size > 1)) ? write_buf[1] : 0xff,
               ((NULL != write_buf) && (write_size > 2)) ? write_buf[2] : 0xff);
    SFUD_DEBUG("++++  Read %d Bytes @%p: 0x%x, 0x%x, 0x%x", 
               read_size, read_buf, 
               ((NULL != read_buf) && (read_size > 0)) ? read_buf[0] : 0xff, 
               ((NULL != read_buf) && (read_size > 1)) ? read_buf[1] : 0xff,
               ((NULL != read_buf) && (read_size > 2)) ? read_buf[2] : 0xff);
    SFUD_DEBUG("spim@%p end+++++++++++++++++++++++++++++++++++++++++++++++++++", spim_p);

    if (write_size && read_size)
    {
        result = SfudSpiPortInterruptTx(spim_p, write_buf, write_size);
        if (SFUD_SUCCESS != result)
            goto err_ret;

        result = SfudSpiPortInterruptRx(spim_p, read_buf, read_size);
    }
    else if (write_size)
    {
        result = SfudSpiPortInterruptTx(spim_p, write_buf, write_size);
    }
    else if (read_size)
    {
        result = SfudSpiPortInterruptRx(spim_p, read_buf, read_size);
    }

err_ret:
    return result;  
}
#endif


static sfud_err FspiWriteRead(const sfud_spi *spi, const uint8_t *write_buf, size_t write_size, uint8_t *read_buf,
        size_t read_size) {
    sfud_err result = SFUD_SUCCESS;
    uint8_t send_data, read_data;
    FSpimCore *spi_p = (FSpimCore *)spi->user_data ;
    FSpim * spim_p = &spi_p->spim ;
    if (write_size) {
        SFUD_ASSERT(write_buf);
    }
    if (read_size) {
        SFUD_ASSERT(read_buf);
    }

    /**
     * add your spi write and read code
     */
    SfudSpiPortCsOnOff(spi_p,SFUD_SPI_CS_ON);

#ifdef CONFIG_SFUD_TRANS_MODE_POLL_FIFO
    result = SfudSpiPortPollFifoTransfer(spim_p, write_buf, write_size, read_buf, read_size);
#endif

#ifdef CONFIG_SFUD_TRANS_MODE_INTRRUPT
    result = SfudSpiPortInterruptTransfer(spim_p, write_buf, write_size, read_buf, read_size);
#endif
    SfudSpiPortCsOnOff(spi_p,SFUD_SPI_CS_OFF);


    return result;
}

sfud_err FSpimProbe(sfud_flash *flash)
{

    sfud_spi *spi_p = &flash->spi; 
    sfud_err result = SFUD_SUCCESS;
    FSpim *spim_p;
    u32 spim_id = FSPI0_ID;

    if (!memcmp(FSPIM0_SFUD_NAME, spi_p->name, strlen(FSPIM0_SFUD_NAME)))
    {
        spim_id = FSPI0_ID;
    } 
    else if (!memcmp(FSPIM1_SFUD_NAME, spi_p->name, strlen(FSPIM1_SFUD_NAME)))
    {
        spim_id = FSPI1_ID;
    }

#if defined(CONFIG_TARGET_E2000)
    else if (!memcmp(FSPIM2_SFUD_NAME, spi_p->name, strlen(FSPIM2_SFUD_NAME)))
    {
        spim_id = FSPI2_ID;
    } 
    else if (!memcmp(FSPIM3_SFUD_NAME, spi_p->name, strlen(FSPIM3_SFUD_NAME)))
    {
        spim_id = FSPI3_ID;
    } 
#endif

    else
    {
        return SFUD_ERR_NOT_FOUND;
    }
    
    spim_p = &fspim[spim_id].spim;
    FSpimConfig input_cfg = *FSpimLookupConfig(spim_id);
    u32 slave_dev = FSPIM_SLAVE_DEV_0;
    
    FIOPadSetSpimMux(spim_id);

    memset(&fspim[spim_id], 0, sizeof(fspim[spim_id]));
    if (0 != SfudSpiPortSetupCs(&fspim[spim_id]))
    {
        SFUD_DEBUG("init gpio cs failed");
        result = SFUD_ERR_INIT_FAILED;
        return result;
    }

    input_cfg.slave_dev_id = FSPIM_SLAVE_DEV_0;
    input_cfg.cpha = FSPIM_CPHA_1_EDGE;
    input_cfg.cpol = FSPIM_CPOL_LOW;
    input_cfg.n_bytes = FSPIM_1_BYTE; /* sfud only support 1 bytes read/write */
    input_cfg.max_freq_hz = 12000000;

    if (FSPIM_SUCCESS != FSpimCfgInitialize(spim_p, &input_cfg))
    {
        SFUD_DEBUG("init spi failed");
        result = SFUD_ERR_INIT_FAILED;
        return result;
    }

#ifdef CONFIG_SFUD_TRANS_MODE_INTRRUPT
    if (FSPIM_SUCCESS != SfudSpiSetupInterrupt(spim_p))
    {
        SFUD_DEBUG("init spi interrupt failed");
        result = SFUD_ERR_INIT_FAILED;
        return result;               
    }

    FSpimRegisterIntrruptHandler(spim_p, FSPIM_INTR_EVT_RX_DONE, SfudSpiRxDoneHandler, 
                                    &sfud_spi_rx_done);
#endif

    flash->spi.wr = FspiWriteRead;
    flash->spi.user_data = &fspim[spim_id];
    /* adout 200 seconds timeout */
    flash->retry.times = 200 * 10000;

    device_select_mask |= 0x1 ;

    return result;
}

