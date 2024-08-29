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
 * Function: Portable interface for each platform.
 * Created on: 2016-04-23
 */

#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif
#include <sfud.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "fsleep.h"
#include "fdebug.h"

#ifdef CONFIG_SFUD_CTRL_FSPIM
#include "fspim_sfud_core.h"
#endif
#ifdef CONFIG_SFUD_CTRL_FQSPI
#include "fqspi_sfud_core.h"
#endif 

static char log_buf[256];

void sfud_log_debug(const char *file, const long line, const char *format, ...);
void sfud_log_info(const char *format, ...);

static void spi_lock(const sfud_spi *spi) 
{

}

static void spi_unlock(const sfud_spi *spi) 
{

}

/* about 100 microsecond delay */
static void retry_delay_100us(void) 
{
    fsleep_microsec(100);
}

/**
 * @name: sfud_spi_port_init
 * @msg: 通过SFUD框架初始化SPI驱动
 * @return {*}
 * @param {sfud_flash} *flash
 */
sfud_err sfud_spi_port_init(sfud_flash *flash) 
{
    sfud_err result = SFUD_SUCCESS;
    /**
     * add your port spi bus and device object initialize code like this:
     * 1. rcc initialize
     * 2. gpio initialize
     * 3. spi device initialize
     * 4. flash->spi and flash->retry item initialize
     *    flash->spi.wr = spi_write_read; //Required
     *    flash->spi.qspi_read = qspi_read; //Required when QSPI mode enable
     *    flash->spi.lock = spi_lock;
     *    flash->spi.unlock = spi_unlock;
     *    flash->spi.user_data = &spix;
     *    flash->retry.delay = null;
     *    flash->retry.times = 10000; //Required
     */
#ifdef CONFIG_SFUD_CTRL_FQSPI
    if (!memcmp(FQSPI0_SFUD_NAME, flash->spi.name, strlen(FQSPI0_SFUD_NAME)))
    {
        result = FQspiProbe(flash);
        if (result != SFUD_SUCCESS)
        {
            SFUD_ERROR("FQspiProbe failed\n");
            return result;
        }
        else if(result == SFUD_SUCCESS)
        if (result == SFUD_SUCCESS)
        {
            SFUD_INFO("FQspiProbe success flash.index=%d.", flash->index);
            goto ret;
        }
    } else
#endif
#ifdef CONFIG_SFUD_CTRL_FSPIM
    {
        result = FSpimProbe(flash);
        if (result != SFUD_SUCCESS)
        {
            SFUD_ERROR("FQspiProbe failed\n");
            return result;
        }
        else if(result == SFUD_SUCCESS)
        {
            SFUD_INFO("FSpimProbe success flash.index=%d.", flash->index);
            goto ret;
        }
    }
#endif
    return SFUD_ERR_NOT_FOUND;

ret:
    flash->spi.lock = spi_lock;
    flash->spi.unlock = spi_unlock;
    /* about 100 microsecond delay */
    flash->retry.delay = retry_delay_100us;
    /* adout 60 seconds timeout */
    flash->retry.times = 60 * 10000;

    return result;
}

/**
 * This function is print debug info.
 *
 * @param file the file which has call this function
 * @param line the line number which has call this function
 * @param format output format
 * @param ... args
 */
void sfud_log_debug(const char *file, const long line, const char *format, ...) 
{
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);
    SFUD_DEBUG("[SFUD](%s:%ld) ", file, line);
    /* must use vprintf to print */
    vsnprintf(log_buf, sizeof(log_buf), format, args);
    SFUD_DEBUG("%s\n", log_buf);
    va_end(args);
}

/**
 * This function is print routine info.
 *
 * @param format output format
 * @param ... args
 */
void sfud_log_info(const char *format, ...) 
{
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);
    SFUD_INFO("[SFUD]");
    /* must use vprintf to print */
    vsnprintf(log_buf, sizeof(log_buf), format, args);
    SFUD_INFO("%s\n", log_buf);
    va_end(args);
}