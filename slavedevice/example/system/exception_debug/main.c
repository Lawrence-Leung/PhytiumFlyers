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
 * FilePath: main.c
 * Date: 2022-02-24 13:42:19
 * LastEditTime: 2022-03-21 17:02:53
 * Description:  This file is for exception debug example main entry.
 *
 * Modify History:
 *  Ver   Who            Date         Changes
 * ----- ------        --------    --------------------------------------
 *  1.0  wangxiaodong  2022/11/1    init commit
 */
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "ftypes.h"
#include "shell.h"
#include "shell_port.h"
#include "fmmu.h"
#include "fio.h"
#include "fkernel.h"
#include "sdkconfig.h"

#ifdef __aarch64__
#include "faarch64.h"
#else
#include "faarch32.h"
#endif


static int FExcOpsInvalidMemAccess(void)
{
    static char buf[64] = {0};

    buf[0] = 'A';
    buf[32] = 'B';
    printf("Buf @%p buf[0] = %c, buf[32] = %c\r\n",
           buf, buf[0], buf[32]);

#ifdef __aarch64__           
    FSetTlbAttributes((uintptr)buf, sizeof(buf), MT_NORMAL | MT_RO | MT_NS);
#else
    FSetTlbAttributes((uintptr)buf, sizeof(buf), MT_NORMAL | MT_P_RO_U_RO | MT_NS);
#endif    

    buf[0] = 'a'; /* cause sync error */

    return 0;
}

static int FExcOpsUndefInstr(void)
{
    printf("Exception debug FExcOpsUndefInstr func\n");

#ifdef __aarch64__
    AARCH64_READ_SYSREG(SCTLR_EL3);
#else
    #define UNDEFINED_INS	15, 0, 3, 2, 1
    AARCH32_READ_SYSREG_32(UNDEFINED_INS);
#endif

    return 0;
}

static int FExcOpsDataAbort(void)
{
    printf("Exception debug FExcOpsDataAbort func\n");
    uintptr data_addr = 0x02000000;
    FtOut32(data_addr, 12); /* invalid data access to trap Data abort */
    // FtIn32(data_addr); /* invalid data access to trap Data abort */

    return 0;
}



int main()
{
    printf("Exception debug func, FT Date: %s, Time: %s\n", __DATE__, __TIME__);
    BaseType_t xReturn = pdPASS;

#if defined(CONFIG_EXCEPTION_INVALID_INSTRUCTION)
    /* Synchronous exception test */
    FExcOpsUndefInstr();    /* a64--Synchronous-Invalid instructions, a32--UNDEFINED instruction*/

#elif defined(CONFIG_EXCEPTION_ACCESS_PERMISSION_ERROR)
    /* Synchronous exception test */
    FExcOpsInvalidMemAccess();/* a64--Synchronous-Memory access, a32--Data Abort */

#else
    /* Asynchronous exception test */
    FExcOpsDataAbort(); /* a64--SError, a32--Data Abort */

#endif

    xReturn = LSUserShellTask();
    if (xReturn != pdPASS)
    {
        goto FAIL_EXIT;
    }

    vTaskStartScheduler(); /* 启动任务，开启调度 */

    while (1); /* 正常不会执行到这里 */

FAIL_EXIT:
    printf("Failed,the xReturn value is 0x%x. \r\n", xReturn);
    return 0;
}