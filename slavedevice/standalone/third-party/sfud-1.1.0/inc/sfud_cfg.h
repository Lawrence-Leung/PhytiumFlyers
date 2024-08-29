/*
 * This file is part of the Serial Flash Universal Driver Library.
 *
 * Copyright (c) 2016-2018, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: It is the configure head file for this library.
 * Created on: 2016-04-23
 */

#ifndef _SFUD_CFG_H_
#define _SFUD_CFG_H_


#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif

#define SFUD_DEBUG_MODE

#define SFUD_USING_SFDP

#define SFUD_USING_FLASH_INFO_TABLE

#ifdef CONFIG_SFUD_CTRL_FQSPI
#define SFUD_USING_QSPI
#endif

#if defined(CONFIG_TARGET_E2000)
enum {
    SFUD_FSPIM0_INDEX = 0,
    SFUD_FSPIM1_INDEX = 1,
    SFUD_FSPIM2_INDEX = 2,
    SFUD_FSPIM3_INDEX = 3,
    SFUD_FQSPI0_INDEX = 4,
    SFUD_DEVICE_INDEX
};
#endif

#if defined(CONFIG_TARGET_D2000) || defined(CONFIG_TARGET_FT2004)
enum {
    SFUD_FSPIM0_INDEX = 0,
    SFUD_FSPIM1_INDEX = 1,
    SFUD_FQSPI0_INDEX = 2,
    SFUD_DEVICE_INDEX
};
#endif

#define FSPIM0_SFUD_NAME "FSPIM0"
#define FSPIM1_SFUD_NAME "FSPIM1"
#if defined(CONFIG_TARGET_E2000)
#define FSPIM2_SFUD_NAME "FSPIM2"
#define FSPIM3_SFUD_NAME "FSPIM3"
#endif
#define FQSPI0_SFUD_NAME "FQSPI0"

#if defined(CONFIG_TARGET_E2000)
#define SFUD_FLASH_DEVICE_TABLE                                                 \
{                                                                               \
    [SFUD_FSPIM0_INDEX] = {.name = "SPI0-FLASH", .spi.name = FSPIM0_SFUD_NAME}, \
    [SFUD_FSPIM1_INDEX] = {.name = "SPI1-FLASH", .spi.name = FSPIM1_SFUD_NAME}, \
    [SFUD_FSPIM2_INDEX] = {.name = "SPI2-FLASH", .spi.name = FSPIM2_SFUD_NAME}, \
    [SFUD_FSPIM3_INDEX] = {.name = "SPI3-FLASH", .spi.name = FSPIM3_SFUD_NAME}, \
    [SFUD_FQSPI0_INDEX] = {.name = "QSPI0-FLASH", .spi.name = FQSPI0_SFUD_NAME} \
}
#endif

#if defined(CONFIG_TARGET_D2000) || defined(CONFIG_TARGET_FT2004)
#define SFUD_FLASH_DEVICE_TABLE                                                 \
{                                                                               \
    [SFUD_FSPIM0_INDEX] = {.name = "SPI0-FLASH", .spi.name = FSPIM0_SFUD_NAME}, \
    [SFUD_FSPIM1_INDEX] = {.name = "SPI1-FLASH", .spi.name = FSPIM1_SFUD_NAME}, \
    [SFUD_FQSPI0_INDEX] = {.name = "QSPI0-FLASH", .spi.name = FQSPI0_SFUD_NAME} \
}
#endif

#endif /* _SFUD_CFG_H_ */
