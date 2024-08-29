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
 * FilePath: fqspi_sfud_core.c
 * Date: 2022-02-10 14:53:44
 * LastEditTime: 2022-02-25 11:47:57
 * Description:  This files is for providing sfud func based on qspi.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  wangxiaodong 2022/8/9   first commit
 */

#include "fparameters.h"
#include "fqspi_sfud_core.h"
#include "sdkconfig.h"
#include "fqspi_os.h"

#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

/* ../port/sfup_port.c */
extern void sfud_log_debug(const char *file, const long line, const char *format, ...);
extern void sfud_log_info(const char *format, ...);

typedef struct
{
    u32 id;
    FFreeRTOSQspi *os_qspi_p;
    u8 cs;
    boolean is_inited;
} FQspiSfudOs;

static FQspiSfudOs sfud_instance =
{
    .id = FQSPI0_ID,
    .os_qspi_p = NULL,
    .cs = FQSPI_CS_0,
    .is_inited = FALSE
};

static FQspiSfudOs fqspi_sfud_os[FQSPI_NUM] = {0} ;


#ifdef SFUD_USING_QSPI
/**
 * read flash data by QSPI
 */
static sfud_err FQspiFastRead(const sfud_spi *spi, uint32_t addr, sfud_qspi_read_cmd_format *qspi_read_cmd_format,
                              uint8_t *read_buf, size_t read_size)
{

    sfud_err result = SFUD_SUCCESS;
    FQspiSfudOs *qspi_sfud_os_p = (FQspiSfudOs *)spi->user_data;

    FFreeRTOSQspiMessage message;
    memset(&message, 0, sizeof(message));

    /* set default read instruction */
#ifdef CONFIG_SFUD_QSPI_READ_MODE_READ
    qspi_read_cmd_format->instruction = SFUD_CMD_READ_DATA;
#endif

#ifdef CONFIG_SFUD_QSPI_READ_MODE_DUAL_READ
    qspi_read_cmd_format->instruction = SFUD_CMD_DUAL_IO_READ_DATA;
#endif

#ifdef CONFIG_SFUD_QSPI_READ_MODE_QUAD_READ
    qspi_read_cmd_format->instruction = SFUD_CMD_QUAD_IO_READ_DATA;
#endif

    /* add your qspi read flash data code */
    message.read_buf = read_buf;
    message.length = read_size;
    message.addr = addr;
    message.cmd = qspi_read_cmd_format->instruction;
    message.cs = qspi_sfud_os_p->cs;
    result = FFreeRTOSQspiTransfer(qspi_sfud_os_p->os_qspi_p, &message);

    return result;
}

#endif /* SFUD_USING_QSPI */

static sfud_err FQspiFlashTransfer(const sfud_spi *spi, const u8 *write_buf,
                                   size_t write_size, u8 *read_buf, size_t read_size)
{
    SFUD_ASSERT(spi);
    sfud_err ret = SFUD_SUCCESS;
    u8 command = 0;
    u32 addr = 0;
    u8 i = 0;
    size_t len = 0;

    FQspiSfudOs *qspi_sfud_os_p = (FQspiSfudOs *)spi->user_data;
    FQspiCtrl *qspi_p = &qspi_sfud_os_p->os_qspi_p->qspi_ctrl;

    FFreeRTOSQspiMessage message;
    memset(&message, 0, sizeof(message));

    len = (qspi_p->flash_size > SZ_16M) ? 5 : 4;
    if (write_size && read_size)
    {
        command = write_buf[0];
        switch (command)
        {
            case SFUD_CMD_JEDEC_ID:
                message.cs = qspi_sfud_os_p->cs;
                message.cmd = command;
                message.read_buf = read_buf;
                message.length = read_size;
                ret = FFreeRTOSQspiTransfer(qspi_sfud_os_p->os_qspi_p, &message);
                if (SFUD_SUCCESS != ret)
                {
                    printf("failed cmd = %#x, test result 0x%x.\r\n", command, ret);
                    return ret;
                }
                break;

            case SFUD_CMD_READ_SFDP_REGISTER:
                if (write_size >= 4)
                {
                    addr = ((write_buf[1] << 16) | (write_buf[2] << 8) | (write_buf[3]));
                }
                message.cs = qspi_sfud_os_p->cs;
                message.cmd = command;
                message.addr = addr;
                message.read_buf = read_buf;
                message.length = read_size;
                ret = FFreeRTOSQspiTransfer(qspi_sfud_os_p->os_qspi_p, &message);
                if (SFUD_SUCCESS != ret)
                {
                    printf("failed cmd = %#x, test result 0x%x.\r\n", command, ret);
                    return ret;
                }
                break;

            case SFUD_CMD_READ_STATUS_REGISTER:
                message.cs = qspi_sfud_os_p->cs;
                message.cmd = command;
                message.read_buf = read_buf;
                message.length = 1;
                ret = FFreeRTOSQspiTransfer(qspi_sfud_os_p->os_qspi_p, &message);
                if (SFUD_SUCCESS != ret)
                {
                    printf("failed cmd = %#x, test result 0x%x.\r\n", command, ret);
                    return ret;
                }
                break;
            default:
                break;
        }
    }
    else if (write_size)
    {
        command = write_buf[0];
        switch (command)
        {
            case SFUD_CMD_ENABLE_RESET:
            case SFUD_CMD_RESET:
            case SFUD_CMD_WRITE_ENABLE:
            case SFUD_CMD_WRITE_DISABLE:
            case SFUD_CMD_WRITE_STATUS_REGISTER:
                message.cs = qspi_sfud_os_p->cs;
                message.cmd = command;
                ret = FFreeRTOSQspiTransfer(qspi_sfud_os_p->os_qspi_p, &message);
                if (SFUD_SUCCESS != ret)
                {
                    printf("failed cmd = %#x, test result 0x%x.\r\n", command, ret);
                    return ret;
                }
                break;

            /* some erase commands are used in SFUD_FLASH_CHIP_TABLE, users need add if not identify */
            case SFUD_CMD_ERASE_CHIP:
            case SFUD_CMD_ERASE_SECTOR:
                SFUD_ASSERT(write_size >= len);
                for (i = 1; i < len; i++)
                {
                    addr = ((addr << 8) | (write_buf[i]));
                }
                message.cs = qspi_sfud_os_p->cs;
                message.cmd = command;
                message.addr = addr;
                ret = FFreeRTOSQspiTransfer(qspi_sfud_os_p->os_qspi_p, &message);
                if (SFUD_SUCCESS != ret)
                {
                    printf("failed cmd = %#x, test result 0x%x.\r\n", command, ret);
                    return ret;
                }
                break;

            case SFUD_CMD_PAGE_PROGRAM:
                /* write Flash data */
                SFUD_ASSERT(write_size > len);
                for (i = 1; i < len; i++)
                {
                    addr = ((addr << 8) | (write_buf[i]));
                }

                message.write_buf = &write_buf[len];
                message.length = write_size - len;
                message.addr = addr;
                message.cmd = command;
                message.cs = qspi_sfud_os_p->cs;
                ret = FFreeRTOSQspiTransfer(qspi_sfud_os_p->os_qspi_p, &message);
                if (SFUD_SUCCESS != ret)
                {
                    printf("failed cmd = %#x, test result 0x%x.\r\n", command, ret);
                    return ret;
                }
                break;
            default:
                break;
        }
    }
    else
    {
        SFUD_ASSERT(0);
    }
    return ret;

}

/**
 * SPI write data then read data
 */
static sfud_err FQspiWriteRead(const sfud_spi *spi, const uint8_t *write_buf,
                               size_t write_size, uint8_t *read_buf,
                               size_t read_size)
{
    sfud_err result = SFUD_SUCCESS;
    uint8_t send_data, read_data;

    if (write_size)
    {
        SFUD_ASSERT(write_buf);
    }

    if (read_size)
    {
        SFUD_ASSERT(read_buf);
    }

    FQspiFlashTransfer(spi, write_buf, write_size, read_buf, read_size);

    return result;
}

sfud_err FQspiProbe(sfud_flash *flash)
{
    sfud_spi *spi_p = &flash->spi;

    sfud_err result = SFUD_SUCCESS;

    FQspiSfudOs *user_data = &sfud_instance;

    if (!memcmp(FQSPI0_SFUD_NAME, spi_p->name, strlen(FQSPI0_SFUD_NAME)))
    {
        if (FALSE == user_data->is_inited)
        {
            user_data->os_qspi_p = FFreeRTOSQspiInit(user_data->id);

            flash->spi.wr = FQspiWriteRead;

            flash->spi.user_data = user_data;

            /* adout 60 seconds timeout */
            flash->retry.times = 60 * 10000;

#ifdef SFUD_USING_QSPI
            flash->spi.qspi_read = FQspiFastRead;
#endif
            user_data->is_inited = TRUE;
        }
    }
    else
    {
        result = SFUD_ERR_NOT_FOUND;
    }

    return result;
}


