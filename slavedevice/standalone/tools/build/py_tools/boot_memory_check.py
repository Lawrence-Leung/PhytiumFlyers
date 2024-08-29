# '''
# Copyright : (C) 2022 Phytium Information Technology, Inc. 
# All Rights Reserved.
 
# This program is OPEN SOURCE software: you can redistribute it and/or modify it  
# under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,  
# either version 1.0 of the License, or (at your option) any later version. 
 
# This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;  
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the Phytium Public License for more details. 
 

# FilePath: boot_range_check.py
# Date: 2023-06-01 17:37:36
# LastEditTime: 2023-06-01 17:37:36
# Description:  This file is for 

# Modify History: 
#  Ver   Who  Date   Changes
# ----- ------  -------- --------------------------------------
# '''

import os
import re


def boot_memory_check():
    # 在当前目录查找 sdkconfig.h 和 available_space.h 文件
    current_directory = os.getcwd()
    sdkconfig_file = os.path.join(current_directory, 'sdkconfig.h')
    available_space_file = os.path.join(current_directory, 'available_space.h')

    # 定义参数列表
    boot_load_range_params = ['CONFIG_IMAGE_LOAD_ADDRESS', 'CONFIG_IMAGE_MAX_LENGTH']
    available_space_params = ['AVAILABLE_SPACE_START_', 'AVAILABLE_SPACE_END_']

    def get_params_from_vaild_header(filename, params):
        with open(filename, 'r') as f:
            content = f.read()
        result = {}

        for param in params:
            matches = re.findall(f'#define\s+{param}(\d+)\s+(0x[0-9a-fA-F]+)', content)
            for x, value in matches:
                result[param + x] = int(value, 16)
        return result

    def get_params_from_file(filename, params):
        with open(filename, 'r') as f:
            content = f.read()
        result = {}
        for param in params:
            match = re.search(f'#define\s+{param}\s+(0x[0-9a-fA-F]+)', content)
            if match:
                result[param] = int(match.group(1), 16)
        return result

    boot_load_range = get_params_from_file(sdkconfig_file, boot_load_range_params)
    available_space_ranges = get_params_from_vaild_header(available_space_file, available_space_params)


    boot_load_start = boot_load_range[boot_load_range_params[0]]
    boot_load_end = boot_load_start + boot_load_range[boot_load_range_params[1]] - 1

    available_flg = 0

    for i in range(len(available_space_ranges) // 2):
        available_space_start = available_space_ranges[available_space_params[0] + str(i)]
        available_space_end = available_space_ranges[available_space_params[1] + str(i)]
        
        print("|{:^77}|".format("available space range - start: 0x{:x}, end: 0x{:x}".format(available_space_start, available_space_end)))
        
        if boot_load_start >= available_space_start and boot_load_end <= available_space_end:
            print('Boot load range is within available space range.')
            available_flg = 1 
            break
        else:
            pass
            
    if available_flg == 0:
        print("Adjust boot code space to match the following available space,via make menuconfig")
        print("|{:^77}|".format("Boot load range - start: 0x{:x}, end: 0x{:x}".format(boot_load_start, boot_load_end)))
        exit(1)
    

if __name__ == "__main__":
    boot_memory_check()

