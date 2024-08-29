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
 * FilePath: system_call.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 09:25:20
 * Description:  This file is for C standard library stub function implmentation
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  huanghe     2021-11-10   first release
 */


#include <unistd.h>
#include <signal.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>
#include <reent.h>
#include <stdio.h>
#include "fearly_uart.h"
#include "sdkconfig.h"
#include "fsmp.h"
#include "ftypes.h"

/* _exit - Simple implementation. Does not return.
 */
void _exit(int return_value)
{
    (void)return_value;
    asm("dsb sy");
    while (1)
    {
        asm("wfi");
    }
}

#ifdef __cplusplus
extern "C"
{
s32 _open(const char *buf, s32 flags, s32 mode);
}
#endif

/*
 * _open -- open a file descriptor. We don't have a filesystem, so
 *         we return an error.
 */
s32 _open(const char *buf, s32 flags, s32 mode)
{
    (void)buf;
    (void)flags;
    (void)mode;
    errno = EIO;
    return (-1);
}

extern u8 _heap_start[];
extern u8 _heap_end[];

#ifdef __cplusplus
extern "C"
{
caddr_t _sbrk(s32 incr);
}
#endif

caddr_t _sbrk(s32 incr)
{
    static u8 *heap = NULL;
    u8 *prev_heap;
    static u8 *HeapEndPtr = (u8 *)&_heap_end;
    caddr_t Status;

    if (heap == NULL)
    {
        heap = (u8 *)&_heap_start;
    }
    prev_heap = heap;

    if (((heap + incr) <= HeapEndPtr) && (prev_heap != NULL))
    {
        heap += incr;
        Status = (caddr_t)((void *)prev_heap);
    }
    else
    {
        Status = (caddr_t) -1;
    }

    return Status;
}

/*
 * abort -- go out via exit...
 */
// void abort(void)
// {
//     _exit(1);
// }

#ifdef __cplusplus
extern "C"
{
s32 _close(s32 fd);
}
#endif

/*
 * close -- We don't need to do anything, but pretend we did.
 */

s32 _close(s32 fd)
{
    (void)fd;
    return (0);
}

struct tms *tms;
#ifdef __cplusplus
extern "C"
{
clock_t _times(struct tms *tms);
}
#endif

clock_t _times(struct tms *tms)
{
    (void)tms;

    errno = EIO;
    return (-1);
}

/*
 * fcntl -- Manipulate a file descriptor.
 *          We don't have a filesystem, so we do nothing.
 */
s32 fcntl(s32 fd, s32 cmd, long arg)
{
    (void)fd;
    (void)cmd;
    (void)arg;
    return 0;
}

#ifdef __cplusplus
extern "C"
{
s32 _fstat(s32 fd, struct stat *buf);
}
#endif
/*
 * fstat -- Since we have no file system, we just return an error.
 */
s32 _fstat(s32 fd, struct stat *buf)
{
    (void)fd;
    buf->st_mode = S_IFCHR; /* Always pretend to be a tty */

    return (0);
}

/*
 * getpid -- only one process, so just return 1.
 */
#ifdef __cplusplus
extern "C"
{
s32 _getpid(void);
}
#endif

pid_t getpid(void)
{
    return 1;
}

s32 _getpid(void)
{
    return 1;
}

#ifdef __cplusplus
extern "C"
{
s32 _isatty(s32 fd);
}
#endif

s32 _isatty(s32 fd)
{
    (void)fd;
    return (1);
}

#ifdef __cplusplus
extern "C"
{
int _kill(pid_t pid, int sig);
}
#endif

/*
 * kill -- go out via exit...
 */

int kill(pid_t pid, int sig)
{
    if (pid == 1)
    {
        _exit(sig);
    }
    return 0;
}

int _kill(pid_t pid, int sig)
{
    if (pid == 1)
    {
        _exit(sig);
    }
    return 0;
}

#ifdef __cplusplus
extern "C"
{
off_t _lseek(s32 fd, off_t offset, s32 whence);
}
#endif
/*
 * lseek --  Since a serial port is non-seekable, we return an error.
 */
off_t lseek(int fd, off_t offset, int whence)
{
    (void)fd;
    (void)offset;
    (void)whence;
    errno = ESPIPE;
    return ((off_t) -1);
}

off_t _lseek(s32 fd, off_t offset, s32 whence)
{
    (void)fd;
    (void)offset;
    (void)whence;
    errno = ESPIPE;
    return ((off_t) -1);
}

#ifdef __cplusplus
extern "C"
{
s32 open(char *buf, s32 flags, s32 mode);
}
#endif
/*
 * open -- open a file descriptor. We don't have a filesystem, so
 *         we return an error.
 */
s32 open(char *buf, s32 flags, s32 mode)
{
    (void)buf;
    (void)flags;
    (void)mode;
    errno = EIO;
    return (-1);
}

#ifdef __cplusplus
extern "C"
{
s32 _read(s32 fd, char *buf, s32 nbytes);
}
#endif

/*
 * read  -- read bytes from the serial port. Ignore fd, since
 *          we only have stdin.
 */
_READ_WRITE_RETURN_TYPE read(int fd, void *buf, size_t nbytes)
{
#ifdef STDIN_BASEADDRESS
    s32 i;
    s32 numbytes = 0;
    char *LocalBuf = buf;

    (void)fd;
    if (LocalBuf != NULL)
    {
        for (i = 0; i < nbytes; i++)
        {
            numbytes++;
            *(LocalBuf + i) = GetByte();
            if ((*(LocalBuf + i) == '\n') || (*(LocalBuf + i) == '\r'))
            {
                break;
            }
        }
    }

    return numbytes;
#else
    (void)fd;
    (void)buf;
    (void)nbytes;
    return 0;
#endif
}

s32 _read(s32 fd, char *buf, s32 nbytes)
{
#ifdef STDIN_BASEADDRESS
    s32 i;
    s32 numbytes = 0;
    char *LocalBuf = buf;

    (void)fd;
    if (LocalBuf != NULL)
    {
        for (i = 0; i < nbytes; i++)
        {
            numbytes++;
            *(LocalBuf + i) = GetByte();
            if ((*(LocalBuf + i) == '\n') || (*(LocalBuf + i) == '\r'))
            {
                break;
            }
        }
    }

    return numbytes;
#else
    (void)fd;
    (void)buf;
    (void)nbytes;
    return 0;
#endif
}

#ifdef __cplusplus
extern "C"
{
char *sbrk(s32 nbytes);
}
#endif

extern char HeapBase[];
extern char HeapLimit[];

void *sbrk(ptrdiff_t nbytes)
{
    char *base;
    static char *heap_ptr = HeapBase;

    base = heap_ptr;
    if ((heap_ptr != NULL) && (heap_ptr + nbytes <= (char *)&HeapLimit + 1))
    {
        heap_ptr += nbytes;
        return base;
    }
    else
    {
        errno = ENOMEM;
        return ((char *) -1);
    }
}

#ifdef __cplusplus
extern "C"
{
clock_t clock(void);
}
#endif
/*
 * clock -- It supposed to return processor time. We are not implementing
 *          this function, as timekeeping is tightly coupled with system, hence
 *          always returning 0. Users can override this with their system
 *          specific implementation.
 *
 */
clock_t clock(void)
{
    return (0);
}

#ifdef __cplusplus
extern "C"
{
s32 unlink(char *path);
}
#endif
/*
 * unlink -- since we have no file system,
 *           we just return an error.
 */
int unlink(const char *path)
{
    (void)path;
    errno = EIO;
    return (-1);
}

#ifdef __cplusplus
extern "C"
{
__attribute__((weak)) s32 _write(s32 fd, char *buf, s32 nbytes);
}
#endif

/*
 * write -- write bytes to the serial port. Ignore fd, since
 *          stdout and stderr are the same. Since we have no filesystem,
 *          open will only return an error.
 */
__attribute__((weak)) _READ_WRITE_RETURN_TYPE write(int fd, const void *buf, size_t nbytes)
{
#ifdef STDOUT_BASEADDRESS
    size_t i;
    char *LocalBuf = (char *)buf;

    (void)fd;
    for (i = 0; i < nbytes; i++)
    {
        if (LocalBuf != NULL)
        {
            LocalBuf += i;
        }
        if (LocalBuf != NULL)
        {
            if (*LocalBuf == '\n')
            {
                OutByte('\r');
            }
            OutByte(*LocalBuf);
        }
        if (LocalBuf != NULL)
        {
            LocalBuf -= i;
        }
    }
    return (nbytes);
#else
    (void)fd;
    (void)buf;
    (void)nbytes;
    return 0;
#endif
}

__attribute__((weak)) s32 _write(s32 fd, char *buf, s32 nbytes)
{

#ifdef STDOUT_BASEADDRESS
    s32 i;
    char *LocalBuf = buf;
    (void)fd;
#ifdef CONFIG_USE_AMP
    SpinLock();
#endif
    for (i = 0; i < nbytes; i++)
    {
        if (LocalBuf != NULL)
        {
            LocalBuf += i;
        }
        if (LocalBuf != NULL)
        {
            if (*LocalBuf == '\n')
            {

                OutByte('\r');
            }
            OutByte(*LocalBuf);
        }
        if (LocalBuf != NULL)
        {
            LocalBuf -= i;
        }
    }

#ifdef CONFIG_USE_AMP
    SpinUnlock();
#endif
    return (nbytes);
#else
    (void)fd;
    (void)buf;
    (void)nbytes;
    return 0;
#endif

}

int _gettimeofday(void)
{
    return 0;
}

void __sync_synchronize(void)
{

}