# 1. 使用方法

## 1.1 新建一个 freertos 应用工程

### 1.1.1 选择工程模板

- 进入`[FREERTOS_SDK_DIR]/example/template`目录，作为 freertos 应用工程

```
$ ls
Kconfig       --> 应用工程配置menu文件
makefile      --> makefile
main.c        --> 包含main函数
sdkconfig     --> 配置输出
sdkconfig.h   --> 配置输出
inc           --> 用户头文件*
src           --> 用户源文件*
```

>请注意使用小写makefile，使用Makefile在部分平台不能被识别

### 1.1.2 选择目标平台

例子已经提供好具体的编译指令，以下进行介绍：
    1. make 将目录下的工程进行编译
    2. make clean  将目录下的工程进行清理
    3. make image   将目录下的工程进行编译，并将生成的elf 复制到目标地址
    4. make list_kconfig 当前工程支持哪些配置文件
    5. make load_kconfig LOAD_CONFIG_NAME=<kconfig configuration files>  将预设配置加载至工程中
    6. make menuconfig   配置目录下的参数变量
    7. make backup_kconfig 将目录下的sdkconfig 备份到./configs下

- make list_kconfig 当前工程支持哪些配置文件
- make load_kconfig LOAD_CONFIG_NAME=<kconfig configuration files>  将预设配置加载至工程中

>配置成E2000D，使用对应的默认配置，如E2000d 32位:

```
$ make load_kconfig LOAD_CONFIG_NAME=e2000d_aarch32_demo_i2c
```

- 编译应用工程, 生成`*.bin`或者`*.elf`文件用于下载到开发板(bin文件生成配置可选，使用make menuconfig 具体使用可参考standalone SDK手册)

- 在host侧完成构建

```
$ make clean image
e2000d_aarch32_demo_eg.bin   --> 二进制文件
e2000d_aarch32_demo_eg.elf   --> ELF文件
e2000d_aarch32_demo_eg.map   --> 内存布局文件
```

## 1.2 下载镜像跳转启动

### 1.2.1 在 host 侧（Ubuntu 20.04）配置 tftp 服务

- 在开发环境`host`侧安装`tftp`服务

```
sudo apt-get install tftp-hpa tftpd-hpa
sudo apt-get install xinetd
```

- 新建 tftboot 目录, `/mnt/d/tftboot`, 确保 tftboot 目录有执行权限`chmod 777 /**/tftboot`

- 配置主机 tftpboot 服务, 新建并配置文件`/etc/xinetd.d/tftp`

```
# /etc/xinetd.d/tftp

server tftp
{
  socket_type = dgram
  protocol = udp
  wait = yes
  user = root
  server =  /usr/sbin/in.tftpd
  server_args = -s /mnt/d/tftboot
  disable = no
  per_source = 11
  cps = 100 2
  flags = IPv4
}
```

- 启动主机`tftp`服务，生成默认配置

```
$ sudo service tftpd-hpa start
```

- 修改主机`tftp`配置，指向`tftboot`目录
  修改/etc/default/tftpd-hpa

```
$ sudo nano /etc/default/tftpd-hpa
# /etc/default/tftpd-hpa

TFTP_USERNAME="tftp"
TFTP_DIRECTORY="/mnt/d/tftboot"
TFTP_ADDRESS=":69"
TFTP_OPTIONS="-l -c -s"
```

- 重启主机`tftp`服务

```
$ sudo service tftpd-hpa restart
```

- 测试主机`tftp`服务的可用性
  > 登录`tftp`服务，获取`tftboot`目录下的一个文件

```
$ tftp 192.168.4.50
tftp> get test1234
tftp> q
```

### 1.3.2 在 host 侧（Windows）配置 tftp 服务

- 将2.1.1章下载的`tftp.zip`解压到开发环境，如`D:\phytium-dev\tftp`
- 以**管理员权限**打开Windows cmd，进入`D:\phytium-dev\tftp`，运行`.\reinstall.cmd`完成Tftpd服务安装

![输入图片说明](https://images.gitee.com/uploads/images/2021/1013/193051_97e9949e_8736513.png "管理员权限打开.png")

![输入图片说明](https://images.gitee.com/uploads/images/2021/1014/111121_2be64492_8736513.png "屏幕截图.png")

- 之后每次使用前，进入Windows服务，手动将一下服务打开

![输入图片说明](https://images.gitee.com/uploads/images/2021/1013/152422_b398ef37_8736513.png "启动tftpd服务.png")

- 进入SDK，双击`D:\phytium-dev\phytium-free-rtos-sdk`目录下的`run_tftd.cmd`，启动tftp工具，设置tftp目录和ip

![输入图片说明](https://images.gitee.com/uploads/images/2021/1013/152524_2db6cb31_8736513.png "tftpd配置工具.png")

- 将镜像文件放置在上图所示的`%PHYTIUM_IDE_PATH%\tftp`目录下，开发板即可通过`tftpboot`加载镜像

> 在`template_mingw64`工程中，通过定义`USR_BOOT_DIR`可以将编译的镜像自动拷贝带tftp目录下

### 1.3.3 配置开发板 ip，连通 host 下载启动镜像

- 将`BIN`文件或者`ELF`文件复制到`tftboot`目录
- 此步骤目前已经集成到`make image`阶段，可以省略，如果未成功，可进行手动复制

```
$ cp ./freertos.bin /mnt/d/tftboot
```

```
$ cp ./freertos.elf /mnt/d/tftboot
```

- 连通开发板串口，进入`u-boot`界面，配置开发板`ip`，`host`侧`ip`和网关地址

```
# setenv ipaddr 192.168.4.20
# setenv serverip 192.168.4.50
# setenv gatewayip 192.168.4.1
```

> 镜像启动的地址为`0x80100000`, 对于`BIN`文件，需要直接加载到`0x80100000`，对于`ELF`文件，启动地址会自动获取，需要加载到`DRAM`中一段可用的地址，这里选择`0x90100000`

- 支持使用以下几种方式跳转启动(二选一)
- 1. `AARCH32/AARCH64`支持加载`BIN`文件到启动地址，刷新缓存后，通过`go`命令跳转启动

```
# tftpboot 0x80100000 freertos.bin
# dcache flush
# go 0x80100000
```

- 2. `AARCH32/AARCH64`支持加载`ELF`文件到`DRAM`，通过`bootelf`解析跳转启动

```
# tftpboot 0x90100000 freertos.elf
# bootelf -p 0x90100000
```