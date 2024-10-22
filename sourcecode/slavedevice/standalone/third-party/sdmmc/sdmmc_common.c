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

static const char* TAG = "sdmmc_common";

sdmmc_err_t sdmmc_init_ocr(sdmmc_card_t* card)
{
    sdmmc_err_t err;
    /* In SPI mode, READ_OCR (CMD58) command is used to figure out which voltage
     * ranges the card can support. This step is skipped since 1.8V isn't
     * supported on the ESP32.
     */

    uint32_t host_ocr = get_host_ocr(card->host.io_voltage);
    if ((card->ocr & SD_OCR_SDHC_CAP) != 0) {
        host_ocr |= SD_OCR_SDHC_CAP;
    }
    /* Send SEND_OP_COND (ACMD41) command to the card until it becomes ready. */
    err = sdmmc_send_cmd_send_op_cond(card, host_ocr, &card->ocr);

    /* If time-out, re-try send_op_cond as MMC */
    if (err == SDMMC_ERR_TIMEOUT /*&& !host_is_spi(card)*/) {
        SDMMC_LOGD(TAG, "send_op_cond timeout, trying MMC");
        card->is_mmc = 1;
        err = sdmmc_send_cmd_send_op_cond(card, host_ocr, &card->ocr);
    }

    if (err != SDMMC_OK) {
        SDMMC_LOGE(TAG, "%s: send_op_cond (1) returned 0x%x", __func__, err);
        return err;
    }
    if (host_is_spi(card)) {
        err = sdmmc_send_cmd_read_ocr(card, &card->ocr);
        if (err != SDMMC_OK) {
            SDMMC_LOGE(TAG, "%s: read_ocr returned 0x%x", __func__, err);
            return err;
        }
    }
    SDMMC_LOGD(TAG, "host_ocr=0x%x card_ocr=0x%x", host_ocr, card->ocr);

    /* Clear all voltage bits in host's OCR which the card doesn't support.
     * Don't touch CCS bit because in SPI mode cards don't report CCS in ACMD41
     * response.
     */
    host_ocr &= (card->ocr | (~SD_OCR_VOL_MASK));
    SDMMC_LOGD(TAG, "sdmmc_card_init: host_ocr=%08x, card_ocr=%08x", host_ocr, card->ocr);
    return SDMMC_OK;
}

sdmmc_err_t sdmmc_init_cid(sdmmc_card_t* card)
{
    sdmmc_err_t err;
    sdmmc_response_t raw_cid;
    if (!host_is_spi(card)) {
        err = sdmmc_send_cmd_all_send_cid(card, &raw_cid);
        if (err != SDMMC_OK) {
            SDMMC_LOGE(TAG, "%s: all_send_cid returned 0x%x", __func__, err);
            return err;
        }
        if (!card->is_mmc) {
            err = sdmmc_decode_cid(raw_cid, &card->cid);
            if (err != SDMMC_OK) {
                SDMMC_LOGE(TAG, "%s: decoding CID failed (0x%x)", __func__, err);
                return err;
            }
        } else {
            /* For MMC, need to know CSD to decode CID. But CSD can only be read
             * in data transfer mode, and it is not possible to read CID in data
             * transfer mode. We temporiliy store the raw cid and do the
             * decoding after the RCA is set and the card is in data transfer
             * mode.
             */
            memcpy(card->raw_cid, raw_cid, sizeof(sdmmc_response_t));
        }
    } else {
        err = sdmmc_send_cmd_send_cid(card, &card->cid);
        if (err != SDMMC_OK) {
            SDMMC_LOGE(TAG, "%s: send_cid returned 0x%x", __func__, err);
            return err;
        }
    }
    return SDMMC_OK;
}

sdmmc_err_t sdmmc_init_rca(sdmmc_card_t* card)
{
    sdmmc_err_t err;
    err = sdmmc_send_cmd_set_relative_addr(card, &card->rca);
    if (err != SDMMC_OK) {
        SDMMC_LOGE(TAG, "%s: set_relative_addr returned 0x%x", __func__, err);
        return err;
    }
    return SDMMC_OK;
}

sdmmc_err_t sdmmc_init_mmc_decode_cid(sdmmc_card_t* card)
{
    sdmmc_err_t err;
    sdmmc_response_t raw_cid;
    memcpy(raw_cid, card->raw_cid, sizeof(raw_cid));
    err = sdmmc_mmc_decode_cid(card->csd.mmc_ver, raw_cid, &card->cid);
    if (err != SDMMC_OK) {
        SDMMC_LOGE(TAG, "%s: decoding CID failed (0x%x)", __func__, err);
        return err;
    }
    return SDMMC_OK;
}

sdmmc_err_t sdmmc_init_csd(sdmmc_card_t* card)
{
    SDMMC_ASSERT(card->is_mem == 1);
    /* Get and decode the contents of CSD register. Determine card capacity. */
    sdmmc_err_t err = sdmmc_send_cmd_send_csd(card, &card->csd);
    if (err != SDMMC_OK) {
        SDMMC_LOGE(TAG, "%s: send_csd returned 0x%x", __func__, err);
        return err;
    }
    const size_t max_sdsc_capacity = UINT32_MAX / card->csd.sector_size + 1;
    if (!(card->ocr & SD_OCR_SDHC_CAP) &&
         (size_t)card->csd.capacity > max_sdsc_capacity) {
        SDMMC_LOGW(TAG, "%s: SDSC card reports capacity=%u. Limiting to %u.",
                __func__, card->csd.capacity, max_sdsc_capacity);
        card->csd.capacity = max_sdsc_capacity;
    }
    return SDMMC_OK;
}

sdmmc_err_t sdmmc_init_select_card(sdmmc_card_t* card)
{
    SDMMC_ASSERT(!host_is_spi(card));
    sdmmc_err_t err = sdmmc_send_cmd_select_card(card, card->rca);
    if (err != SDMMC_OK) {
        SDMMC_LOGE(TAG, "%s: select_card returned 0x%x", __func__, err);
        return err;
    }
    return SDMMC_OK;
}

sdmmc_err_t sdmmc_init_card_hs_mode(sdmmc_card_t* card)
{
    sdmmc_err_t err = SDMMC_ERR_NOT_SUPPORTED;
    if (card->is_mem && !card->is_mmc) {
        err = sdmmc_enable_hs_mode_and_check(card);
    } else if (card->is_sdio) {
        err = sdmmc_io_enable_hs_mode(card);
    } else if (card->is_mmc){
        err = sdmmc_mmc_enable_hs_mode(card);
    }
    if (err == SDMMC_ERR_NOT_SUPPORTED) {
        SDMMC_LOGW(TAG, "%s: host supports HS mode, but card doesn't", __func__);
        card->max_freq_khz = SDMMC_FREQ_DEFAULT;
    } else if (err != SDMMC_OK) {
        return err;
    }
    return SDMMC_OK;
}

sdmmc_err_t sdmmc_init_host_bus_width(sdmmc_card_t* card)
{
    int bus_width = 1;

    if ((card->host.flags & SDMMC_HOST_FLAG_4BIT) &&
        (card->log_bus_width == 2)) {
        bus_width = 4;
    } else if ((card->host.flags & SDMMC_HOST_FLAG_8BIT) &&
        (card->log_bus_width == 3)) {
        bus_width = 8;
    }
    SDMMC_LOGD(TAG, "%s: using %d-bit bus", __func__, bus_width);
    if (bus_width > 1) {
        sdmmc_err_t err = (*card->host.set_bus_width)(card->host.slot, bus_width);
        if (err != SDMMC_OK) {
            SDMMC_LOGE(TAG, "host.set_bus_width failed (0x%x)", err);
            return err;
        }
    }
    return SDMMC_OK;
}

sdmmc_err_t sdmmc_init_host_frequency(sdmmc_card_t* card)
{
    if (card->max_freq_khz > card->host.max_freq_khz){
        SDMMC_LOGE(TAG, "max freq of card (%dKHz) > host (%dKHz)", card->max_freq_khz, card->host.max_freq_khz);
        return SDMMC_FAIL;
    }

    if (card->max_freq_khz > SDMMC_FREQ_PROBING) {
        sdmmc_err_t err = (*card->host.set_card_clk)(card->host.slot, card->max_freq_khz);
        if (err != SDMMC_OK) {
            SDMMC_LOGE(TAG, "failed to switch bus frequency (0x%x)", err);
            return err;
        }

        err = (*card->host.get_real_freq)(card->host.slot, &(card->real_freq_khz));
        if (err != SDMMC_OK) {
            SDMMC_LOGE(TAG, "failed to get real working frequency (0x%x)", err);
            return err;
        }
    }

    if (card->is_ddr) {
        if (card->host.set_bus_ddr_mode == NULL) {
            SDMMC_LOGE(TAG, "host doesn't support DDR mode or voltage switching");
            return SDMMC_ERR_NOT_SUPPORTED;
        }
        sdmmc_err_t err = (*card->host.set_bus_ddr_mode)(card->host.slot, true);
        if (err != SDMMC_OK) {
            SDMMC_LOGE(TAG, "failed to switch bus to DDR mode (0x%x)", err);
            return err;
        }
    }
    return SDMMC_OK;
}

void sdmmc_flip_byte_order(uint32_t* response, size_t size)
{
    SDMMC_ASSERT(size % (2 * sizeof(uint32_t)) == 0);
    const size_t n_words = size / sizeof(uint32_t);
    for (int i = 0; i < (int)n_words / 2; ++i) {
        uint32_t left = __builtin_bswap32(response[i]);
        uint32_t right = __builtin_bswap32(response[n_words - i - 1]);
        response[i] = right;
        response[n_words - i - 1] = left;
    }
}

void sdmmc_card_print_info(const sdmmc_card_t* card)
{
    bool print_scr = false;
    bool print_csd = false;
    const char* type;

    SDMMC_PRINTF("Name: %s\n", card->cid.name);

    if (card->is_sdio) {
        type = "SDIO";
        print_scr = true;
        print_csd = true;
    } else if (card->is_mmc) {
        type = "MMC";
        print_csd = true;
    } else {
        type = (card->ocr & SD_OCR_SDHC_CAP) ? "SDHC/SDXC" : "SDSC";
        print_csd = true;
    }
    SDMMC_PRINTF("Type: %s\n", type);

    if (card->real_freq_khz == 0) {
        SDMMC_PRINTF("Speed: N/A\n");
    } else {
        const char *freq_unit = card->real_freq_khz < 1000 ? "kHz" : "MHz";
        const float freq = card->real_freq_khz < 1000 ? card->real_freq_khz : card->real_freq_khz / 1000.0;
        const char *max_freq_unit = card->max_freq_khz < 1000 ? "kHz" : "MHz";
        const float max_freq = card->max_freq_khz < 1000 ? card->max_freq_khz : card->max_freq_khz / 1000.0;
        SDMMC_PRINTF("Speed: %.0f %s (limit: %.2f %s)%s\n", freq, freq_unit, max_freq, max_freq_unit, card->is_ddr ? ", DDR" : "");
    }

    SDMMC_PRINTF("Size: %.0fMB\n", ((double) card->csd.capacity) * (double)card->csd.sector_size / (double)(1024ULL * 1024ULL));

    if (print_csd) {
        SDMMC_PRINTF("CSD: ver=%d, sector_size=%d, capacity=%d read_bl_len=%d\n",
                (card->is_mmc ? card->csd.csd_ver : card->csd.csd_ver + 1),
                card->csd.sector_size, card->csd.capacity, card->csd.read_block_len);
        if (card->is_mmc) {
            SDMMC_PRINTF("EXT CSD: bus_width=%d\n", (1 << card->log_bus_width));
        } else if (!card->is_sdio){ // make sure card is SD
            SDMMC_PRINTF("SSR: bus_width=%d\n", (card->ssr.cur_bus_width ? 4 : 1));
        }
    }
    if (print_scr) {
        SDMMC_PRINTF("SCR: sd_spec=%d, bus_width=%d\n", card->scr.sd_spec, card->scr.bus_width);
    }
}

void sdmmc_get_card_info(const sdmmc_card_t* card, sdmmc_card_info_t *out_info)
{
    SDMMC_ASSERT(card && out_info);

    memset(out_info, 0, sizeof(*out_info));

    out_info->is_mem = card->is_mem;
    out_info->is_mmc = card->is_mmc;
    out_info->is_sdio = card->is_sdio;
    out_info->is_ddr = card->is_ddr;

    if (card->is_mem)
        out_info->is_sdhc = !!(card->ocr & SD_OCR_SDHC_CAP);

    out_info->sector_size = card->csd.sector_size;
    out_info->sector_num = card->csd.capacity;
}

sdmmc_err_t sdmmc_fix_host_flags(sdmmc_card_t* card)
{
    const uint32_t width_1bit = SDMMC_HOST_FLAG_1BIT;
    const uint32_t width_4bit = SDMMC_HOST_FLAG_4BIT;
    const uint32_t width_8bit = SDMMC_HOST_FLAG_8BIT;
    const uint32_t width_mask = width_1bit | width_4bit | width_8bit;

    int slot_bit_width = card->host.get_bus_width(card->host.slot);
    if (slot_bit_width == 1 &&
            (card->host.flags & (width_4bit | width_8bit))) {
        card->host.flags &= ~width_mask;
        card->host.flags |= width_1bit;
    } else if (slot_bit_width == 4 && (card->host.flags & width_8bit)) {
        if ((card->host.flags & width_4bit) == 0) {
            SDMMC_LOGW(TAG, "slot width set to 4, but host flags don't have 4 line mode enabled; using 1 line mode");
            card->host.flags &= ~width_mask;
            card->host.flags |= width_1bit;
        } else {
            card->host.flags &= ~width_mask;
            card->host.flags |= width_4bit;
        }
    }

    SDMMC_LOGD(TAG, "card@%p host flags = 0x%x", card, card->host.flags);
    SDMMC_LOGD(TAG, "    1-bit: %d", !!(card->host.flags & SDMMC_HOST_FLAG_1BIT));
    SDMMC_LOGD(TAG, "    4-bit: %d", !!(card->host.flags & SDMMC_HOST_FLAG_4BIT));
    SDMMC_LOGD(TAG, "    8-bit: %d", !!(card->host.flags & SDMMC_HOST_FLAG_8BIT));
    SDMMC_LOGD(TAG, "    spi: %d", !!(card->host.flags & SDMMC_HOST_FLAG_SPI));
    SDMMC_LOGD(TAG, "    ddr: %d", !!(card->host.flags & SDMMC_HOST_FLAG_DDR));

    return SDMMC_OK;
}

uint32_t sdmmc_get_erase_timeout_ms(const sdmmc_card_t* card, int arg, size_t erase_size_kb)
{
    if (card->is_mmc) {
        return sdmmc_mmc_get_erase_timeout_ms(card, arg, erase_size_kb);
    } else {
        return sdmmc_sd_get_erase_timeout_ms(card, arg, erase_size_kb);
    }
}
