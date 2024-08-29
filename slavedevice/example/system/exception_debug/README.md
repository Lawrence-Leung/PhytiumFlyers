# exception debug

## 1. 例程介绍

本例程示范了freertos环境下测试Synchronous exception和Asynchronous exception

menuconfig选项中有如下配置选择：

EXCEPTION_INVALID_INSTRUCTION：Invalid instruction，未定义指令

EXCEPTION_ACCESS_PERMISSION_ERROR：Memory access permission error，写只读内存区域

EXCEPTION_ACCESS_VIOLATION：Memory access violation，越界访问地址空间

其中Synchronous exception包含Invalid instruction和Memory access permission error两种测试，Asynchronous exception包含Memory access violation一种测试

注意aarch32和aarch64下，其异常分类有所差异，具体查看手册

## 2. 如何使用例程

本例程需要用到

- Phytium开发板（FT2000-4/D2000/E2000D/E2000Q/PHYTIUMPI）
- [Phytium freeRTOS SDK](https://gitee.com/phytium_embedded/phytium-free-rtos-sdk)
- [Phytium standalone SDK](https://gitee.com/phytium_embedded/phytium-standalone-sdk)

### 2.1 硬件配置方法

本例程支持的硬件平台包括

- FT2000-4
- D2000
- E2000D
- E2000Q
- PHYTIUMPI

对应的配置项是，

- CONFIG_TARGET_FT2004
- CONFIG_TARGET_D2000
- CONFIG_TARGET_E2000D
- CONFIG_TARGET_E2000Q
- CONFIG_TARGET_PHYTIUMPI

### 2.2 SDK配置方法

本例程需要，

- 使能Shell

对应的配置项是，

- CONFIG_USE_LETTER_SHELL

本例子已经提供好具体的编译指令，以下进行介绍:

- make 将目录下的工程进行编译
- make clean  将目录下的工程进行清理
- make image   将目录下的工程进行编译，并将生成的elf 复制到目标地址
- make list_kconfig 当前工程支持哪些配置文件
- make menuconfig   配置目录下的参数变量
- make backup_kconfig 将目录下的sdkconfig 备份到./configs下

具体使用方法为:

- 在当前目录下
- 执行以上指令

### 2.3 构建和下载

#### 2.3.1 构建过程

- 在host侧完成配置

> 配置成ft2004，对于其它平台，使用对于的默认配置，如D2000 `make load_kconfig LOAD_CONFIG_NAME=d2000_aarch64_TEST_exception`

- 选择目标平台

```
make load_kconfig LOAD_CONFIG_NAME=e2000d_aarch64_demo_exception
```

- 选择例程需要的配置

```
make menuconfig
```

- 编译并将编译出的镜像放置到tftp目录下

```
make image
```

#### 2.3.2 下载过程

- host侧设置重启host侧tftp服务器

```
sudo service tftpd-hpa restart
```

- 开发板侧使用bootelf命令跳转

```
setenv ipaddr 192.168.4.20  
setenv serverip 192.168.4.50 
setenv gatewayip 192.168.4.1 
tftpboot 0x90100000 freertos.elf
bootelf -p 0x90100000
```

### 2.4 输出与实验现象

## 3. 如何解决问题
