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

#include "sdmmc_common.h"

static const char* TAG = "sdmmc_sd";

sdmmc_err_t sdmmc_init_sd_if_cond(sdmmc_card_t* card)
{
    /* SEND_IF_COND (CMD8) command is used to identify SDHC/SDXC cards.
     * SD v1 and non-SD cards will not respond to this command.
     */
    uint32_t host_ocr = get_host_ocr(card->host.io_voltage);
    sdmmc_err_t err = sdmmc_send_cmd_send_if_cond(card, host_ocr);
    if (err == SDMMC_OK) {
        SDMMC_LOGD(TAG, "SDHC/SDXC card");
        host_ocr |= SD_OCR_SDHC_CAP;
    } else if (err == SDMMC_ERR_TIMEOUT) {
        SDMMC_LOGD(TAG, "CMD8 timeout; not an SD v2.00 card");
    } else if (/*host_is_spi(card) &&*/ err == SDMMC_ERR_NOT_SUPPORTED) {
        SDMMC_LOGD(TAG, "CMD8 rejected; not an SD v2.00 card");
    } else {
        SDMMC_LOGE(TAG, "%s: send_if_cond (1) returned 0x%x", __func__, err);
        return err;
    }
    card->ocr = host_ocr;
    return SDMMC_OK;
}

sdmmc_err_t sdmmc_init_sd_blocklen(sdmmc_card_t* card)
{
    /* SDSC cards support configurable data block lengths.
     * We don't use this feature and set the block length to 512 bytes,
     * same as the block length for SDHC cards.
     */
    if ((card->ocr & SD_OCR_SDHC_CAP) == 0) {
        sdmmc_err_t err = sdmmc_send_cmd_set_blocklen(card, &card->csd);
        if (err != SDMMC_OK) {
            SDMMC_LOGE(TAG, "%s: set_blocklen returned 0x%x", __func__, err);
            return err;
        }
    }
    return SDMMC_OK;
}

sdmmc_err_t sdmmc_init_sd_scr(sdmmc_card_t* card)
{
    sdmmc_err_t err;
    /* Get the contents of SCR register: bus width and the version of SD spec
     * supported by the card.
     * In SD mode, this is the first command which uses D0 line. Errors at
     * this step usually indicate connection issue or lack of pull-up resistor.
     */
    err = sdmmc_send_cmd_send_scr(card, &card->scr);
    if (err != SDMMC_OK) {
        SDMMC_LOGE(TAG, "%s: send_scr (1) returned 0x%x", __func__, err);
        return err;
    }

    if ((card->scr.bus_width & SCR_SD_BUS_WIDTHS_4BIT)
            && (card->host.flags & SDMMC_HOST_FLAG_4BIT)) {
        card->log_bus_width = 2;
    } else {
        card->log_bus_width = 0;
    }
    SDMMC_LOGD(TAG, "log_bus_width=%d", card->log_bus_width);
    return SDMMC_OK;
}

sdmmc_err_t sdmmc_init_sd_ssr(sdmmc_card_t* card)
{
    sdmmc_err_t err = SDMMC_OK;
    /* Get the contents of SSR register: SD additional information
     * ACMD13 to read 512byte SD status information
     */
    uint32_t* sd_ssr = sdmmc_sys_heap_caps_malloc(/*1 * SD_SSR_SIZE*/512U, MALLOC_CAP_DMA);
    if (!sd_ssr) {
        SDMMC_LOGE(TAG, "%s: could not allocate sd_ssr", __func__);
        return SDMMC_ERR_NO_MEM;
    }

    sdmmc_command_t cmd = {
        .data = sd_ssr,
        .datalen = SD_SSR_SIZE,
        .blklen = SD_SSR_SIZE,
        .opcode = SD_APP_SD_STATUS,
        .arg = 0,
        .flags = SCF_CMD_ADTC | SCF_RSP_R1 | SCF_CMD_READ
    };

    // read SD status register
    err = sdmmc_send_app_cmd(card, &cmd);
    if (err != SDMMC_OK) {
        sdmmc_sys_free(sd_ssr);
        SDMMC_LOGE(TAG, "%s: sdmmc_send_cmd returned 0x%x", __func__, err);
        return err;
    }

    err = sdmmc_decode_ssr(sd_ssr, &card->ssr);
    if (err != SDMMC_OK) {
        SDMMC_LOGE(TAG, "%s: error sdmmc_decode_scr returned 0x%x", __func__, err);
    }

    sdmmc_sys_free(sd_ssr);
    return err;
}

sdmmc_err_t sdmmc_init_sd_bus_width(sdmmc_card_t* card)
{
    int width = 1;
    if (card->log_bus_width == 2) {
        width = 4;
    } else if (card->log_bus_width == 3) {
        width = 8;
    }
    sdmmc_err_t err = sdmmc_send_cmd_set_bus_width(card, width);
    if (err != SDMMC_OK) {
        SDMMC_LOGE(TAG, "set_bus_width failed (0x%x)", err);
        return err;
    }
    return SDMMC_OK;
}

sdmmc_err_t sdmmc_init_sd_wait_data_ready(sdmmc_card_t* card)
{
    /* Wait for the card to be ready for data transfers */
    uint32_t status = 0;
    uint32_t count = 0;
    while (!host_is_spi(card) && !(status & MMC_R1_READY_FOR_DATA)) {
        // TODO: add some timeout here
        sdmmc_err_t err = sdmmc_send_cmd_send_status(card, &status);
        if (err != SDMMC_OK) {
            return err;
        }
        if (++count % 16 == 0) {
            SDMMC_LOGV(TAG, "waiting for card to become ready (%d)", count);
        }
    }
    return SDMMC_OK;
}

sdmmc_err_t sdmmc_send_cmd_switch_func(sdmmc_card_t* card,
        uint32_t mode, uint32_t group, uint32_t function,
        sdmmc_switch_func_rsp_t* resp)
{
    if (card->scr.sd_spec < SCR_SD_SPEC_VER_1_10 ||
        ((card->csd.card_command_class & SD_CSD_CCC_SWITCH) == 0)) {
            return SDMMC_ERR_NOT_SUPPORTED;
    }

    if (group == 0 ||
        group > SD_SFUNC_GROUP_MAX ||
        function > SD_SFUNC_FUNC_MAX) {
        return SDMMC_ERR_INVALID_ARG;
    }

    if (mode > 1) {
        return SDMMC_ERR_INVALID_ARG;
    }

    uint32_t group_shift = (group - 1) << 2;
    /* all functions which should not be affected are set to 0xf (no change) */
    uint32_t other_func_mask = (0x00ffffff & ~(0xf << group_shift));
    uint32_t func_val = (function << group_shift) | other_func_mask;

    sdmmc_command_t cmd = {
            .opcode = MMC_SWITCH,
            .flags = SCF_CMD_ADTC | SCF_CMD_READ | SCF_RSP_R1,
            .blklen = sizeof(sdmmc_switch_func_rsp_t),
            .data = resp->data,
            .datalen = sizeof(sdmmc_switch_func_rsp_t),
            .arg = (!!mode << 31) | func_val
    };

    sdmmc_err_t err = sdmmc_send_cmd(card, &cmd);
    if (err != SDMMC_OK) {
        SDMMC_LOGE(TAG, "%s: sdmmc_send_cmd returned 0x%x", __func__, err);
        return err;
    }
    sdmmc_flip_byte_order(resp->data, sizeof(sdmmc_switch_func_rsp_t));
    uint32_t resp_ver = SD_SFUNC_VER(resp->data);
    if (resp_ver == 0) {
        /* busy response is never sent */
    } else if (resp_ver == 1) {
        if (SD_SFUNC_BUSY(resp->data, group) & (1 << function)) {
            SDMMC_LOGD(TAG, "%s: response indicates function %d:%d is busy",
                    __func__, group, function);
            return SDMMC_ERR_INVALID_STATE;
        }
    } else {
        SDMMC_LOGD(TAG, "%s: got an invalid version of SWITCH_FUNC response: 0x%02x",
                __func__, resp_ver);
        return SDMMC_ERR_INVALID_RESPONSE;
    }
    return SDMMC_OK;
}

sdmmc_err_t sdmmc_enable_hs_mode(sdmmc_card_t* card)
{
    /* This will determine if the card supports SWITCH_FUNC command,
     * and high speed mode. If the cards supports both, this will enable
     * high speed mode at the card side.
     */
    if (card->scr.sd_spec < SCR_SD_SPEC_VER_1_10 ||
        ((card->csd.card_command_class & SD_CSD_CCC_SWITCH) == 0)) {
            return SDMMC_ERR_NOT_SUPPORTED;
    }
    sdmmc_switch_func_rsp_t* response = (sdmmc_switch_func_rsp_t*)
            sdmmc_sys_heap_caps_malloc(sizeof(*response), MALLOC_CAP_DMA);
    if (response == NULL) {
        return SDMMC_ERR_NO_MEM;
    }

    sdmmc_err_t err = sdmmc_send_cmd_switch_func(card, 0, SD_ACCESS_MODE, 0, response);
    if (err != SDMMC_OK) {
        SDMMC_LOGD(TAG, "%s: sdmmc_send_cmd_switch_func (1) returned 0x%x", __func__, err);
        goto out;
    }
    uint32_t supported_mask = SD_SFUNC_SUPPORTED(response->data, 1);
    if ((supported_mask & BIT(SD_ACCESS_MODE_SDR25)) == 0) {
        err = SDMMC_ERR_NOT_SUPPORTED;
        goto out;
    }
    err = sdmmc_send_cmd_switch_func(card, 1, SD_ACCESS_MODE, SD_ACCESS_MODE_SDR25, response);
    if (err != SDMMC_OK) {
        SDMMC_LOGD(TAG, "%s: sdmmc_send_cmd_switch_func (2) returned 0x%x", __func__, err);
        goto out;
    }

out:
    sdmmc_sys_free(response);
    return err;
}

sdmmc_err_t sdmmc_enable_hs_mode_and_check(sdmmc_card_t* card)
{
    /* All cards should support at least default speed */
    card->max_freq_khz = SDMMC_FREQ_DEFAULT;
    if (card->host.max_freq_khz <= card->max_freq_khz) {
        /* Host is configured to use low frequency, don't attempt to switch */
        card->max_freq_khz = card->host.max_freq_khz;
        return SDMMC_OK;
    }

    /* Try to enabled HS mode */
    sdmmc_err_t err = sdmmc_enable_hs_mode(card);
    if (err != SDMMC_OK) {
        return err;
    }
    /* HS mode has been enabled on the card.
     * Read CSD again, it should now indicate that the card supports
     * 50MHz clock.
     * Since SEND_CSD is allowed only in standby mode, and the card is currently in data transfer
     * mode, deselect the card first, then get the CSD, then select the card again. This step is
     * not required in SPI mode, since CMD7 (select_card) is not supported.
     */
    const bool is_spi = host_is_spi(card);
    if (!is_spi) {
        err = sdmmc_send_cmd_select_card(card, 0);
        if (err != SDMMC_OK) {
            SDMMC_LOGE(TAG, "%s: select_card (1) returned 0x%x", __func__, err);
            return err;
        }
    }
    err = sdmmc_send_cmd_send_csd(card, &card->csd);
    if (err != SDMMC_OK) {
        SDMMC_LOGE(TAG, "%s: send_csd returned 0x%x", __func__, err);
        return err;
    }
    if (!is_spi) {
        err = sdmmc_send_cmd_select_card(card, card->rca);
        if (err != SDMMC_OK) {
            SDMMC_LOGE(TAG, "%s: select_card (2) returned 0x%x", __func__, err);
            return err;
        }
    }

    if (card->csd.tr_speed != 50000000) {
        SDMMC_LOGW(TAG, "unexpected: after enabling HS mode, tr_speed=%d", card->csd.tr_speed);
        return SDMMC_ERR_NOT_SUPPORTED;
    }

    card->max_freq_khz = MIN(card->host.max_freq_khz, SDMMC_FREQ_HIGHSPEED);
    return SDMMC_OK;
}

sdmmc_err_t sdmmc_check_scr(sdmmc_card_t* card)
{
    /* If frequency switch has been performed, read SCR register one more time
     * and compare the result with the previous one. Use this simple check as
     * an indicator of potential signal integrity issues.
     */
    sdmmc_scr_t scr_tmp = { 0 };
    sdmmc_err_t err = sdmmc_send_cmd_send_scr(card, &scr_tmp);
    if (err != SDMMC_OK) {
        SDMMC_LOGE(TAG, "%s: send_scr returned 0x%x", __func__, err);
        return err;
    }
    if (memcmp(&card->scr, &scr_tmp, sizeof(scr_tmp)) != 0) {
        SDMMC_LOGE(TAG, "got corrupted data after increasing clock frequency");
        return SDMMC_ERR_INVALID_RESPONSE;
    }
    return SDMMC_OK;
}

sdmmc_err_t sdmmc_init_spi_crc(sdmmc_card_t* card)
{
    /* In SD mode, CRC checks of data transfers are mandatory and performed
     * by the hardware. In SPI mode, CRC16 of data transfers is optional and
     * needs to be enabled.
     */
    SDMMC_ASSERT(host_is_spi(card));
    sdmmc_err_t err = sdmmc_send_cmd_crc_on_off(card, true);
    if (err != SDMMC_OK) {
        SDMMC_LOGE(TAG, "%s: sdmmc_send_cmd_crc_on_off returned 0x%x", __func__, err);
        return err;
    }
    return SDMMC_OK;
}

sdmmc_err_t sdmmc_decode_cid(sdmmc_response_t resp, sdmmc_cid_t* out_cid)
{
    SDMMC_LOGD(TAG, "CID info:");

    out_cid->mfg_id = SD_CID_MID(resp);
    out_cid->oem_id = SD_CID_OID(resp);
    SD_CID_PNM_CPY(resp, out_cid->name);
    out_cid->revision = SD_CID_REV(resp);
    out_cid->serial = SD_CID_PSN(resp);
    out_cid->date = SD_CID_MDT(resp);

    SDMMC_LOGD(TAG, "   name: %s", out_cid->name);
    SDMMC_LOGD(TAG, "   mfg_id: %d", out_cid->mfg_id);
    SDMMC_LOGD(TAG, "   oem_id: %d", out_cid->oem_id);
    SDMMC_LOGD(TAG, "   revision: %d", out_cid->revision);
    SDMMC_LOGD(TAG, "   serial: %d", out_cid->serial);
    SDMMC_LOGD(TAG, "   date: %d", out_cid->date);
    return SDMMC_OK;
}

sdmmc_err_t sdmmc_decode_csd(sdmmc_response_t response, sdmmc_csd_t* out_csd)
{
    SDMMC_LOGD(TAG, "CSD info:");
    out_csd->csd_ver = SD_CSD_CSDVER(response);
    switch (out_csd->csd_ver) {
    case SD_CSD_CSDVER_2_0:
        out_csd->capacity = SD_CSD_V2_CAPACITY(response);
        out_csd->read_block_len = SD_CSD_V2_BL_LEN;
        SDMMC_LOGD(TAG, "   csd structure: 2.0");  
        break;
    case SD_CSD_CSDVER_1_0:
        out_csd->capacity = SD_CSD_CAPACITY(response);
        out_csd->read_block_len = SD_CSD_READ_BL_LEN(response);
        SDMMC_LOGD(TAG, "   csd structure: 1.0");  
        break;
    default:
        SDMMC_LOGE(TAG, "unknown SD CSD structure version 0x%x", out_csd->csd_ver);
        return SDMMC_ERR_NOT_SUPPORTED;
    }

    out_csd->card_command_class = SD_CSD_CCC(response);
    int read_bl_size = 1 << out_csd->read_block_len;
    out_csd->sector_size = MIN(read_bl_size, 512);
    if (out_csd->sector_size < read_bl_size) {
        out_csd->capacity *= read_bl_size / out_csd->sector_size;
    }
    int speed = SD_CSD_SPEED(response);
    if (speed == SD_CSD_SPEED_50_MHZ) {
        out_csd->tr_speed = 50000000;
    } else {
        out_csd->tr_speed = 25000000;
    }

    SDMMC_LOGD(TAG, "   class: 0x%x", out_csd->card_command_class);
    SDMMC_LOGD(TAG, "       basic: %d", !!(out_csd->card_command_class & SD_CSD_CCC_BASIC));
    SDMMC_LOGD(TAG, "       block read: %d", !!(out_csd->card_command_class & SD_CSD_CCC_BR));
    SDMMC_LOGD(TAG, "       block write: %d", !!(out_csd->card_command_class & SD_CSD_CCC_BW));
    SDMMC_LOGD(TAG, "       erase: %d", !!(out_csd->card_command_class & SD_CSD_CCC_ERASE));
    SDMMC_LOGD(TAG, "       write protection: %d", !!(out_csd->card_command_class & SD_CSD_CCC_WP));
    SDMMC_LOGD(TAG, "       lock card: %d", !!(out_csd->card_command_class & SD_CSD_CCC_LC));
    SDMMC_LOGD(TAG, "       app spec: %d", !!(out_csd->card_command_class & SD_CSD_CCC_AS));
    SDMMC_LOGD(TAG, "       i/o mode: %d", !!(out_csd->card_command_class & SD_CSD_CCC_IOM));
    SDMMC_LOGD(TAG, "       switch: %d", !!(out_csd->card_command_class & SD_CSD_CCC_SWITCH));
    SDMMC_LOGD(TAG, "   capacity: %ld Byte", out_csd->capacity);
    SDMMC_LOGD(TAG, "   read_block_len: %d Byte", (2 << out_csd->read_block_len));
    SDMMC_LOGD(TAG, "   sector_size: %d Byte", out_csd->sector_size);
    SDMMC_LOGD(TAG, "   tr_speed: %d Hz", out_csd->tr_speed);     
    return SDMMC_OK;
}

sdmmc_err_t sdmmc_decode_scr(uint32_t *raw_scr, sdmmc_scr_t* out_scr)
{
    sdmmc_response_t resp = { 0 };
    resp[1] = __builtin_bswap32(raw_scr[0]);
    resp[0] = __builtin_bswap32(raw_scr[1]);
    int ver = SCR_STRUCTURE(resp);
    if (ver != 0) {
        SDMMC_LOGE(TAG, "decode scr failed at %s raw_scr: 0x%x ver: 0x%x",
                   __func__, *raw_scr, ver);
        return SDMMC_ERR_NOT_SUPPORTED;
    }
    out_scr->sd_spec = SCR_SD_SPEC(resp);
    out_scr->erase_mem_state = SCR_DATA_STAT_AFTER_ERASE(resp);
    out_scr->bus_width = SCR_SD_BUS_WIDTHS(resp);
    SDMMC_LOGD(TAG, "sd_spec=%d, bus_width=%d", out_scr->sd_spec, out_scr->bus_width);
    return SDMMC_OK;
}

static const uint32_t s_au_to_size_kb[] = {
    0, 16, 32, 64,
    128, 256, 512, 1024,
    2 * 1024, 4 * 1024,
    8 * 1024, 12 * 1024,
    16 * 1024, 24 * 1024,
    32 * 1024, 64 * 1024
};
_Static_assert(sizeof(s_au_to_size_kb)/sizeof(s_au_to_size_kb[0]) == 16, "invalid number of elements in s_au_to_size_kb");

sdmmc_err_t sdmmc_decode_ssr(uint32_t *raw_ssr, sdmmc_ssr_t* out_ssr)
{
    uint32_t ssr[(SD_SSR_SIZE/sizeof(uint32_t))] = { 0 };
    size_t j = (SD_SSR_SIZE/sizeof(uint32_t) - 1);

    for(size_t i = 0; i < (SD_SSR_SIZE/sizeof(uint32_t)); i++) {
        ssr[j - i] = __builtin_bswap32(raw_ssr[i]);
    }

    out_ssr->cur_bus_width = SSR_DAT_BUS_WIDTH(ssr);
    out_ssr->discard_support = SSR_DISCARD_SUPPORT(ssr);
    out_ssr->fule_support = SSR_FULE_SUPPORT(ssr);
    uint32_t au = SSR_AU_SIZE(ssr);
    out_ssr->alloc_unit_kb = s_au_to_size_kb[au];
    out_ssr->erase_timeout = SSR_ERASE_TIMEOUT(ssr);
    out_ssr->erase_size_au = SSR_ERASE_SIZE(ssr);
    out_ssr->erase_offset = SSR_ERASE_OFFSET(ssr);

    return SDMMC_OK;
}

uint32_t sdmmc_sd_get_erase_timeout_ms(const sdmmc_card_t* card, int arg, size_t erase_size_kb)
{
    if (arg == SDMMC_SD_DISCARD_ARG) {
        return SDMMC_SD_DISCARD_TIMEOUT;
    } else if (arg == SDMMC_SD_ERASE_ARG) {
        if (card->ssr.alloc_unit_kb != 0 &&
                card->ssr.erase_size_au != 0 &&
                card->ssr.erase_timeout != 0 &&
                card->ssr.erase_offset != 0) {
            /* Card supports erase timeout estimation. See the erase timeout equation in SD spec. */
            uint32_t timeout_sec = card->ssr.erase_offset +
                card->ssr.erase_timeout * (erase_size_kb + card->ssr.alloc_unit_kb - 1) /
                    (card->ssr.erase_size_au * card->ssr.alloc_unit_kb);
            SDMMC_LOGD(TAG, "%s: erase timeout %u s (erasing %u kB, ES=%u, ET=%u, EO=%u, AU=%u kB)",
                     __func__, timeout_sec, erase_size_kb, card->ssr.erase_size_au,
                     card->ssr.erase_timeout, card->ssr.erase_offset, card->ssr.alloc_unit_kb);
            return timeout_sec * 1000;
        } else {
            uint32_t timeout_ms = SDMMC_SD_DISCARD_TIMEOUT * erase_size_kb / card->csd.sector_size;
            timeout_ms = MAX(1000, timeout_ms);
            SDMMC_LOGD(TAG, "%s: erase timeout %u s (erasing %u kB, %ums per sector)",
                     __func__, timeout_ms / 1000, erase_size_kb, SDMMC_SD_DISCARD_TIMEOUT);
            return timeout_ms;
        }
    } else {
        SDMMC_ASSERT(false && "unexpected SD erase argument");
        return 0;
    }
}
