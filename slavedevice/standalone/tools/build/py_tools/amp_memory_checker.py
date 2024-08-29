# memory_check.py

import sys
import os
from check_memory_overlap import *
from soc_check import *
from amp_soc_info import *

SDK_CONFIG_NAME = "sdkconfig.h"

# 检查文件是否存在
def check_kconfig(path):
    kconfig_file = os.path.join(path, SDK_CONFIG_NAME)
    if not os.path.exists(kconfig_file):
        print("kconfig file does not exist in {}".format(path))
        return -1
    return 0

# 主函数
def main():
    # 获取AMP路径
    boot_paths = sys.argv[1]
    amp_paths = sys.argv[2:]
    # print(amp_paths)
    config_path_dict = {}
    soc_check_dict = {}
    for path in amp_paths:
        # 检查kconfig文件是否存在
        if check_kconfig(path) == -1:
            print("\033[31m{}\033[0m kconfig is not valid".format(path))
            sys.exit(-1)

        config_path_dict[path] = path + "/"+SDK_CONFIG_NAME
        soc_check_dict[path] = path + "/"+SDK_CONFIG_NAME

    
    # 检查kconfig文件是否存在
    if check_kconfig(boot_paths) == -1:
        print("\033[31m{}\033[0m kconfig is not valid".format(boot_paths))
        sys.exit(-1)

    soc_check_dict[boot_paths] = boot_paths + "/"+SDK_CONFIG_NAME
    
    check_soc_names(soc_check_dict)
    
    if (memory_check_host(config_path_dict) == -1):
        sys.exit("\033[31mAdjust the code distribution of RAM and ROM as prompted\033[0m")

    # 当确定soc 参数之后，开始确定可用的边界值
    print("soc_boot_space")
    soc_boot_space(boot_paths,amp_paths)

if __name__ == "__main__":
    main()
