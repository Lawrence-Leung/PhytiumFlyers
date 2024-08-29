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
 * FilePath: strto.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 09:25:04
 * Description:  This files is for implmentation of common string function not include in standard c library
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2021/10/06   first release
 */

#include <string.h>
#include "strto.h"

/*
 * NOTE! This ctype does not handle EOF like the standard C
 * library is required to.
 */

#define _U 0x01  /* upper */
#define _L 0x02  /* lower */
#define _D 0x04  /* digit */
#define _C 0x08  /* cntrl */
#define _P 0x10  /* punct */
#define _S 0x20  /* white space (space/lf/tab) */
#define _X 0x40  /* hex digit */
#define _SP 0x80 /* hard space (0x20) */

const unsigned char _ctype[] =
{
    _C, _C, _C, _C, _C, _C, _C, _C,                                       /* 0-7 */
    _C, _C | _S, _C | _S, _C | _S, _C | _S, _C | _S, _C, _C,              /* 8-15 */
    _C, _C, _C, _C, _C, _C, _C, _C,                                       /* 16-23 */
    _C, _C, _C, _C, _C, _C, _C, _C,                                       /* 24-31 */
    _S | _SP, _P, _P, _P, _P, _P, _P, _P,                                 /* 32-39 */
    _P, _P, _P, _P, _P, _P, _P, _P,                                       /* 40-47 */
    _D, _D, _D, _D, _D, _D, _D, _D,                                       /* 48-55 */
    _D, _D, _P, _P, _P, _P, _P, _P,                                       /* 56-63 */
    _P, _U | _X, _U | _X, _U | _X, _U | _X, _U | _X, _U | _X, _U,         /* 64-71 */
    _U, _U, _U, _U, _U, _U, _U, _U,                                       /* 72-79 */
    _U, _U, _U, _U, _U, _U, _U, _U,                                       /* 80-87 */
    _U, _U, _U, _P, _P, _P, _P, _P,                                       /* 88-95 */
    _P, _L | _X, _L | _X, _L | _X, _L | _X, _L | _X, _L | _X, _L,         /* 96-103 */
    _L, _L, _L, _L, _L, _L, _L, _L,                                       /* 104-111 */
    _L, _L, _L, _L, _L, _L, _L, _L,                                       /* 112-119 */
    _L, _L, _L, _P, _P, _P, _P, _C,                                       /* 120-127 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                       /* 128-143 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                       /* 144-159 */
    _S | _SP, _P, _P, _P, _P, _P, _P, _P, _P, _P, _P, _P, _P, _P, _P, _P, /* 160-175 */
    _P, _P, _P, _P, _P, _P, _P, _P, _P, _P, _P, _P, _P, _P, _P, _P,       /* 176-191 */
    _U, _U, _U, _U, _U, _U, _U, _U, _U, _U, _U, _U, _U, _U, _U, _U,       /* 192-207 */
    _U, _U, _U, _U, _U, _U, _U, _P, _U, _U, _U, _U, _U, _U, _U, _L,       /* 208-223 */
    _L, _L, _L, _L, _L, _L, _L, _L, _L, _L, _L, _L, _L, _L, _L, _L,       /* 224-239 */
    _L, _L, _L, _L, _L, _L, _L, _P, _L, _L, _L, _L, _L, _L, _L, _L
};      /* 240-255 */

#define __ismask(x) (_ctype[(int)(unsigned char)(x)])

#define isalnum(c) ((__ismask(c) & (_U | _L | _D)) != 0)
#define isalpha(c) ((__ismask(c) & (_U | _L)) != 0)
#define iscntrl(c) ((__ismask(c) & (_C)) != 0)
#define isdigit(c) ((__ismask(c) & (_D)) != 0)
#define isgraph(c) ((__ismask(c) & (_P | _U | _L | _D)) != 0)
#define islower(c) ((__ismask(c) & (_L)) != 0)
#define isprint(c) ((__ismask(c) & (_P | _U | _L | _D | _SP)) != 0)
#define ispunct(c) ((__ismask(c) & (_P)) != 0)
#define isspace(c) ((__ismask(c) & (_S)) != 0)
#define isupper(c) ((__ismask(c) & (_U)) != 0)
#define isxdigit(c) ((__ismask(c) & (_D | _X)) != 0)

static inline unsigned char __tolower(unsigned char c)
{
    if (isupper(c))
    {
        c -= 'A' - 'a';
    }
    return c;
}

static inline unsigned char __toupper(unsigned char c)
{
    if (islower(c))
    {
        c -= 'a' - 'A';
    }
    return c;
}

#define tolower(c) __tolower(c)
#define toupper(c) __toupper(c)

/* from lib/kstrtox.c */
static const char *_parse_integer_fixup_radix(const char *s, unsigned int *base)
{
    if (*base == 0)
    {
        if (s[0] == '0')
        {
            if (tolower(s[1]) == 'x' && isxdigit(s[2]))
            {
                *base = 16;
            }
            else
            {
                *base = 8;
            }
        }
        else
        {
            *base = 10;
        }
    }
    if (*base == 16 && s[0] == '0' && tolower(s[1]) == 'x')
    {
        s += 2;
    }
    return s;
}

unsigned long simple_strtoul(const char *cp, char **endp,
                             unsigned int base)
{
    unsigned long result = 0;
    unsigned long value;

    cp = _parse_integer_fixup_radix(cp, &base);

    while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp - '0' : (islower(*cp) ? toupper(*cp) : *cp) - 'A' + 10) < base)
    {
        result = result * base + value;
        cp++;
    }

    if (endp)
    {
        *endp = (char *)cp;
    }

    return result;
}

int strict_strtoul(const char *cp, unsigned int base, unsigned long *res)
{
    char *tail;
    unsigned long val;
    size_t len;

    *res = 0;
    len = strlen(cp);
    if (len == 0)
    {
        return -1;
    }

    val = simple_strtoul(cp, &tail, base);
    if (tail == cp)
    {
        return -1;
    }

    if ((*tail == '\0') ||
        ((len == (size_t)(tail - cp) + 1) && (*tail == '\n')))
    {
        *res = val;
        return 0;
    }

    return -1;
}

long simple_strtol(const char *cp, char **endp, unsigned int base)
{
    if (*cp == '-')
    {
        return -simple_strtoul(cp + 1, endp, base);
    }

    return simple_strtoul(cp, endp, base);
}