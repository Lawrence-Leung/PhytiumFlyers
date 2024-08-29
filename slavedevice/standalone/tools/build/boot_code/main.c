/*
 * @Copyright : (C) 2022 Phytium Information Technology, Inc. 
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
 * @FilePath: main.c
 * @Date: 2023-05-16 13:50:02
 * @LastEditTime: 2023-05-16 13:50:02
 * @Description:  This file is for 
 * 
 * @Modify History: 
 *  Ver   Who  Date   Changes
 * ----- ------  -------- --------------------------------------
 */


#include <string.h>
#include <stdio.h>
#include "sdkconfig.h"
#ifndef SDK_CONFIG_H__
    #warning "Please include sdkconfig.h"
#endif
#ifndef CONFIG_USE_LETTER_SHELL
    #error "Please include letter shell first!!!"
#endif
#include "shell_port.h"
#include "felf.h"
#include "fparameters.h"
#include "fcpu_info.h"
#include "fimage_info.h"
#include "fpsci.h"

extern  void * amp_img_start ;
extern  void * amp_img_end ;


typedef struct 
{
    void * image_start_address ;
    FImageInfo image_info ;
} amp_pack_img;

static amp_pack_img multcore_img[FCORE_NUM] = {0};



static void * CheckElfStartAddress(void * address)
{
    if(ElfIsImageValid((unsigned long)address))
    {
        return address;
    }
    return NULL;
}

static FError LoadCpu(void)
{
    void * temp_address = &amp_img_start;
    FImageInfo image_info ;
    u32 cpu_core_mask = 0 ;
    u32 cpu_id;
    FError ret ;
    
    while (temp_address < (void *)&amp_img_end)
    {
        void * boot_elf_address = CheckElfStartAddress((void *)temp_address);
        if(boot_elf_address != NULL)
        {
            u32 length = sizeof(FImageInfo);
            memset(&image_info,0,sizeof(image_info));
            ret = ElfGetSection((unsigned long)boot_elf_address, ".image_info", (u8 *)&image_info, &length);
            if(!ret)
            {
                if(cpu_core_mask & (1 << image_info.process_core) )
                {
                    printf("Error: Multiple programs for a single core \r\n");
                    return -4;
                }

                if(image_info.process_core >= FCORE_NUM)
                {
                    printf("Error: process_core is over max core number %d \r\n",image_info.process_core);
                    return -5;
                }

                cpu_core_mask |= (1 << image_info.process_core);
                multcore_img[image_info.process_core].image_start_address = (void * )ElfLoadElfImagePhdr((unsigned long)boot_elf_address);
            }
        }
        temp_address++;
    }
    
    if(cpu_core_mask == 0 || __builtin_popcount(cpu_core_mask) > FCORE_NUM)
    {
        printf("Error: No valid ELF image found or the number of found images exceeds the limit of FCORE_NUM \r\n");
        return (cpu_core_mask == 0) ? -1 : -2;
    }

    ret = GetCpuId(&cpu_id);
    if(ret != ERR_SUCCESS )
    {
        printf("Error: Failed to get CPU ID \r\n");
        return -3;
    }

    for (int i = 0; i< FCORE_NUM; i++)
    {
        if(((1<<i) & cpu_core_mask) && (i != cpu_id))
        {
            FPsciCpuMaskOn(1 << i, (uintptr)multcore_img[i].image_start_address);
        }
    }

    if(cpu_core_mask & (1<<cpu_id))
    {
        void (*boot_code)(void) = multcore_img[cpu_id].image_start_address;
        boot_code();
    }

    while (1)
    {
        asm volatile("wfi" \
                           :     \
                           :     \
                           : "memory") ;
    }

} 



int main(void)
{
    int ret;
    printf("LoadCpu is start \r\n");
    LoadCpu();
    /* In normal operation, it would never run here */
    return 0;
}

