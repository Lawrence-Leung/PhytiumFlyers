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
 * FilePath: ymodem.c
 * Date: 2022-02-10 14:53:45
 * LastEditTime: 2022-02-25 11:48:04
 * Description:  This file is for ymodem Protocol
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2022/6/4     first release
 */

#include <string.h>
#include "fassert.h"
#include "fdebug.h"
#include "fearly_uart.h"
#include "ymodem.h"

#define FYMODEM_DEBUG_TAG "FYMODEM"
#define FYMODEM_ERROR(format, ...)   FT_DEBUG_PRINT_E(FYMODEM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FYMODEM_INFO(format, ...)    FT_DEBUG_PRINT_I(FYMODEM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FYMODEM_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FYMODEM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FYMODEM_WARN(format, ...)    FT_DEBUG_PRINT_W(FYMODEM_DEBUG_TAG, format, ##__VA_ARGS__)

#define YMODEM_USING_CRC_TABLE

#ifdef YMODEM_USING_CRC_TABLE
static const u16 ccitt_table[256] =
{
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};
static u16 CRC16(unsigned char *q, int len)
{
    u16 crc = 0;

    while (len-- > 0)
        crc = (crc << 8) ^ ccitt_table[((crc >> 8) ^ *q++) & 0xff];
    return crc;
}
#else
static u16 CRC16(unsigned char *q, int len)
{
    u16 crc;
    char i;

    crc = 0;
    while (--len >= 0)
    {
        crc = crc ^ (int) * q++ << 8;
        i = 8;
        do
        {
            if (crc & 0x8000)
                crc = crc << 1 ^ 0x1021;
            else
                crc = crc << 1;
        }
        while (--i);
    }

    return (crc);
}
#endif


uint8_t aPacketData[PACKET_1K_SIZE + PACKET_DATA_INDEX + PACKET_TRAILER_SIZE];

/**
  * @brief  Convert an Integer to a string
  * @param  p_str: The string output pointer
  * @param  intnum: The integer to be converted
  * @retval None
  */
void Int2Str(u8 *p_str, u32 intnum)
{
  u32 i, divider = 1000000000, pos = 0, status = 0;

  for (i = 0; i < 10; i++)
  {
    p_str[pos++] = (intnum / divider) + 48;

    intnum = intnum % divider;
    divider /= 10;
    if ((p_str[pos-1] == '0') & (status == 0))
    {
      pos = 0;
    }
    else
    {
      status++;
    }
  }
}

/**
  * @brief  Prepare the first block
  * @param  p_data:  output buffer
  * @param  p_file_name: name of the file to be sent
  * @param  length: length of the file to be sent in bytes
  * @retval None
  */
static void PrepareIntialPacket(uint8_t *p_data, const uint8_t *p_file_name, uint32_t length)
{
  uint32_t i, j = 0;
  uint8_t astring[10];

  /* first 3 bytes are constant */
  p_data[PACKET_START_INDEX] = FYM_CODE_SOH;
  p_data[PACKET_NUMBER_INDEX] = 0x00;
  p_data[PACKET_CNUMBER_INDEX] = 0xff;

  /* Filename written */
  for (i = 0; (p_file_name[i] != '\0') && (i < FILE_NAME_LENGTH); i++)
  {
    p_data[i + PACKET_DATA_INDEX] = p_file_name[i];
  }

  p_data[i + PACKET_DATA_INDEX] = 0x00;

  /* file size written */
  Int2Str (astring, length);
  i = i + PACKET_DATA_INDEX + 1;
  while (astring[j] != '\0')
  {
    p_data[i++] = astring[j++];
  }

  /* padding with zeros */
  for (j = i; j < PACKET_SIZE + PACKET_DATA_INDEX; j++)
  {
    p_data[j] = 0;
  }

  u16 send_crc = CRC16(&p_data[3], PACKET_SIZE);
  p_data[131] = (u8)(send_crc >> 8);
  p_data[132] = (u8)(send_crc & 0xff);

}

static void YmodemSend(u8 *p_buf, u16 length)
{
    FASSERT(p_buf);
    u16 i = 0;
    for (i = 0; i < length; i++)
    {
        OutByte(p_buf[i]);
    }
}

static u32 YmodemRecv(u8 *p_buf, u16 length)
{
    
    u16 i = 0;
    for (i = 0; i < length; i++)
    {
        p_buf[i] = GetByte();
    }
    return 1;
}

/**
  * @brief  Prepare the data packet
  * @param  p_source: pointer to the data to be sent
  * @param  p_packet: pointer to the output buffer
  * @param  pkt_nr: number of the packet
  * @param  size_blk: length of the block to be sent in bytes
  * @retval None
  */
static void PreparePacket(u8 *p_source, u8 *p_packet, u8 pkt_nr, u32 size_blk)
{
  u8 *p_record;
  u32 i, size, packet_size;

  /* Make first three packet */
  if(size_blk >= PACKET_SIZE)
  {
      packet_size = PACKET_1K_SIZE;
  }
  else
  {
       packet_size = PACKET_SIZE;
  }

  size = size_blk < packet_size ? size_blk : packet_size;
  if (packet_size == PACKET_1K_SIZE)
  {
    p_packet[PACKET_START_INDEX] = FYM_CODE_STX;
  }
  else
  {
    p_packet[PACKET_START_INDEX] = FYM_CODE_SOH;
  }
  p_packet[PACKET_NUMBER_INDEX] = pkt_nr;
  p_packet[PACKET_CNUMBER_INDEX] = (~pkt_nr);
  p_record = p_source;

  /* Filename packet has valid data */
  for (i = PACKET_DATA_INDEX; i < size + PACKET_DATA_INDEX; i++)
  {
    p_packet[i] = *p_record++;
  }
  if ( size  <= packet_size)
  {
    for (i = size + PACKET_DATA_INDEX; i < packet_size + PACKET_DATA_INDEX; i++)
    {
      p_packet[i] = 0x1A; /* EOF (0x1A) or 0x00 */
    }
  }
}


/* This function
p_buf -- file data buf
*/
int YmodemTransmit(u8 *p_buf, const u8 *p_file_name, u32 file_size )
{
    FtYmodemCode code = FYM_CODE_SOH;
    u8 index = 1; 
    u8 a_rx_ctrl[2];
    u32 ack_recpt = 0;
    u32 state = 0;
    u8 *p_buf_int;
    u16 i = 0;

#ifdef YMODEM_USING_CRC_TABLE    
    u16 temp_crc;
#else /* CRC16_F */   
    u8 temp_chksum;
#endif /* CRC16_F */ 

    /* wait C */
    int err_cnt = 100;
    while(--err_cnt > 0)
    {
        if (YmodemRecv(&a_rx_ctrl[0], 1) == 1)
        {
            if (a_rx_ctrl[0] == FYM_CODE_C)
            {
                break;
            }
        }
    }
    if(err_cnt <= 0)
    {
        return FYM_ERR_ACK;
    }
    
    /* first packet */
    PrepareIntialPacket(aPacketData, p_file_name, file_size);
    YmodemSend(aPacketData, 133);
    
    /* wait ACK */
    if(YmodemRecv(&a_rx_ctrl[0], 1) == 1)
    {
        if (a_rx_ctrl[0] != FYM_CODE_ACK)
        {
            return FYM_ERR_ACK;
        }
    }

    /* wait C */
    if(YmodemRecv(&a_rx_ctrl[0], 1) == 1)
    {
        if (a_rx_ctrl[0] != FYM_CODE_C)
        {
            return FYM_ERR_ACK;
        }
    }

    p_buf_int = p_buf;
    u32 size = file_size;
    u32 blk_number = 1;
    u32 pkt_size;

    while(size) 
    {
        /* Prepare next packet */
        PreparePacket(p_buf_int, aPacketData, blk_number, size);

        /* Send next packet */
        if (size >= PACKET_SIZE)
        {
            pkt_size = PACKET_1K_SIZE;
        }
        else
        {
            pkt_size = PACKET_SIZE;
        }
            
    /* Send CRC or Check Sum based on CRC16_F */
    #ifdef YMODEM_USING_CRC_TABLE    
        temp_crc = CRC16(&aPacketData[PACKET_DATA_INDEX], pkt_size);
        aPacketData[PACKET_DATA_INDEX+pkt_size] = (u8)(temp_crc >> 8);
        aPacketData[PACKET_DATA_INDEX+pkt_size+1] = (u8)(temp_crc & 0xff);
    #else /* CRC16_F */   
        temp_chksum = CalcChecksum (&aPacketData[PACKET_DATA_INDEX], pkt_size);
    #endif /* CRC16_F */

        YmodemSend(aPacketData, pkt_size + PACKET_HEADER_SIZE+PACKET_TRAILER_SIZE);

        /* wait ACK */
        if(YmodemRecv(&a_rx_ctrl[0], 1) == 1)
        {
            if (a_rx_ctrl[0] != FYM_CODE_ACK)
            {
                return FYM_ERR_ACK;
            }
            ack_recpt = 1;
            if (size > pkt_size)
            {
                p_buf_int += pkt_size;
                size -= pkt_size;
                blk_number++;
                
            }
            else
            {
                p_buf_int += pkt_size;
                size = 0;
            }
        }
    }

    /* Sending End Of Transmission char */
    ack_recpt = 0;
    a_rx_ctrl[0] = 0x00;

    OutByte(FYM_CODE_EOT);
    if(YmodemRecv(&a_rx_ctrl[0], 1) == 1)
    {
        if (a_rx_ctrl[0] != FYM_CODE_NAK)
        {
            return FYM_ERR_ACK;
        }
    }

    while (!ack_recpt)
    {
        OutByte(FYM_CODE_EOT);
        /* wait ACK */
        if(YmodemRecv(&a_rx_ctrl[0], 1) == 1)
        {
            if (a_rx_ctrl[0] != FYM_CODE_ACK)
            {
                return FYM_ERR_ACK;
            }
            ack_recpt = 1;
        }
        
        /* wait C */
        if(YmodemRecv(&a_rx_ctrl[0], 1) == 1)
        {
            if (a_rx_ctrl[0] != FYM_CODE_C)
            {
                return FYM_ERR_ACK;
            }
        }
    }
    
  /* Empty packet sent - some terminal emulators need this to close session */
    /* Preparing an empty packet */
    aPacketData[PACKET_START_INDEX] = FYM_CODE_SOH;
    aPacketData[PACKET_NUMBER_INDEX] = 0;
    aPacketData[PACKET_CNUMBER_INDEX] = 0xFF;
    for (i = PACKET_DATA_INDEX; i < (PACKET_SIZE + PACKET_DATA_INDEX); i++)
    {
      aPacketData [i] = 0x00;
    }

     /* Send CRC or Check Sum based on CRC16_F */
    #ifdef YMODEM_USING_CRC_TABLE    
        temp_crc = CRC16(&aPacketData[PACKET_DATA_INDEX], PACKET_SIZE);
        aPacketData[PACKET_DATA_INDEX+PACKET_SIZE] = (u8)(temp_crc >> 8);
        aPacketData[PACKET_DATA_INDEX+PACKET_SIZE+1] = (u8)(temp_crc & 0xff);
    #else /* CRC16_F */   
        temp_chksum = CalcChecksum (&aPacketData[PACKET_DATA_INDEX], PACKET_SIZE);
    #endif /* CRC16_F */

    /* Send Packet */
	YmodemSend(&aPacketData[PACKET_START_INDEX], PACKET_SIZE + PACKET_HEADER_SIZE + PACKET_TRAILER_SIZE);

	/* wait ACK */
    if(YmodemRecv(&a_rx_ctrl[0], 1) == 1)
    {
        if (a_rx_ctrl[0] != FYM_CODE_ACK)
        {
            return FYM_ERR_ACK;
        }
    }

    return FT_SUCCESS;
}