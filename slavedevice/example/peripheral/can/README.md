# can base on freertos

## 1. 例程介绍

本例程示范了freertos环境下的can的使用，包括can的初始化、can周期发送，接收操作；
程序启动后，创建can初始化任务，设置can波特率，中断函数、id mask等；
创建can定时发送任务FFreeRTOSCanSendTask，用于定时发送can报文；
创建can接收任务FFreeRTOSCanRecvTask，用于接收can报文；
创建单次模式的软件定时器，回调函数为删除FFreeRTOSCanSendTask，删除FFreeRTOSCanRecvTask，去初始化can，删除软件定时器；
使用B板进行测试，选择can0和can1回环；

## 2. 如何使用例程

本例程需要用到
- Phytium开发板（E2000D/E2000Q/D2000/FT2000-4）
- [Phytium freeRTOS SDK](https://gitee.com/phytium_embedded/phytium-free-rtos-sdk)
- [Phytium standalone SDK](https://gitee.com/phytium_embedded/phytium-standalone-sdk)
### 2.1 硬件配置方法

本例程支持的硬件平台包括

- FT2000-4
- D2000
- E2000D
- E2000Q

对应的配置项是，

- CONFIG_TARGET_FT2004
- CONFIG_TARGET_D2000
- CONFIG_TARGET_E2000D
- CONFIG_TARGET_E2000Q

### 2.2 SDK配置方法

本例程需要，

- 使能Shell
- 使能Can

对应的配置项是，

- CONFIG_USE_LETTER_SHELL
- CONFIG_FREERTOS_USE_CAN
- CONFIG_USE_CAN

本例子已经提供好具体的编译指令，以下进行介绍:
- make 将目录下的工程进行编译
- make clean  将目录下的工程进行清理
- make image   将目录下的工程进行编译，并将生成的elf 复制到目标地址
- make list_kconfig 当前工程支持哪些配置文件
- make load_kconfig LOAD_CONFIG_NAME=<kconfig configuration files>  将预设配置加载至工程中
- make menuconfig   配置目录下的参数变量
- make backup_kconfig 将目录下的sdkconfig 备份到./configs下

具体使用方法为:
- 在当前目录下
- 执行以上指令

### 2.3 构建和下载

><font size="1">描述构建、烧录下载镜像的过程，列出相关的命令</font><br />

[参考 freertos 使用说明](../../../docs/reference/usr/usage.md)

#### 2.3.1 下载过程

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

- 系统进入后，创建can初始化任务，创建定时发送任务，创建接收任务，并创建单次模式软件定时器

![create](./figs/create.png)

- 定时器时间到，触发单次模式软件定时器的回调函数，删除发送和接收任务，去初始化can，删除软件定时器

![delete](./figs/delete.png)

## 3. 如何解决问题

- 使用can0和can1进行回环测试时，需要将can0和can1的H和L信号线分别进行短接

## 4. 修改历史记录




