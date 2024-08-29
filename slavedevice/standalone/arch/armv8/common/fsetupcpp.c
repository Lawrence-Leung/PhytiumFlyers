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
 * FilePath: fsetupcpp.c
 * Date: 2022-03-08 21:56:42
 * LastEditTime: 2022-03-15 11:10:40
 * Description:  This file is for cpp environment setup
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2023/05/19       first release
 */

#include "sdkconfig.h"
#include "fdebug.h"

#ifdef CONFIG_ENABLE_CXX

typedef void(*FCxxFunc)(void);

/* See: C++ Application Binary Interface Standard for the ARM 64-bit
   Architecture chapter 3.2.3. */
void FCxxInitGlobals(void)
{
	/* call constructors of static objects */
	extern FCxxFunc __init_array_start[];
	extern FCxxFunc __init_array_end[];
	FCxxFunc *func;

	for (func = __init_array_start; func < __init_array_end; func++)
	{
		(*func) ();
	}
}

void FCxxDeInitGlobals(void)
{
	/* call deconstructors of static objects */
	extern FCxxFunc __fini_array_start[];
	extern FCxxFunc __fini_array_end[];
	FCxxFunc *func;

	for (func = __fini_array_start; func < __fini_array_end; func++)
	{
		(*func) ();
	}
}

#endif