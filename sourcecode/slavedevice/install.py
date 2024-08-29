#!/usr/bin/env python3

import sys
import os
import pwd
import stat
import platform
import getpass
import tarfile
import re
import shutil

### platform constant
platform_tags = ["Linux_X86_64" "Linux_AARCH64" "Msys2"]
linux_x86 = 0
linux_aarch64 = 1
windows_msys2 = 2

### environment constant
sdk_profile_path = "/etc/profile.d/phytium_dev.sh"

def rm_line(str, file_path):
    with open(file_path,'r+') as f:
        lines = [line for line in f.readlines() if str not in line]
        f.seek(0)
        f.truncate(0)
        f.writelines(lines)

def ap_line(str, file_path):
    with open(file_path, 'a') as f:
        f.write(str + '\n')

# STEP 1: Check environment
# check install environment
if (platform.system() == 'Linux' ) and (platform.processor() == 'x86_64'):
    install_platform = linux_x86
elif (platform.system() == 'Linux' ) and (platform.processor() == 'aarch64'): # Arm64 computer
    install_platform = linux_aarch64
elif (re.search('MSYS_NT', platform.system()).span() == (0, len('MSYS_NT'))):
    install_platform = windows_msys2
else:
    print("[1]: Platform not support !!! ")
    exit()


# create '/etc/profile.d/phytium_standalone_sdk.sh' need sudo right, ask user to create it first
if not os.path.exists(sdk_profile_path):
    if (install_platform == linux_x86) or (install_platform == linux_aarch64):
        print("[1]: Please create sdk profile with 'sudo touch {}' first".format(sdk_profile_path))
        print("then 'sudo chmod 666 {}'".format(sdk_profile_path))
    else: # for Windows msys2
        print("[1]: Please create sdk profile with 'touch {}' first".format(sdk_profile_path))
        print("then 'chmod 666 {}'".format(sdk_profile_path))
    
    exit()

# get current user to install, profile depends on user
usr = getpass.getuser()
if (install_platform == windows_msys2):
    # arch is not able to get for msys2
    print("[1]: Usr: {}, OS: {}, Type: {}".format(usr, platform.system(), install_platform))
else:
    print("[1]: Usr: {}, OS: {}, Arch: {}, Type: {}".format(usr, platform.system(), platform.processor(), install_platform))

print("[1]: Enviroment variables will set at {}".format(sdk_profile_path))

# get absoulte path current pwd to install sdk
install_path, install_script = os.path.split(os.path.abspath(__file__))
curr_path = os.getcwd()
freertos_sdk_path = ''

# in case user call this script not from current path
if (curr_path != install_path):
    print("[1]: Please cd to install script path first !!!")
    exit()

# get absolute path of sdk install dir
freertos_sdk_path = install_path
print("[1]: Standalone SDK at {}".format(freertos_sdk_path))


# make sure sdk scripts are executable
os.system("chmod +x ./*.sh --silent ")
os.system("chmod +x ./scripts/*.sh --silent ")
os.system("chmod +x ./make/*.mk --silent ")
os.system("chmod +x ./lib/Kconfiglib/*.py --silent ")

# Add standalone sdk
standalone_sdk_v="6061198125dbbcf87ecb9ca56b82cf65ce2ccd8e"
standalone_path=freertos_sdk_path  + '/standalone'
standalone_branche="master"
standalone_remote="https://gitee.com/phytium_embedded/phytium-standalone-sdk.git"

if not os.path.exists(standalone_path):
    current_path = os.getcwd()

    os.system("git clone -b {} {} {}".format(standalone_branche, standalone_remote,standalone_path))
    os.chdir(standalone_path)# 切换工作路径至standalone 路径
    os.system("git config core.sparsecheckout true")
    os.system("git config advice.detachedHead false")
    os.system("echo \"arch/*\" >> {}".format(r'.git/info/sparse-checkout'))
    os.system("echo \"board/*\" >> {}".format(r'.git/info/sparse-checkout'))
    os.system("echo \"common/*\" >> {}".format(r'.git/info/sparse-checkout'))
    os.system("echo \"drivers/*\" >> {}".format(r'.git/info/sparse-checkout'))
    os.system("echo \"standalone.mk\" >> {}".format(r'.git/info/sparse-checkout'))
    os.system("echo \"lib/*\" >> {}".format(r'.git/info/sparse-checkout'))
    os.system("echo \"doc/*\" >> {}".format(r'.git/info/sparse-checkout'))
    os.system("echo \"third-party/*\" >> {}".format(r'.git/info/sparse-checkout'))    
    os.system("echo \"tools/*\" >> {}".format(r'.git/info/sparse-checkout'))
    os.system("echo \"soc/*\" >> {}".format(r'.git/info/sparse-checkout'))

    os.system("git checkout {}".format(standalone_sdk_v))
    print('[1]: Standalone sdk download is succeed')
    os.chdir(current_path) # 切换回当前路径
    lwip_port_arch_path=standalone_path + '/third-party/lwip-2.1.2/ports/arch'
    shutil.rmtree(lwip_port_arch_path)
else:
    print('[1]: Standalone sdk is exist')
    pass

## STEP 2: reset environment
# remove environment variables

try:
    sdk_profile = open(sdk_profile_path, "r+")
    sdk_profile.close()

except Exception as ex:
    print(ex)
    print("[1]: Create SDK profile {} failed !!!!".format(sdk_profile_path))
    exit()


rm_line("### PHYTIUM FREERTOS SDK SETTING START",sdk_profile_path)
rm_line("export FREERTOS_SDK_ROOT=",sdk_profile_path)
rm_line("export FREERTOS_STANDALONE=",sdk_profile_path)
rm_line("### PHYTIUM FREERTOS SDK SETTING END",sdk_profile_path)



print("[2]: Reset environment")

## STEP 3: get cross-platform compiler


#########################################
if not os.path.exists(os.environ.get('AARCH32_CROSS_PATH')):
    print("[3]: Failed, AARCH32 CC package {} non found !!!".format('AARCH32_CROSS_PATH'))
    exit()

if not os.path.exists(os.environ.get('AARCH64_CROSS_PATH')):
    print("[3]: Failed, AARCH64 CC package {} non found !!!".format('AARCH64_CROSS_PATH'))
    exit()

print("[4]: GNU CC version: 10.3.1-2021.07")

## STEP 4: write environment variables
os.environ['FREERTOS_SDK_ROOT'] = freertos_sdk_path

os.system("echo \"### PHYTIUM FREERTOS SDK SETTING START\" >> {}".format(sdk_profile_path))
os.system("echo \"export FREERTOS_SDK_ROOT={}\" >> {}".format(freertos_sdk_path, sdk_profile_path))
os.system("echo \"export FREERTOS_STANDALONE={}\" >> {}".format(standalone_path, sdk_profile_path))
os.system("echo \"### PHYTIUM FREERTOS SDK SETTING END\" >> {}".format(sdk_profile_path))

## STEP 5: display success message and enable environment
print("[5]: Success!!! Standalone SDK is Install at {}".format(freertos_sdk_path))
print("[5]: SDK Environment Variables is in {}".format(sdk_profile_path))
print("[5]: Phytium FREERTOS SDK Setup Done for {}!!!".format(usr))
print("[5]: Input 'source {}' or Reboot System to Active SDK".format(sdk_profile_path))

