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
 * Description:  This file is for providing sfud func based on qspi.
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  wangxiaodong 2022/3/15   first commit
 */

#include "fparameters.h"
#include "fqspi_sfud_core.h"
#include "fqspi_flash.h"
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

/* ../port/sfup_port.c */
extern void sfud_log_debug(const char *file, const long line, const char *format, ...);
extern void sfud_log_info(const char *format, ...);

typedef struct 
{
    FQspiCtrl qspi;
} FqspiCore;

FqspiCore fqspi[FQSPI_NUM] = {0} ;

#ifdef SFUD_USING_QSPI
/**
 * read flash data by QSPI
 */
static sfud_err FQspiFastRead(const sfud_spi *spi, uint32_t addr, sfud_qspi_read_cmd_format *qspi_read_cmd_format,
        uint8_t *read_buf, size_t read_size) {

    sfud_err result = SFUD_SUCCESS;
    FQspiCtrl *qspi_p = (FQspiCtrl *)spi->user_data;

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
    /**
     * add your qspi read flash data code
     */

    result = FQspiFlashReadDataConfig(qspi_p, qspi_read_cmd_format->instruction);
  
    FQspiFlashReadData(qspi_p, addr, read_buf, read_size);

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
    FQspiCtrl *qspi_p = (FQspiCtrl *)spi->user_data;
    len = (qspi_p->flash_size > SZ_16M) ? 5 : 4;
    if (write_size && read_size)
    {
        command = write_buf[0];
        switch(command)
        {
            case FQSPI_FLASH_CMD_RDID:
                FQspiFlashSpecialInstruction(qspi_p, command, read_buf, read_size);
                if (SFUD_SUCCESS != ret)
                {
                    SFUD_ERROR("Failed to read id, test result is 0x%x\r\n", ret);
                    return ret;
                }
                break;
                    
            case FQSPI_FLASH_CMD_SFDP:
                if(write_size >= 4)
                {
                    addr = ((write_buf[1] << 16) | (write_buf[2] << 8) | (write_buf[3]));
                }
                FQspiFlashReadSfdp(qspi_p, addr, read_buf, read_size);
                if (SFUD_SUCCESS != ret)
                {
                    SFUD_ERROR("Failed to read SFDP, test result is 0x%x\r\n", ret);
                    return ret;
                }
                break;

            case FQSPI_FLASH_CMD_RDSR1:
                ret = FQspiFlashSpecialInstruction(qspi_p, command, read_buf, 1);

                if (SFUD_SUCCESS != ret)
                {
                    SFUD_ERROR("Failed to read sr1, test result is 0x%x\r\n", ret);
                    return ret;
                }

                break;
            default:
                    break;
        }
    }
    else if(write_size)
    {
        command = write_buf[0];
        switch(command)
        {
            case FQSPI_CMD_ENABLE_RESET:
            case FQSPI_CMD_RESET:
                FQspiFlashWriteReg(qspi_p, command, NULL, 0);
                break; 
            case FQSPI_FLASH_CMD_WREN:
                FQspiFlashEnableWrite(qspi_p);
                break;
            case FQSPI_FLASH_CMD_WRDI:
                FQspiFlashDisableWrite(qspi_p);
                break;
            case FQSPI_FLASH_CMD_WRR:
                FQspiFlashWriteReg(qspi_p, command, NULL, 0);
                break;
            case FQSPI_FLASH_CMD_SE:                
            case FQSPI_FLASH_CMD_4SE:
            case FQSPI_FLASH_CMD_4BE:
            case FQSPI_FLASH_CMD_P4E:
                SFUD_ASSERT(write_size >= len);
                for (i = 1; i < len; i++)
                {
                    addr = ((addr << 8)|(write_buf[i]));
                }
                FQspiFlashErase(qspi_p, command, addr);
                break;   
            case FQSPI_FLASH_CMD_PP:
                /* write Flash data */
                SFUD_ASSERT(write_size > len);
                for (i = 1; i < len; i++)
                {
                    addr = ((addr << 8)|(write_buf[i]));
                }
                ret = FQspiFlashWriteData(qspi_p, command, addr, &write_buf[len], write_size - len);
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
                                size_t read_size) {
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
    if(!memcmp(FQSPI0_SFUD_NAME, spi_p->name, strlen(FQSPI0_SFUD_NAME)))
    {
        FQspiConfig config_p= *FQspiLookupConfig(FQSPI0_ID);
        
        if (FQSPI_SUCCESS != FQspiCfgInitialize(&fqspi[FQSPI0_ID].qspi, &config_p))
        {
            SFUD_ERROR("qspi failed to initialize");
            result = SFUD_ERR_INIT_FAILED;
            return result;
        }
        /* detect connected flash infomation */
        if (FQSPI_SUCCESS != FQspiFlashDetect(&fqspi[FQSPI0_ID].qspi))
        {
            SFUD_ERROR("qspi flash detection failed");
            result = SFUD_ERR_INIT_FAILED;
            return result;
        } 
        else
        {
            SFUD_INFO("qspi flash detection was successful");
        }
        
        flash->spi.wr = FQspiWriteRead;
        flash->spi.user_data = &fqspi[FQSPI0_ID].qspi;
        /* adout 60 seconds timeout */
        flash->retry.times = 60 * 10000;

#ifdef SFUD_USING_QSPI
        flash->spi.qspi_read = FQspiFastRead;
#endif
    }
    else
    {
        result = SFUD_ERR_NOT_FOUND;
    }

    return result;
}


