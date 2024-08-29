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
 * FilePath: fgmac_os.c
 * Date: 2022-02-24 13:42:19
 * LastEditTime: 2022-03-25 09:16:57
 * Description:  This file is for gmac driver.Functions in this file are the minimum required functions for drivers.
 *
 * Modify History:
 *  Ver   Who        Date                   Changes
 * ----- ------    --------     --------------------------------------
 *  1.0  huanghe  2022/11/15    first release
 */


#include <FreeRTOS.h>
#include <event_groups.h>
#include <semphr.h>
#include <string.h>
#include <stdio.h>
#include "fgmac_os.h"
#include "fgmac_os.h"
#include "fassert.h"
#include "fio.h"
#include "fassert.h"
#include "finterrupt.h"
#include "list.h"
#include "fcpu_info.h"
#include "sys_arch.h"
#include "fdebug.h"
#include "lwip_port.h"
#include "fparameters.h"

#define OS_MAC_DEBUG_TAG "OS_MAC"
#define OS_MAC_DEBUG_D(format, ...) FT_DEBUG_PRINT_D(OS_MAC_DEBUG_TAG, format, ##__VA_ARGS__)
#define OS_MAC_DEBUG_I(format, ...) FT_DEBUG_PRINT_I(OS_MAC_DEBUG_TAG, format, ##__VA_ARGS__)
#define OS_MAC_DEBUG_E(format, ...) FT_DEBUG_PRINT_E(OS_MAC_DEBUG_TAG, format, ##__VA_ARGS__)
#define OS_MAC_DEBUG_W(format, ...) FT_DEBUG_PRINT_W(OS_MAC_DEBUG_TAG, format, ##__VA_ARGS__)

extern void sys_sem_signal(sys_sem_t *sem);
static FGmacOs fgmac_os_instace[FGMAC_NUM] = {0};

static void EthLinkPhyStatusChecker(void *param)
{
    FASSERT(param != NULL);
    FGmac *instance_p = (FGmac *)param;
    uintptr base_addr = instance_p->config.base_addr;

    u32 status = FGMAC_READ_REG32(base_addr, FGMAC_MAC_PHY_STATUS);

    if (FGMAC_RGSMIIIS_LNKSTS_UP == (FGMAC_RGSMIIIS_LNKSTS & status))
    {
        FGmacPhyCfgInitialize(instance_p);
        OS_MAC_DEBUG_I("Link is up.");
    }
    else
    {
        OS_MAC_DEBUG_I("Link is down.");
    }

    return;
}

static void EthLinkDmaErrChecker(void *param)
{
    FASSERT(param != NULL);
    FGmac *instance_p = (FGmac *)param;
    uintptr base_addr = instance_p->config.base_addr;

    u32 reg_val = FGMAC_READ_REG32(base_addr, FGMAC_DMA_INTR_OFFSET);
    u32 status  = FGMAC_READ_REG32(base_addr, FGMAC_DMA_STATUS_OFFSET);

    if ((FGMAC_DMA_STATUS_TPS & status) && (FGMAC_DMA_INTR_ENA_TSE & reg_val))
    {
        OS_MAC_DEBUG_E("Transmit process stopped.");
    }

    if ((FGMAC_DMA_STATUS_TU & status) && (FGMAC_DMA_INTR_ENA_TUE & reg_val))
    {
        OS_MAC_DEBUG_E("Transmit buffer unavailable.");
    }

    if ((FGMAC_DMA_STATUS_TJT & status) && (FGMAC_DMA_INTR_ENA_THE & reg_val))
    {
        OS_MAC_DEBUG_E("Transmit jabber timeout.");
    }

    if ((FGMAC_DMA_STATUS_OVF & status) && (FGMAC_DMA_INTR_ENA_OVE & reg_val))
    {
        OS_MAC_DEBUG_E("Receive overflow.");
    }

    if ((FGMAC_DMA_STATUS_UNF & status) && (FGMAC_DMA_INTR_ENA_UNE & reg_val))
    {
        OS_MAC_DEBUG_E("Transmit underflow.");
    }

    if ((FGMAC_DMA_STATUS_RU & status) && (FGMAC_DMA_INTR_ENA_RUE & reg_val))
    {
        OS_MAC_DEBUG_E("Receive buffer unavailable.");
    }

    if ((FGMAC_DMA_STATUS_RPS & status) && (FGMAC_DMA_INTR_ENA_RSE & reg_val))
    {
        OS_MAC_DEBUG_E("Receive process stopped.");
    }

    if ((FGMAC_DMA_STATUS_RWT & status) && (FGMAC_DMA_INTR_ENA_RWE & reg_val))
    {
        OS_MAC_DEBUG_E("Receive watchdog timeout.");
    }

    if ((FGMAC_DMA_STATUS_ETI & status) && (FGMAC_DMA_INTR_ENA_ETE & reg_val))
    {
        OS_MAC_DEBUG_E("Early transmit interrupt.");
    }

    if ((FGMAC_DMA_STATUS_FBI & status) && (FGMAC_DMA_INTR_ENA_FBE & reg_val))
    {
        OS_MAC_DEBUG_E("Fatal bus error.");
    }

    return;
}

static void EthLinkStatusChecker(void *param)
{
    FASSERT(param);
    FGmac *instance_p = (FGmac *)param;
    uintptr base_addr = instance_p->config.base_addr;
    u32 status = FGMAC_READ_REG32(base_addr, FGMAC_MAC_PHY_STATUS);
    u32 speed_status, duplex_status;
    u32 speed, duplex;

    /* Check the link status */
    if (FGMAC_RGSMIIIS_LNKSTS_UP == (FGMAC_RGSMIIIS_LNKSTS & status))
    {
        speed_status = FGMAC_RGSMIIIS_SPEED & status;
        duplex_status = FGMAC_RGSMIIIS_LNKMODE & status;

        if (FGMAC_RGSMIIIS_SPEED_125MHZ == speed_status)
        {
            speed = FGMAC_PHY_SPEED_1000;
        }
        else if (FGMAC_RGSMIIIS_SPEED_25MHZ == speed_status)
        {
            speed = FGMAC_PHY_SPEED_100;
        }
        else
        {
            speed = FGMAC_PHY_SPEED_10;
        }

        if (FGMAC_RGSMIIIS_LNKMODE_HALF == duplex_status)
        {
            duplex = FGMAC_PHY_MODE_HALFDUPLEX;
        }
        else
        {
            duplex = FGMAC_PHY_MODE_FULLDUPLEX;
        }

        OS_MAC_DEBUG_I("Link is up --- %d/%s",
                       speed, (FGMAC_PHY_MODE_FULLDUPLEX == duplex) ? "full" : "half");
    }
    else
    {
        OS_MAC_DEBUG_I("Link is down ---");
    }
}

static void EthLinkTransDoneCallback(void *param)
{
    FASSERT(param);
    FGmac *instance_p = (FGmac *)param;

    FGmacResumeDmaSend(instance_p->config.base_addr);
    OS_MAC_DEBUG_I("Resume trans.");
    return;
}

static void GmacReceiveCallBack(void *args)
{
    LWIP_ASSERT("args != NULL", (args != NULL));
    struct LwipPort *gmac_netif_p;
    FGmacOs *instance_p = (FGmacOs *)args;
    gmac_netif_p = (struct LwipPort *)instance_p->stack_pointer;
    sys_sem_signal(&gmac_netif_p->sem_rx_data_available);
}


static int FGmacSetupIsr(FGmac *gmac_p)
{
    LWIP_ASSERT("gmac_p != NULL", (gmac_p != NULL));
    FGmacConfig *config_p = &gmac_p->config;
    u32 irq_num = config_p->irq_num;
    u32 cpu_id;


    /* gic initialize */
    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(irq_num, cpu_id);

    /* disable all gmac & dma intr */
    FGmacSetInterruptMask(gmac_p, FGMAC_CTRL_INTR, FGMAC_ISR_MASK_ALL_BITS);
    FGmacSetInterruptMask(gmac_p, FGMAC_DMA_INTR, FGMAC_DMA_INTR_ENA_ALL_MASK);

    InterruptSetPriority(irq_num, GMAC_OS_IRQ_PRIORITY_VALUE);
    InterruptInstall(irq_num, FGmacInterruptHandler, gmac_p, "GMAC-IRQ");

    /* register intr callback */
    FGmacRegisterEvtHandler(gmac_p, FGMAC_PHY_STATUS_EVT, EthLinkPhyStatusChecker);
    FGmacRegisterEvtHandler(gmac_p, FGMAC_DMA_ERR_EVT, EthLinkDmaErrChecker);
    FGmacRegisterEvtHandler(gmac_p, FGMAC_LINK_STATUS_EVT, EthLinkStatusChecker);
    FGmacRegisterEvtHandler(gmac_p, FGMAC_TX_COMPLETE_EVT, EthLinkTransDoneCallback);
    /* Set Receive Callback */
    FGmacRegisterEvtHandler(gmac_p, FGMAC_RX_COMPLETE_EVT, GmacReceiveCallBack);


    /* enable some interrupts */
    FGmacSetInterruptUmask(gmac_p, FGMAC_CTRL_INTR, FGMAC_ISR_MASK_RSIM);
    FGmacSetInterruptUmask(gmac_p, FGMAC_DMA_INTR,
                           FGMAC_DMA_INTR_ENA_NIE | FGMAC_DMA_INTR_ENA_RIE | FGMAC_DMA_INTR_ENA_AIE);

    /* umask intr */
    InterruptUmask(irq_num);

    OS_MAC_DEBUG_I("Gmac interrupt setup done.");
    return 0;
}

/* step 1: initialize instance */
/* step 2: depend on config set some options : JUMBO / IGMP */
/* step 3: FGmacSelectClk */
/* step 4: FGmacInitInterface */
/* step 5: initialize phy */
/* step 6: initialize dma */
/* step 7: initialize interrupt */
/* step 8: start mac */

/* step1 :get driver config */
/* step2 :depend on config set some options :phy */
/* step3 :FGmacCfgInitialize*/
/* step4 :initialize dma  */
/* step5 :initialize interrupt  */
/* step6 :start mac */
FError FGmacOsInit(FGmacOs *instance_p)
{
    FGmacConfig mac_config;
    const FGmacConfig *mac_config_p;
    FGmac *gmac_p ;
    FError status;

    gmac_p = &instance_p->instance;
    OS_MAC_DEBUG_I("instance_id IS %d", instance_p->mac_config.instance_id);
    mac_config_p = FGmacLookupConfig(instance_p->mac_config.instance_id);
    if (mac_config_p == NULL)
    {
        OS_MAC_DEBUG_E("FGmacLookupConfig is error , instance_id is %d", instance_p->mac_config.instance_id);
        return FREERTOS_GMAC_INIT_ERROR;
    }
    mac_config = *mac_config_p;

    if (instance_p->mac_config.autonegotiation)
    {
        mac_config.en_auto_negtiation = 1;
    }
    else
    {
        mac_config.en_auto_negtiation = 0;
    }

    switch (instance_p->mac_config.phy_speed)
    {
        case FGMAC_PHY_SPEED_10M:
            mac_config.speed = 10;
            break;
        case FGMAC_PHY_SPEED_100M:
            mac_config.speed = 100;
            break;
        case FGMAC_PHY_SPEED_1000M:
            mac_config.speed = 1000;
            break;
        default:
            OS_MAC_DEBUG_E("Setting speed is not valid , speed is %d", instance_p->mac_config.phy_speed);
            return FREERTOS_GMAC_INIT_ERROR;
    }

    switch (instance_p->mac_config.phy_duplex)
    {
        case FGMAC_PHY_HALF_DUPLEX:
            mac_config.duplex_mode = 0 ;
            break;
        case FGMAC_PHY_FULL_DUPLEX:
            mac_config.duplex_mode = 1 ;
            break;
    }

    status = FGmacCfgInitialize(gmac_p, &mac_config);
    if (status != FGMAC_SUCCESS)
    {
        OS_MAC_DEBUG_W("In %s:EmacPs Configuration Failed....", __func__);
    }

    FGmacSetMacAddr(instance_p->instance.config.base_addr, (void *)(instance_p->hwaddr));

    /* initialize phy */
    status = FGmacPhyCfgInitialize(gmac_p);
    if (status != FGMAC_SUCCESS)
    {
        OS_MAC_DEBUG_W("FGmacPhyCfgInitialize: init phy failed.");
    }

    /* Initialize Rx Description list : ring Mode */
    status = FGmacSetupRxDescRing(gmac_p, (FGmacDmaDesc *)(instance_p->rx_desc), instance_p->rx_buf, FGMAC_MAX_PACKET_SIZE, GMAC_RX_DESCNUM);
    if (FT_SUCCESS != status)
    {
        OS_MAC_DEBUG_E("Gmac setup rx return err code %d", status);
        FASSERT(FT_SUCCESS == status);
    }

    /* Initialize Tx Description list : ring Mode */
    status = FGmacSetupTxDescRing(gmac_p, (FGmacDmaDesc *)(instance_p->tx_desc), instance_p->tx_buf, FGMAC_MAX_PACKET_SIZE, GMAC_TX_DESCNUM);
    if (FT_SUCCESS != status)
    {
        OS_MAC_DEBUG_E("Gmac setup tx return err code %d", status);
        FASSERT(FT_SUCCESS == status);
    }

    /* initialize interrupt */
    FGmacSetupIsr(gmac_p);



    return FT_SUCCESS ;
}


FGmacOs *FGmacOsGetInstancePointer(FtOsGmacPhyControl *config_p)
{
    FGmacOs *instance_p;
    FASSERT(config_p != NULL);
    FASSERT(config_p->instance_id < FGMAC_NUM);
    FASSERT_MSG(config_p->autonegotiation <= 1, "config_p->autonegotiation %d is over 1", config_p->autonegotiation);
    FASSERT_MSG(config_p->phy_speed <= FGMAC_PHY_SPEED_1000M, "config_p->phy_speed %d is over 1000", config_p->phy_speed);
    FASSERT_MSG(config_p->phy_duplex <= FGMAC_PHY_FULL_DUPLEX, "config_p->phy_duplex %d is over FGMAC_PHY_FULL_DUPLEX", config_p->phy_duplex);

    instance_p = &fgmac_os_instace[config_p->instance_id];
    memcpy(&instance_p->mac_config, config_p, sizeof(FtOsGmacPhyControl));
    return instance_p;
}



FError FGmacOsConfig(FGmacOs *instance_p, int cmd, void *arg)
{
    return FT_SUCCESS;
}


void *FGmacOsRx(FGmacOs *instance_p)
{
    struct pbuf *p = NULL;
    struct pbuf *q = NULL;
    u16 length = 0;
    u8 *buffer;
    volatile FGmacDmaDesc *dma_rx_desc;
    u32 buffer_offset = 0;
    u32 pay_load_offset = 0;
    u32 bytes_left_to_copy = 0;

    u32 desc_buffer_index; /* For Current Desc buffer buf position */
    FGmacOs *os_gmac;
    FGmac *gmac_p;

    gmac_p = &instance_p->instance;

    /* get received frame */
    if (FGmacRecvFrame(gmac_p) != FT_SUCCESS)
    {
        return NULL;
    }

    desc_buffer_index = gmac_p->rx_ring.desc_buf_idx;
    length = (gmac_p->rx_desc[desc_buffer_index].status & FGMAC_DMA_RDES0_FRAME_LEN_MASK) >> FGMAC_DMA_RDES0_FRAME_LEN_SHIFT;
    buffer = (u8 *)(intptr)(gmac_p->rx_desc[desc_buffer_index].buf_addr);

#if ETH_PAD_SIZE
    length += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif

    if (length > 0)
    {
        /* We allocate a pbuf chain of pbufs from the Lwip buffer pool */
        p = pbuf_alloc(PBUF_RAW, length, PBUF_POOL);
    }

#ifdef RAW_DATA_PRINT
    dump_hex(Buffer, (u32)length);
#endif
    if (p != NULL)
    {
#if ETH_PAD_SIZE
        pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif
        dma_rx_desc = &gmac_p->rx_desc[desc_buffer_index];
        buffer_offset = 0;
        for (q = p; q != NULL; q = q->next)
        {
            bytes_left_to_copy = q->len;
            pay_load_offset = 0;
            /* Check if the length of bytes to copy in current pbuf is bigger than Rx buffer size*/
            while ((bytes_left_to_copy + buffer_offset) > FGMAC_MAX_PACKET_SIZE)
            {
                /* Copy data to pbuf */
                memcpy((u8 *)((u8 *)q->payload + pay_load_offset), (u8 *)((u8 *)buffer + buffer_offset), (FGMAC_MAX_PACKET_SIZE - buffer_offset));

                /* Point to next descriptor */
                FGMAC_DMA_INC_DESC(desc_buffer_index, gmac_p->rx_ring.desc_max_num);
                if (desc_buffer_index == gmac_p->rx_ring.desc_idx)
                {
                    break;
                }

                dma_rx_desc = &gmac_p->rx_desc[desc_buffer_index];
                buffer = (u8 *)(intptr)(dma_rx_desc->buf_addr);

                bytes_left_to_copy = bytes_left_to_copy - (FGMAC_MAX_PACKET_SIZE - buffer_offset);
                pay_load_offset = pay_load_offset + (FGMAC_MAX_PACKET_SIZE - buffer_offset);
                buffer_offset = 0;
            }
            /* Copy remaining data in pbuf */
            memcpy((u8 *)((u8 *)q->payload + pay_load_offset), (u8 *)((u8 *)buffer + buffer_offset), bytes_left_to_copy);
            buffer_offset = buffer_offset + bytes_left_to_copy;
        }

#if ETH_PAD_SIZE
        pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif
    }
    else
    {
        OS_MAC_DEBUG_E("Error malloc is %d", length);
    }

    /* Release descriptors to DMA */
    /* Point to first descriptor */
    dma_rx_desc = &gmac_p->rx_desc[desc_buffer_index];
    /* Set Own bit in Rx descriptors: gives the buffers back to DMA */
    for (desc_buffer_index = gmac_p->rx_ring.desc_buf_idx; desc_buffer_index != gmac_p->rx_ring.desc_idx; FGMAC_DMA_INC_DESC(desc_buffer_index, gmac_p->rx_ring.desc_max_num))
    {
        dma_rx_desc->status |= FGMAC_DMA_RDES0_OWN;
        dma_rx_desc = &gmac_p->rx_desc[desc_buffer_index];
    }

    /* Sync index */
    gmac_p->rx_ring.desc_buf_idx = gmac_p->rx_ring.desc_idx;

    FGmacResumeDmaRecv(gmac_p->config.base_addr);

    return p;

}

FError FGmacOsTx(FGmacOs *instance_p, void *tx_buf)
{
    FASSERT(instance_p != NULL);
    FASSERT(tx_buf != NULL);
    err_t errval = ERR_OK;
    struct pbuf *q;
    struct pbuf *p = tx_buf;
    FError ret;
    u8 *buffer = NULL;
    volatile FGmacDmaDesc *dma_tx_desc;
    u32 frame_length = 0;
    u32 buffer_offset = 0;
    u32 bytes_left_to_copy = 0;
    u32 pay_load_offset = 0;
    FGmac *gmac_p;

    gmac_p = &instance_p->instance;
    dma_tx_desc = &gmac_p->tx_desc[gmac_p->tx_ring.desc_buf_idx];
    buffer = (u8 *)(intptr)(dma_tx_desc->buf_addr);

    if (buffer == NULL)
    {
        OS_MAC_DEBUG_I("Error buffer is 0.");
        return ERR_VAL;
    }

#if ETH_PAD_SIZE
    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

    for (q = p; q != NULL; q = q->next)
    {
        /* Is this buffer available? If not, goto error */
        if ((dma_tx_desc->status & FGMAC_DMA_TDES0_OWN) != 0)
        {
            errval = ERR_USE;
            OS_MAC_DEBUG_I("Error errval = ERR_USE;");
            goto error;
        }

        /* Get bytes in current lwIP buffer */
        bytes_left_to_copy = q->len;
        pay_load_offset = 0;

        /* Check if the length of data to copy is bigger than Tx buffer size*/
        while ((bytes_left_to_copy + buffer_offset) > FGMAC_MAX_PACKET_SIZE)
        {
            /* Copy data to Tx buffer*/
            memcpy((u8 *)((u8 *)buffer + buffer_offset), (u8 *)((u8 *)q->payload + pay_load_offset), (FGMAC_MAX_PACKET_SIZE - buffer_offset));
            FGMAC_DMA_INC_DESC(gmac_p->tx_ring.desc_buf_idx, gmac_p->tx_ring.desc_max_num);
            /* Point to next descriptor */
            dma_tx_desc = &gmac_p->tx_desc[gmac_p->tx_ring.desc_buf_idx];

            /* Check if the Bufferis available */
            if ((dma_tx_desc->status & FGMAC_DMA_TDES0_OWN) != (u32)0)
            {
                errval = ERR_USE;
                OS_MAC_DEBUG_I("Check if the Bufferis available.");
                goto error;
            }

            buffer = (u8 *)(intptr)(dma_tx_desc->buf_addr);
            bytes_left_to_copy = bytes_left_to_copy - (FGMAC_MAX_PACKET_SIZE - buffer_offset);
            pay_load_offset = pay_load_offset + (FGMAC_MAX_PACKET_SIZE - buffer_offset);
            frame_length = frame_length + (FGMAC_MAX_PACKET_SIZE - buffer_offset);
            buffer_offset = 0;

            if (buffer == NULL)
            {
                OS_MAC_DEBUG_I("Error Buffer is 0.");
                return ERR_VAL;
            }
        }

        /* Copy the remaining bytes */
        memcpy((u8 *)((u8 *)buffer + buffer_offset), (u8 *)((u8 *)q->payload + pay_load_offset), bytes_left_to_copy);
        buffer_offset = buffer_offset + bytes_left_to_copy;
        frame_length = frame_length + bytes_left_to_copy;
    }

    FGMAC_DMA_INC_DESC(gmac_p->tx_ring.desc_buf_idx, gmac_p->tx_ring.desc_max_num);

#if ETH_PAD_SIZE
    pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

    ret = FGmacSendFrame(gmac_p, frame_length);

    if (ret != FGMAC_SUCCESS)
    {
        errval = ERR_USE;
        OS_MAC_DEBUG_I("Error errval = ERR_USE; FGmacSendFrame.");
        goto error;
    }

error:
    FGmacResmuDmaUnderflow(gmac_p->config.base_addr);

    return errval;
}


static u32 FGmacPhyLinkDetect(FGmac *instance_p, u32 phy_addr)
{
    u16 status;

    /* Read Phy Status register twice to get the confirmation of the current
     * link status.
     */
    FGmacReadPhyReg(instance_p, instance_p->phy_addr, FGMAC_PHY_MII_STATUS_REG, &status);

    if (status & FGMAC_PHY_MII_SR_LSTATUS)
    {
        return 1;
    }
    return 0;
}



enum lwip_port_link_status FGmacPhyStatus(struct LwipPort *gmac_netif_p)
{
    FGmac *gmac_p;
    FGmacOs *instance_p;
    u32  phy_link_status;
    FASSERT(gmac_netif_p != NULL);
    FASSERT(gmac_netif_p->state != NULL);

    instance_p = (FGmacOs *)(gmac_netif_p->state);

    gmac_p = &instance_p->instance;

    if (gmac_p->is_ready != FT_COMPONENT_IS_READY)
    {
        OS_MAC_DEBUG_E("instance_p is not ready.");
        return ETH_LINK_DOWN;
    }

    /* read gmac phy link status */
    phy_link_status = FGmacPhyLinkDetect(gmac_p, gmac_p->phy_addr);

    if (phy_link_status)
    {
        return ETH_LINK_UP;
    }
    else
    {
        return ETH_LINK_DOWN;
    }
}

void FGmacOsStart(FGmacOs *instance_p)
{
    FASSERT(instance_p != NULL);

    /* start mac */
    FGmacStartTrans(&instance_p->instance);
}


void FGmacOsStop(FGmacOs *instance_p)
{
    FASSERT(instance_p != NULL);
    /* step 1 close mac controler  */
    FGmacStopTrans(&instance_p->instance);
}
