'''
Copyright : (C) 2022 Phytium Information Technology, Inc. 
All Rights Reserved.
 
This program is OPEN SOURCE software: you can redistribute it and/or modify it  
under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,  
either version 1.0 of the License, or (at your option) any later version. 
 
This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;  
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the Phytium Public License for more details. 
 

FilePath: check_memory_overlap.py
Date: 2023-05-29 14:22:31
LastEditTime: 2023-05-29 14:22:31
Description:  This file is for 

Modify History: 
 Ver   Who  Date   Changes
----- ------  -------- --------------------------------------
'''
#!/usr/bin/env python3
import re
import sys
from memory_regions_checker import *


def memory_check(filename,project_name):
    # 定义正则表达式模式
    all_pattern = r"#define\s+(\w+)\s+(.+)"

    # 读取头文件内容
    try:
        with open(filename, "r") as f:
            lines  = f.readlines()
    except :
        print("\033[31m{}\033[0m is not valid, please check AMP_PATH".format(filename))
        sys.exit(-1)
        
    # 提取宏定义中的数值
    # matches = re.findall(pattern, content)
    # all_matches = re.findall(all_pattern, content)


    # 将键值对存储到字典中
    all_config = {}

    for line in lines:
        match = re.match(all_pattern, line)
        if match:
            key = match.group(1)
            value = match.group(2).strip().strip('"')
            all_config[key] = value
            # print(key,value)
        
        
    # 名称
    
    try:
        memory_name = all_config["CONFIG_SOC_NAME"]+ all_config["CONFIG_TARGET_TYPE_NAME"] +"_"+all_config["CONFIG_ARCH_EXECUTION_STATE"] +"_" + all_config["CONFIG_BOARD_NAME"]
    except:
        memory_name = all_config["CONFIG_SOC_NAME"] +"_"+all_config["CONFIG_ARCH_EXECUTION_STATE"] +"_" + all_config["CONFIG_BOARD_NAME"]

    # 计算地址范围
    rom_start = int(all_config["CONFIG_IMAGE_LOAD_ADDRESS"],16)
    rom_size = int(all_config["CONFIG_IMAGE_MAX_LENGTH"],16)
    rom_end = rom_start + rom_size
    

    # 打印内存区域范围的ASCII字符画


    print("+{}+".format("-" * 77))
    print("|{:^77}|".format(project_name + "  " + memory_name))
    print("+{}+".format("-" * 77))
    print("Memory Range: 0x{:08X} - 0x{:08X} ({} bytes)".format(rom_start, rom_end, rom_size))
    print("+{}+".format("-" * 77))
    
    # 提取路径名
    # 使用split()函数分割路径，获取需要保留的子路径
    subpath = '/'.join(filename.split('/')[-2:-1])

    # 使用切片操作获取文件名
    filename = filename.split('/')[-1]
    # 使用join()函数连接子路径和文件名
    result = '/'.join([subpath, filename])
    
    return {"name":result,"CONFIG_IMAGE_LOAD_ADDRESS":rom_start,"CONFIG_IMAGE_MAX_LENGTH":rom_size}


def memory_check_host(config_path_dict):
    
    memory_regions=[]
    for key in config_path_dict:
        memory_regions.append(memory_check(config_path_dict[key],key))


    
    overlapping_regions = check_memory_regions(memory_regions)
    print(overlapping_regions)
    if overlapping_regions:
        print_overlapping_regions(overlapping_regions,memory_regions)
        return -1
    else:
        print("No overlapping regions found")
        return 0
    

if __name__ == '__main__':
    config_path_dict = {"apu_running":"/mnt/d/phytium/project/2.0/right/phytium-standalone-sdk/baremetal/test/amp_work_test_type1/apu_running/sdkconfig.h",
                        "rpu_running":"/mnt/d/phytium/project/2.0/right/phytium-standalone-sdk/baremetal/test/amp_work_test_type1/rpu_running/sdkconfig.h"}
    memory_check_host(config_path_dict)
    # apu_regions = memory_check("/mnt/d/phytium/project/2.0/right/phytium-standalone-sdk/baremetal/test/amp_work_test_type1/apu_running/sdkconfig.h","apu_running")
    # rpu_regions = memory_check("/mnt/d/phytium/project/2.0/right/phytium-standalone-sdk/baremetal/test/amp_work_test_type1/rpu_running/sdkconfig.h","rpu_running")
    # memory_regions=[]
    # memory_regions.append(apu_regions)
    # memory_regions.append(rpu_regions)
    # overlapping_regions = check_memory_regions(memory_regions)
    # if overlapping_regions:
    #         print_overlapping_regions(overlapping_regions,memory_regions)
    # else:
    #     print("No overlapping regions found")
    