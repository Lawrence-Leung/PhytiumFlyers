/*
 * Copyright (c) 2006 Uwe Stuehler <uwe@openbsd.org>
 * Adaptations to ESP-IDF Copyright (c) 2016-2018 Espressif Systems (Shanghai) PTE LTD
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <unistd.h>
#include "sdmmc_common.h"

static const char* TAG = "sdmmc_mmc";


sdmmc_err_t sdmmc_init_mmc_read_ext_csd(sdmmc_card_t* card)
{
    int card_type;
    sdmmc_err_t err = SDMMC_OK;

    uint8_t* ext_csd = sdmmc_sys_heap_caps_malloc(EXT_CSD_MMC_SIZE, MALLOC_CAP_DMA);
    if (!ext_csd) {
        SDMMC_LOGE(TAG, "%s: could not allocate ext_csd", __func__);
        return SDMMC_ERR_NO_MEM;
    }

    uint32_t sectors = 0;

    SDMMC_LOGD(TAG, "MMC version: %d", card->csd.mmc_ver);
    if (card->csd.mmc_ver < MMC_CSD_MMCVER_4_0) {
        err = SDMMC_ERR_NOT_SUPPORTED;
        goto out;
    }

    /* read EXT_CSD */
    err = sdmmc_mmc_send_ext_csd_data(card, ext_csd, EXT_CSD_MMC_SIZE);
    if (err != SDMMC_OK) {
        SDMMC_LOGE(TAG, "%s: send_ext_csd_data error 0x%x", __func__, err);
        goto out;
    }
    card_type = ext_csd[EXT_CSD_CARD_TYPE];
    card->is_ddr = 0;
    if (card_type & EXT_CSD_CARD_TYPE_F_52M_1_8V) { /* 52MHz, 1.8v */
        card->max_freq_khz = SDMMC_FREQ_52M;
        if ((card->host.flags & SDMMC_HOST_FLAG_DDR) &&
                card->host.max_freq_khz >= SDMMC_FREQ_26M &&
                card->host.get_bus_width(card->host.slot) == 4) {
            SDMMC_LOGD(TAG, "card and host support DDR mode");
            card->is_ddr = 1;
        }
    } else if (card_type & EXT_CSD_CARD_TYPE_F_52M) {
        card->max_freq_khz = SDMMC_FREQ_52M;
    } else if (card_type & EXT_CSD_CARD_TYPE_F_26M) {
        card->max_freq_khz = SDMMC_FREQ_26M;
    } else {
        SDMMC_LOGW(TAG, "%s: unknown CARD_TYPE 0x%x", __func__, card_type);
    }
    /* For MMC cards, use speed value from EXT_CSD */
    card->csd.tr_speed = card->max_freq_khz * 1000;
    SDMMC_LOGD(TAG, "MMC card type %d, max_freq_khz=%d, is_ddr=%d", card_type, card->max_freq_khz, card->is_ddr);
    card->max_freq_khz = MIN(card->max_freq_khz, card->host.max_freq_khz);

    if (card->host.flags & SDMMC_HOST_FLAG_8BIT) {
        card->ext_csd.power_class = ext_csd[(card->max_freq_khz > SDMMC_FREQ_26M) ?
                EXT_CSD_PWR_CL_52_360 : EXT_CSD_PWR_CL_26_360] >> 4;
        card->log_bus_width = 3;
    } else if (card->host.flags & SDMMC_HOST_FLAG_4BIT) {
        card->ext_csd.power_class = ext_csd[(card->max_freq_khz > SDMMC_FREQ_26M) ?
                EXT_CSD_PWR_CL_52_360 : EXT_CSD_PWR_CL_26_360] & 0x0f;
        card->log_bus_width = 2;
    } else {
        card->ext_csd.power_class = 0; //card must be able to do full rate at powerclass 0 in 1-bit mode
        card->log_bus_width = 0;
    }

    sectors = ( ext_csd[EXT_CSD_SEC_COUNT + 0] << 0 )
        | ( ext_csd[EXT_CSD_SEC_COUNT + 1] << 8 )
        | ( ext_csd[EXT_CSD_SEC_COUNT + 2] << 16 )
        | ( ext_csd[EXT_CSD_SEC_COUNT + 3] << 24 );

    if (sectors > (2u * 1024 * 1024 * 1024) / 512) {
        card->csd.capacity = sectors;
    }

    /* erased state of a bit, if 1 byte value read is 0xFF else 0x00 */
    card->ext_csd.erase_mem_state = ext_csd[EXT_CSD_ERASED_MEM_CONT];
    card->ext_csd.rev = ext_csd[EXT_CSD_REV];
    card->ext_csd.sec_feature = ext_csd[EXT_CSD_SEC_FEATURE_SUPPORT];

out:
    sdmmc_sys_free(ext_csd);
    return err;
}

sdmmc_err_t sdmmc_init_mmc_bus_width(sdmmc_card_t* card)
{
    sdmmc_err_t err;
    if (card->ext_csd.power_class != 0) {
        err = sdmmc_mmc_switch(card, EXT_CSD_CMD_SET_NORMAL,
                EXT_CSD_POWER_CLASS, card->ext_csd.power_class);
        if (err != SDMMC_OK) {
            SDMMC_LOGE(TAG, "%s: can't change power class (%d bit), 0x%x"
                    , __func__, card->ext_csd.power_class, err);
            return err;
        }
    }

    if (card->log_bus_width > 0) {
        int csd_bus_width_value = EXT_CSD_BUS_WIDTH_1;
        int bus_width = 1;
        if (card->log_bus_width == 2) {
            if (card->is_ddr) {
                csd_bus_width_value = EXT_CSD_BUS_WIDTH_4_DDR;
            } else {
                csd_bus_width_value = EXT_CSD_BUS_WIDTH_4;
            }
            bus_width = 4;
        } else if (card->log_bus_width == 3) {
            if (card->is_ddr) {
                csd_bus_width_value = EXT_CSD_BUS_WIDTH_8_DDR;
            } else {
                csd_bus_width_value = EXT_CSD_BUS_WIDTH_8;
            }
            bus_width = 8;
        }
        err = sdmmc_mmc_switch(card, EXT_CSD_CMD_SET_NORMAL,
                EXT_CSD_BUS_WIDTH, csd_bus_width_value);
        if (err != SDMMC_OK) {
            SDMMC_LOGE(TAG, "%s: can't change bus width (%d bit), 0x%x",
                    __func__, bus_width, err);
            return err;
        }
    }
    return SDMMC_OK;
}

sdmmc_err_t sdmmc_mmc_enable_hs_mode(sdmmc_card_t* card)
{
    sdmmc_err_t err;
    if (card->max_freq_khz > SDMMC_FREQ_26M) {
        /* switch to high speed timing */
        err = sdmmc_mmc_switch(card, EXT_CSD_CMD_SET_NORMAL,
                EXT_CSD_HS_TIMING, EXT_CSD_HS_TIMING_HS);
        if (err != SDMMC_OK) {
            SDMMC_LOGE(TAG, "%s: mmc_switch EXT_CSD_HS_TIMING_HS error 0x%x",
                    __func__, err);
            return err;
        }
    }
    return SDMMC_OK;
}

sdmmc_err_t sdmmc_mmc_decode_cid(int mmc_ver, sdmmc_response_t resp, sdmmc_cid_t* out_cid)
{
    static const char *version[] = {
        [MMC_CSD_MMCVER_1_0] = "1.0",
        [MMC_CSD_MMCVER_1_4] = "1.4",
        [MMC_CSD_MMCVER_2_0] = "2.0",
        [MMC_CSD_MMCVER_3_1] = "3.1",
        [MMC_CSD_MMCVER_4_0] = "4.0"
    };

    SDMMC_LOGD(TAG, "CID info:");
    SDMMC_LOGD(TAG, "   version: %s", 
               (mmc_ver < (int)(sizeof(version) / sizeof(const char*)))?version[mmc_ver]:"N.A");

    if (mmc_ver == MMC_CSD_MMCVER_1_0 ||
            mmc_ver == MMC_CSD_MMCVER_1_4) {
        out_cid->mfg_id = MMC_CID_MID_V1(resp);
        out_cid->oem_id = 0;
        MMC_CID_PNM_V1_CPY(resp, out_cid->name);
        out_cid->revision = MMC_CID_REV_V1(resp);
        out_cid->serial = MMC_CID_PSN_V1(resp);
        out_cid->date = MMC_CID_MDT_V1(resp);
    } else if (mmc_ver == MMC_CSD_MMCVER_2_0 ||
            mmc_ver == MMC_CSD_MMCVER_3_1 ||
            mmc_ver == MMC_CSD_MMCVER_4_0) {
        out_cid->mfg_id = MMC_CID_MID_V2(resp);
        out_cid->oem_id = MMC_CID_OID_V2(resp);
        MMC_CID_PNM_V1_CPY(resp, out_cid->name);
        out_cid->revision = 0;
        out_cid->serial = MMC_CID_PSN_V1(resp);
        out_cid->date = 0;
    }

    SDMMC_LOGD(TAG, "   name: %s", out_cid->name);
    SDMMC_LOGD(TAG, "   mfg_id: %d", out_cid->mfg_id);
    SDMMC_LOGD(TAG, "   oem_id: %d", out_cid->oem_id);
    SDMMC_LOGD(TAG, "   revision: %d", out_cid->revision);
    SDMMC_LOGD(TAG, "   serial: %d", out_cid->serial);
    SDMMC_LOGD(TAG, "   date: %d", out_cid->date);
    return SDMMC_OK;
}

sdmmc_err_t sdmmc_mmc_decode_csd(sdmmc_response_t response, sdmmc_csd_t* out_csd)
{
    SDMMC_LOGD(TAG, "CSD info:");    
    out_csd->csd_ver = MMC_CSD_CSDVER(response);
    if (out_csd->csd_ver == MMC_CSD_CSDVER_1_0 ||
            out_csd->csd_ver == MMC_CSD_CSDVER_2_0 ||
            out_csd->csd_ver == MMC_CSD_CSDVER_EXT_CSD) {
        out_csd->mmc_ver = MMC_CSD_MMCVER(response);
        out_csd->capacity = MMC_CSD_CAPACITY(response);
        out_csd->read_block_len = MMC_CSD_READ_BL_LEN(response);
    } else {
        SDMMC_LOGE(TAG, "unknown MMC CSD structure version 0x%x\n", out_csd->csd_ver);
        return 1;
    }
    int read_bl_size = 1 << out_csd->read_block_len;
    out_csd->sector_size = MIN(read_bl_size, 512);
    if (out_csd->sector_size < read_bl_size) {
        out_csd->capacity *= read_bl_size / out_csd->sector_size;
    }
    /* tr_speed will be determined when reading CXD */
    out_csd->tr_speed = 0;

    SDMMC_LOGD(TAG, "   version: %d", out_csd->csd_ver);
    SDMMC_LOGD(TAG, "   capacity: %ld Byte", out_csd->capacity);
    SDMMC_LOGD(TAG, "   read_block_len: %d Byte", (2 << out_csd->read_block_len));
    SDMMC_LOGD(TAG, "   sector_size: %d Byte", out_csd->sector_size);
    return SDMMC_OK;
}

sdmmc_err_t sdmmc_mmc_send_ext_csd_data(sdmmc_card_t* card, void *out_data, size_t datalen)
{
    SDMMC_ASSERT(SDMMC_PTR_DMA_CAP(out_data));
    sdmmc_command_t cmd = {
        .data = out_data,
        .datalen = datalen,
        .blklen = datalen,
        .opcode = MMC_SEND_EXT_CSD,
        .arg = 0,
        .flags = SCF_CMD_ADTC | SCF_RSP_R1 | SCF_CMD_READ
    };
    return sdmmc_send_cmd(card, &cmd);
}

sdmmc_err_t sdmmc_mmc_switch(sdmmc_card_t* card, uint8_t set, uint8_t index, uint8_t value)
{
    sdmmc_command_t cmd = {
            .opcode = MMC_SWITCH,
            .arg = (MMC_SWITCH_MODE_WRITE_BYTE << 24) | (index << 16) | (value << 8) | set,
            .flags = SCF_RSP_R1B | SCF_CMD_AC | SCF_WAIT_BUSY,
    };
    sdmmc_err_t err = sdmmc_send_cmd(card, &cmd);
    if (err == SDMMC_OK) {
        //check response bit to see that switch was accepted
        if (MMC_R1(cmd.response) & MMC_R1_SWITCH_ERROR) {
            err = SDMMC_ERR_INVALID_RESPONSE;
        }
    }

    return err;
}

sdmmc_err_t sdmmc_init_mmc_check_ext_csd(sdmmc_card_t* card)
{
    SDMMC_ASSERT(card->is_mem == 1 && card->rca != 0);

    /*
     * Integrity check required if card switched to HS mode
     * card->max_freq_khz = MIN(card->max_freq_khz, card->host.max_freq_khz)
     * For 26MHz limit background see sdmmc_mmc_enable_hs_mode()
     */
    if (card->max_freq_khz <= SDMMC_FREQ_26M) {
        return SDMMC_OK;
    }

    /* ensure EXT_CSD buffer is available before starting any SD-card operation */
    uint8_t* ext_csd = sdmmc_sys_heap_caps_malloc(EXT_CSD_MMC_SIZE, MALLOC_CAP_DMA);
    if (!ext_csd) {
        SDMMC_LOGE(TAG, "%s: could not allocate ext_csd", __func__);
        return SDMMC_ERR_NO_MEM;
    }

    /* ensure card is in transfer state before read ext_csd */
    uint32_t status;
    sdmmc_err_t err = sdmmc_send_cmd_send_status(card, &status);
    if (err != SDMMC_OK) {
        SDMMC_LOGE(TAG, "%s: send_status returned 0x%x", __func__, err);
        goto out;
    }
    status = ((status & MMC_R1_CURRENT_STATE_MASK) >> MMC_R1_CURRENT_STATE_POS);
    if (status != MMC_R1_CURRENT_STATE_TRAN) {
        SDMMC_LOGE(TAG, "%s: card not in transfer state", __func__);
        err = SDMMC_ERR_INVALID_STATE;
        goto out;
    }

    /* read EXT_CSD to ensure device works fine in HS mode */
    err = sdmmc_mmc_send_ext_csd_data(card, ext_csd, EXT_CSD_MMC_SIZE);
    if (err != SDMMC_OK) {
        SDMMC_LOGE(TAG, "%s: send_ext_csd_data error 0x%x", __func__, err);
        goto out;
    }

    /* EXT_CSD static fields should match the previous read values in sdmmc_card_init */
    if ((card->ext_csd.rev != ext_csd[EXT_CSD_REV]) ||
            (card->ext_csd.sec_feature != ext_csd[EXT_CSD_SEC_FEATURE_SUPPORT])) {
        SDMMC_LOGE(TAG, "%s: Data integrity test fail in HS mode", __func__);
        err = SDMMC_FAIL;
    }

out:
    sdmmc_sys_free(ext_csd);
    return err;
}

uint32_t sdmmc_mmc_get_erase_timeout_ms(const sdmmc_card_t* card, int arg, size_t erase_size_kb)
{
    /* TODO: calculate erase timeout based on ext_csd (trim_timeout) */
    uint32_t timeout_ms = SDMMC_SD_DISCARD_TIMEOUT * erase_size_kb / card->csd.sector_size;
    timeout_ms = MAX(1000, timeout_ms);
    SDMMC_LOGD(TAG, "%s: erase timeout %u s (erasing %u kB, %ums per sector)",
             __func__, timeout_ms / 1000, erase_size_kb, SDMMC_SD_DISCARD_TIMEOUT);
    return timeout_ms;
}
