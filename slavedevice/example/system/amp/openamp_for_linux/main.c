/*
 * @ : Copyright (c) 2021 Phytium Information Technology, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0.
 *
 * @Date: 2021-12-09 17:12:52
 * LastEditTime: 2023-01-11 15:36:09
 * @Description:  This file is for openamp test
 *
 * @Modify History:
 *  Ver   Who           Date         Changes
 * ----- ------         --------    --------------------------------------
 * 1.0   huanghe        2022/06/20      first release
 * 1.1  liushengming    2023/05/16      for openamp test
 */
/***************************** Include Files *********************************/
#include <openamp/version.h>
#include <metal/version.h>
#include <stdio.h>
#include "ftypes.h"
#include "fsleep.h"
#include "fprintk.h"
#include "stdio.h"
#include "fdebug.h"
#include "finterrupt.h"
#include "fcache.h"
#include "sdkconfig.h"
#include "shell_port.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define OPENAMP_MAIN_DEBUG_TAG "OPENAMP_SLAVE_MAIN"
#define OPENAMP_MAIN_DEBUG_I(format, ...) FT_DEBUG_PRINT_I(OPENAMP_MAIN_DEBUG_TAG, format, ##__VA_ARGS__)
#define OPENAMP_MAIN_DEBUG_W(format, ...) FT_DEBUG_PRINT_W(OPENAMP_MAIN_DEBUG_TAG, format, ##__VA_ARGS__)
#define OPENAMP_MAIN_DEBUG_E(format, ...) FT_DEBUG_PRINT_E(OPENAMP_MAIN_DEBUG_TAG, format, ##__VA_ARGS__)


/************************** Function Prototypes ******************************/
extern int FOpenampExample(void);

int main(void)
{
    OPENAMP_MAIN_DEBUG_I("complier %s ,%s \r\n", __DATE__, __TIME__);
    OPENAMP_MAIN_DEBUG_I("openamp lib version: %s (", openamp_version());
    OPENAMP_MAIN_DEBUG_I("Major: %d, ", openamp_version_major());
    OPENAMP_MAIN_DEBUG_I("Minor: %d, ", openamp_version_minor());
    OPENAMP_MAIN_DEBUG_I("Patch: %d)\r\n", openamp_version_patch());

    OPENAMP_MAIN_DEBUG_I("libmetal lib version: %s (", metal_ver());
    OPENAMP_MAIN_DEBUG_I("Major: %d, ", metal_ver_major());
    OPENAMP_MAIN_DEBUG_I("Minor: %d, ", metal_ver_minor());
    OPENAMP_MAIN_DEBUG_I("Patch: %d)\r\n", metal_ver_patch());
    /*run the atomic example*/
    return FOpenampExample();
}

