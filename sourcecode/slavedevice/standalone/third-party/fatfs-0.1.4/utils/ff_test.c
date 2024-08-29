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
 * FilePath: ff_test.c
 * Date: 2022-07-21 13:21:43
 * LastEditTime: 2022-07-21 13:21:44
 * Description:  This files is for fatfs test implmentation
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/12/3   init commit
 */

#include <string.h>
#include "fdebug.h"
#include "fassert.h"
#include "fkernel.h"
#include "ferror_code.h"
#include "ff_utils.h"
#include "diskio.h"

#define FF_DEBUG_TAG "FATFS"
#define FF_ERROR(format, ...)   FT_DEBUG_PRINT_E(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_INFO(format, ...)    FT_DEBUG_PRINT_I(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_WARN(format, ...)    FT_DEBUG_PRINT_W(FF_DEBUG_TAG, format, ##__VA_ARGS__)

FRESULT ff_contiguous_file_test (
    FIL* fp,    /* [IN]  Open file object to be checked */
    int* cont   /* [OUT] 1:Contiguous, 0:Fragmented or zero-length */
);

FRESULT ff_list_test (TCHAR* path)
{
	FRESULT res; 
	DIR dir;
	UINT i;
	static FILINFO fno;
	res = f_opendir(&dir,path); /* open dir */

	if(res == FR_OK) 
	{
		for(;;) /* traversal dir */
		{
			res = f_readdir(&dir, &fno); /* read dir */
			if(res != FR_OK || fno.fname[0] == 0) 
                break;
			if(fno.fattrib & AM_DIR) /* if it is dir */
			{
				i = strlen(path); /* get length of dir */
				sprintf(&path[i],"/%s",fno.fname); /* append dir */
				printf("is dir::%s\r\n",path);
				res = ff_list_test(path);
				if(res != FR_OK) break;
				path[i] = 0; 
			}
            else
			{
				printf("\t\tis file:%s/%s\r\n",path, fno.fname); /* if it is file */
                printf("\t\tSize: %lu\n", fno.fsize);
                printf("\t\tTimestamp: %u-%02u-%02u, %02u:%02u\n",
                    (fno.fdate >> 9) + 1980, fno.fdate >> 5 & 15, fno.fdate & 31,
                    fno.ftime >> 11, fno.ftime >> 5 & 63);
                printf("\t\tAttributes: %c%c%c%c%c\n",
                    (fno.fattrib & AM_DIR) ? 'D' : '-',
                    (fno.fattrib & AM_RDO) ? 'R' : '-',
                    (fno.fattrib & AM_HID) ? 'H' : '-',
                    (fno.fattrib & AM_SYS) ? 'S' : '-',
                    (fno.fattrib & AM_ARC) ? 'A' : '-');
            }
		}
	}
	else
	{
		printf("Failed - %s\r\n",&res);
	}

	f_closedir(&dir);
	return res;    
}

/*------------------------------------------------------------/
/ Open or create a file in append mode
/ (This function was sperseded by FA_OPEN_APPEND flag at FatFs R0.12a)
/------------------------------------------------------------*/

FRESULT ff_append_test (
    const TCHAR* path    /* [IN]  File name to be opened */
)
{
    FRESULT fr;
    FATFS fs;
    FIL fil;
    static int cnt = 0;
    FILINFO fno;
    BYTE buff[32];
    UINT bytes_wr = 0;

    /* Opens an existing file. If not exist, creates a new file. */    
    fr = f_stat(path, &fno);
    switch (fr) {

    case FR_OK:
        printf("Size: %lu\n", fno.fsize);
        printf("Timestamp: %u-%02u-%02u, %02u:%02u\n",
               (fno.fdate >> 9) + 1980, fno.fdate >> 5 & 15, fno.fdate & 31,
               fno.ftime >> 11, fno.ftime >> 5 & 63);
        printf("Attributes: %c%c%c%c%c\n",
               (fno.fattrib & AM_DIR) ? 'D' : '-',
               (fno.fattrib & AM_RDO) ? 'R' : '-',
               (fno.fattrib & AM_HID) ? 'H' : '-',
               (fno.fattrib & AM_SYS) ? 'S' : '-',
               (fno.fattrib & AM_ARC) ? 'A' : '-');
        fr = f_open(&fil, path, FA_WRITE | FA_READ);

        /* Seek to end of the file to append data */
        fr = f_lseek(&fil, f_size(&fil));
        if (fr != FR_OK){
            printf("Seek file %s failed \r\n", path);
            f_close(&fil);
        }

        break;

    case FR_NO_FILE:
        printf("\"%s\" is not exist.\n", path);
        fr = f_open(&fil, path, FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
        break;

    default:
        printf("An error occured. (%d)\n", fr);
        return fr;
    }    

    if (fr != FR_OK)
    {
        printf("Open file %s failed, err = %d !!! \r\n", path, fr);
        return fr;
    } 

    /* Append a line */
	sprintf(buff, "cnt = %02u\n", cnt++); /* append dir */
    fr = f_write(&fil, buff, strlen(buff) + 1, &bytes_wr);
    if (fr != FR_OK)
    {
        printf("Append file %s failed, err = %d  !!! \r\n", path, fr);
        return fr;
    }

    /* Close the file */
    fr = f_close(&fil);
    if (fr != FR_OK)
    {
        printf("Close file %s failed, err = %d  !!! \r\n", path, fr);
        return fr;
    }

    return fr;    
}

FRESULT ff_read_test (const TCHAR* path)
{
    FASSERT(path);
    FRESULT res;
    UINT fnum;
    FIL file_handler;
    static BYTE buff[256];

    res = f_open(&file_handler, path, FA_READ);
    if (res != FR_OK)
    {
        printf("Open file %s failed !!!\r\n", path);
        return res;
    }

    res = f_lseek(&file_handler, 0);
    if (res != FR_OK)
    {
        printf("Seek to file begin failed !!!\r\n");
        f_close(&file_handler);
        return res;
    }    

    memset(buff, 0, sizeof(buff));
    res = f_read(&file_handler, buff, 255, &fnum);
    if (res != FR_OK)
    {
        printf("Read file %s failed !!!\r\n", path);
        f_close(&file_handler);
        return res;
    }
    else
    {
        printf("Read file %p success\r\n", &file_handler);
        FtDumpHexByte(buff, fnum);
    }


    f_close(&file_handler);
    return res;        
}

/*------------------------------------------------------------/
/ Delete a sub-directory even if it contains any file
/-------------------------------------------------------------/
/ The delete_node() function is for R0.12+.
/ It works regardless of FF_FS_RPATH.
*/

static FRESULT delete_node (
                TCHAR* path,    /* Path name buffer with the sub-directory to delete */
                UINT sz_buff,   /* Size of path name buffer (items) */
                FILINFO* fno    /* Name read buffer */
)
{
    UINT i, j;
    FRESULT fr;
    DIR dir;


    fr = f_opendir(&dir, path); /* Open the sub-directory to make it empty */
    if (fr != FR_OK) return fr;

    for (i = 0; path[i]; i++) ; /* Get current path length */
    path[i++] = _T('/');

    for (;;) {
        fr = f_readdir(&dir, fno);  /* Get a directory item */
        if (fr != FR_OK || !fno->fname[0]) break;   /* End of directory? */
        j = 0;
        do {    /* Make a path name */
            if (i + j >= sz_buff) { /* Buffer over flow? */
                fr = 100; break;    /* Fails with 100 when buffer overflow */
            }
            path[i + j] = fno->fname[j];
        } while (fno->fname[j++]);
        if (fno->fattrib & AM_DIR) {    /* Item is a sub-directory */
            fr = delete_node(path, sz_buff, fno);
        } else {                        /* Item is a file */
            fr = f_unlink(path);
        }
        if (fr != FR_OK) break;
    }

    path[--i] = 0;  /* Restore the path name */
    f_closedir(&dir);

    if (fr == FR_OK) fr = f_unlink(path);  /* Delete the empty sub-directory */
    return fr;
}

FRESULT ff_delete_test (const TCHAR* path)
{
    FRESULT fr;
    FATFS fs;
    static TCHAR buff[256] = {0};
    FILINFO fno;

    /* Directory to be deleted */
    strcpy(buff, path);

    /* Delete the directory */
    fr = delete_node(buff, sizeof(buff) / sizeof(buff[0]), &fno);

    /* Check the result */
    if (fr) {
        printf(_T("Failed to delete the directory. (%u)\r\n"), fr);
    } else {
        printf(_T("The directory and the contents have successfully been deleted.\r\n"), buff);
    }

    return fr;
}

/*----------------------------------------------------------------------/
/ Test if the file is contiguous                                        /
/----------------------------------------------------------------------*/

FRESULT ff_contiguous_file_test (
    FIL* fp,    /* [IN]  Open file object to be checked */
    int* cont   /* [OUT] 1:Contiguous, 0:Fragmented or zero-length */
)
{
    DWORD clst, clsz, step;
    FSIZE_t fsz;
    FRESULT fr;


    *cont = 0;
    fr = f_lseek(fp, 0);            /* Validates and prepares the file */
    if (fr != FR_OK) return fr;

#if FF_MAX_SS == FF_MIN_SS
    clsz = (DWORD)fp->obj.fs->csize * FF_MAX_SS;    /* Cluster size */
#else
    clsz = (DWORD)fp->obj.fs->csize * fp->obj.fs->ssize;
#endif
    fsz = f_size(fp);
    if (fsz > 0) {
        clst = fp->obj.sclust - 1;  /* A cluster leading the first cluster for first test */
        while (fsz) {
            step = (fsz >= clsz) ? clsz : (DWORD)fsz;
            fr = f_lseek(fp, f_tell(fp) + step);    /* Advances file pointer a cluster */
            if (fr != FR_OK) return fr;
            if (clst + 1 != fp->clust) break;       /* Is not the cluster next to previous one? */
            clst = fp->clust; fsz -= step;          /* Get current cluster for next test */
        }
        if (fsz == 0) *cont = 1;    /* All done without fail? */
    }

    return FR_OK;
}

FRESULT ff_big_file_test(const TCHAR* path, UINT sz_mb)
{
    FRESULT fr;
    FATFS fs;
    FIL fil;
    static BYTE buff[32];
    UINT bytes_wr = 0;

    fr = f_open(&fil, path, FA_CREATE_ALWAYS | FA_WRITE | FA_READ);

    /* Alloacte sz_mb of contiguous area to the file */
    fr = f_expand(&fil, sz_mb * SZ_1M, 1); /* opt = 1, allocate now */
    if (fr) /* Check if the file has been expanded */
    {
        f_close(&fil);
        printf("Failed to allocate contiguous area.\r\n");
        return fr;
    }

    fr = f_lseek(&fil, 0);
    if (fr != FR_OK)
    {
        printf("Seek to file begin failed !!!\r\n");
        return fr;
    }          

    /* Append a line */
	sprintf(buff, "%s\r\n", "this is a big file"); /* append dir */
    fr = f_write(&fil, buff, strlen(buff) + 1, &bytes_wr);
    if (fr != FR_OK)
    {
        printf("Append file %s failed, err = %d  !!! \r\n", path, fr);
        return fr;
    }

    /* Close the file */
    fr = f_close(&fil);
    if (fr != FR_OK)
    {
        printf("Close file %s failed, err = %d  !!! \r\n", path, fr);
        return fr;
    }

    printf("Create file-%s in %d MB\r\n", path, sz_mb);
    return fr;
}

FRESULT ff_basic_test (const TCHAR* mount_point, const TCHAR* file_name)
{
    FRESULT fr = FR_OK;
    static BYTE path[256];

	sprintf(path, "%s/%s", mount_point, file_name); /* append dir */
    fr = ff_append_test(path);
    if (fr != FR_OK)
    {
        printf("ff_append_test %s failed, err = %d  !!! \r\n", path, fr);
        return fr;
    }

	sprintf(path, "%s/%s", mount_point, file_name);
    fr = ff_append_test(path);
    if (fr != FR_OK)
    {
        printf("ff_append_test %s failed, err = %d  !!! \r\n", path, fr);
        return fr;
    }

	sprintf(path, "%s/%s", mount_point, file_name);
    fr = ff_read_test(path);
    if (fr != FR_OK)
    {
        printf("ff_read_test %s failed, err = %d  !!! \r\n", path, fr);
        return fr;
    }

	sprintf(path, "%s/%s", mount_point, file_name);
    fr = ff_big_file_test(path, 50);
    if (fr != FR_OK)
    {
        printf("ff_big_file_test %s failed, err = %d  !!! \r\n", path, fr);
        return fr;
    }

	sprintf(path, "%s", mount_point); /* append dir */
    fr = ff_list_test(path);
    if (fr != FR_OK)
    {
        printf("ff_list_test %s failed, err = %d  !!! \r\n", path, fr);
        return fr;
    }

	sprintf(path, "%s/%s", mount_point, file_name);
    fr = ff_read_test(path);
    if (fr != FR_OK)
    {
        printf("ff_read_test %s failed, err = %d  !!! \r\n", path, fr);
        return fr;
    }
    
    return fr;
}