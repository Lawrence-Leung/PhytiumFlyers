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
 * FilePath: cmd_md.c
 * Date: 2022-02-10 14:53:43
 * LastEditTime: 2022-02-25 11:47:34
 * Description:  This files is for md command implmentation
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2021/9/6    init commit
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../src/shell.h"
#include "fio.h"
#include "ftypes.h"

static void MdCmdUsage()
{
	printf("usage:\r\n");
	printf("    md [-b|-w|-l|-q] address [-c count]\r\n");
}

static int MdCmdEntry(int argc, char *argv[])
{
	uintptr addr = 0;
	char buf[16];
	int n = 64, size = 1;
	int i, len;
	u8 b;
	u16 w;
	u32 l;
	u64 q;

	if (argc < 2)
	{
		MdCmdUsage();
		return -1;
	}

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-b"))
			size = 1;
		else if (!strcmp(argv[i], "-w"))
			size = 2;
		else if (!strcmp(argv[i], "-l"))
			size = 4;
		else if (!strcmp(argv[i], "-q"))
			size = 8;
		else if (!strcmp(argv[i], "-c") && (argc > i + 1))
		{
			n = strtoul(argv[i + 1], NULL, 0);
			i++;
		}
		else if (*argv[i] == '-')
		{
			MdCmdUsage();
			return (-1);
		}
		else if (*argv[i] != '-' && strcmp(argv[i], "-") != 0)
		{
			addr = strtoul(argv[i], NULL, 0);
		}
	}

	if (size == 1)
	{
		addr &= ~((uintptr)0x0);
	}
	else if (size == 2)
	{
		addr &= ~((uintptr)0x1);
	}
	else if (size == 4)
	{
		addr &= ~((uintptr)0x3);
	}
	else if (size == 8)
	{
		addr &= ~((uintptr)0x7);
	}
	n = n * size;

	while (n > 0)
	{
		len = (n > 16) ? 16 : n;
		printf("%08lx: ", addr);
		if (size == 1)
		{
			for (i = 0; i < len; i += size)
			{
				FtOut8((uintptr)(&buf[i]), (b = FtIn8(addr + i)));
				printf(" %02lx", b);
			}
		}
		else if (size == 2)
		{
			for (i = 0; i < len; i += size)
			{
				FtOut16((uintptr)(&buf[i]), (w = FtIn16(addr + i)));
				printf(" %04lx", w);
			}
		}
		else if (size == 4)
		{
			for (i = 0; i < len; i += size)
			{
				FtOut32((uintptr)(&buf[i]), (l = FtIn32(addr + i)));
				printf(" %08lx", l);
			}
		}
		else if (size == 8)
		{
			for (i = 0; i < len; i += size)
			{
				FtOut64((uintptr)(&buf[i]), (q = FtIn64(addr + i)));
				printf(" %016llx", q);
			}
		}
		printf("%*s", (16 - len) * 2 + (16 - len) / size + 4, "");
		for (i = 0; i < len; i++)
		{
			if ((buf[i] < 0x20) || (buf[i] > 0x7e))
				printf(".");
			else
				printf("%c", buf[i]);
		}
		addr += len;
		n -= len;
		printf("\r\n");
	}

	return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), md, MdCmdEntry, dump a memory region);