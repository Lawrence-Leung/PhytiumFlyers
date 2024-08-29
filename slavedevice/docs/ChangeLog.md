# Phytium Standalone SDK 2023-09-26 ChangeLog

Change Log since 2023-09-21

## example

- fix i2c bug

## drivers

- remove define of D2000、FT2004

# Phytium FreeRTOS SDK 2023-09-21 ChangeLog

Change Log sinc 2023-09-20

# example

- modify readme

# Phytium FreeRTOS SDK 2023-09-20 ChangeLog

Change Log sinc 2023-09-18

# example

- modify readme

# Phytium FreeRTOS SDK 2023-09-18 ChangeLog

Change Log sinc 2023-09-18

# example

- update Readme.md of something example including:lwip_startup,udp_multicast and timer_tacho.
- delete redundant statements in makefile of timer_tacho.

# Phytium FreeRTOS SDK 2023-09-18 ChangeLog

Change Log sinc 2023-09-15

# example

- add PHYTIUMPI readme of ddma, gdma, gpio, sdio, openamp, excepyion_debug, nested_interrupt examples


# Phytium FreeRTOS SDK 2023-09-15 ChangeLog

Change Log sinc 2023-09-15

# example

- Fix resource test example

# Phytium FreeRTOS SDK 2023-09-15 ChangeLog

Change Log sinc 2023-09-08

# example

- modify README.md of some examples

# Phytium FreeRTOS SDK 2023-09-08 ChangeLog

Change Log sinc 2023-09-07

# example

- change all configs of default
- rebuild template example

# README.md

- modify example of support
- add PHYTIUMPI declaration

# Phytium FreeRTOS SDK 2023-09-07 ChangeLog

Change Log sinc 2023-09-06

# example

- change the i2s example to adapt the phytiumpi, modify some bugs 

# driver

- modify the i2s driver
# Phytium FreeRTOS SDK 2023-09-06 ChangeLog

Change Log sinc 2023-09-01

# example

- change the i2s example to adapt the phytiumpi, modify some bugs 

# driver

- modify the i2s driver
# Phytium FreeRTOS SDK 2023-09-06 ChangeLog

Change Log sinc 2023-09-01

# example

- the new compilation framework adaptation in can, pwm, qspi, spi, fatfs, spi_spiffs, qspi_spiffs, atomic

# Phytium FreeRTOS SDK 2023-09-06 ChangeLog

Change Log sinc 2023-09-06

# example

- the new compilation framework adaptation in gdma, ddma, sdio, amp, atomic, exception_debug, nested_interrupt

# Phytium FreeRTOS SDK 2023-09-06 ChangeLog

Change Log sinc 2023-09-01

# example

- update configs list for the new compilation framework and add configs for phytiumpi about usb and media .

# Phytium FreeRTOS SDK 2023-09-01 ChangeLog

Change Log sinc 2023-09-01

# example
- update configs list for the new compilation framework and add configs for phytiumpi about network.
- delete the source code about the test board,and only support e2000 demo board now.
- update readme.md of network example and timer_tacho example.
- add phytiumpi configs about timer_tacho,and add iomux support for phytiumpi board.

# Phytium FreeRTOS SDK 2023-09-01 ChangeLog

Change Log sinc 2023-08-31

# example

- add new default configs

# Phytium FreeRTOS SDK 2023-08-31 ChangeLog

Change Log sinc 2023-08-30
## example

- Change the storage examples makefile and default configs

## third-party

- Change the fatfs src.mk and include.mk 

# Phytium FreeRTOS SDK 2023-08-31 ChangeLog

Change Log sinc 2023-08-29
## drivers

- Change the makefile , and *.mk to use the relative addr

## example

- Change the peripheral makefile and Kconfig , compile the all example 

## third-party

- Change the lvgl , cherryusb and sdmmc makefile and Kconfig 
- Change the third-party/include.mk , thirdparty.mk

# Phytium FreeRTOS SDK 2023-08-29 ChangeLog

Change Log sinc 2023-08-28

## drivers

- add *.mk

## example

- update network/lwip_startup

# Phytium FreeRTOS SDK 2023-08-28 ChangeLog

Change Log sinc 2023-08-20

## example 

- adapt to new frameworks
- adapt feature, qspi, atomic 

# Phytium FreeRTOS SDK 2023-07-14 ChangeLog

Change Log sinc 2023-07-10

## example 

- add template_new example to show new complier use
## third-party

- adapt standalone sdk new complier
# Phytium FreeRTOS SDK 2023-07-18 ChangeLog

Change Log sinc 2023-07-06

## third-party

- adapt the sdk and change the lvgl config

## example

- change the test example, the whole work process may be set as: cmd->task creat-> driver and third-party config-> demo

# Phytium FreeRTOS SDK 2023-07-06 ChangeLog

Change Log sinc 2023-07-04

## third-party

- update freertos kernel version to v10.5.1


# Phytium FreeRTOS SDK 2023-07-03 ChangeLog

Change Log sinc 2023-07-02

## third-party

- letter-shell adapt to new psci api 

# Phytium FreeRTOS SDK 2023-06-28 ChangeLog

Change Log sinc 2023-6-27

## example

- add atomic test example

# Phytium FreeRTOS SDK 2023-06-26 ChangeLog

Change Log sinc 2023-6-24

## third-party

- remove freertos boot.s, use fboot.s in standalone folder

# Phytium FreeRTOS SDK 2023-06-19 ChangeLog

Change Log sinc 2023-6-7

## driver

- eth/xmac: add new function FXmacRecvSemaphoreHandler,which can notify the receiving thread of incoming packets.
- eth/xmac: add new macro definition FXMAC_OS_CONFIG_RX_POLL_RECV,which can select poll mode.
- add new code logic,which can disable the RXCOMPL interrupt.
- modify interrupt registration function:FXmacRecvHandler->FXmacRecvSemaphoreHandler,which means using interrupt mode to notify the rx thread of the receipt of the packet.
- the value of max_fr_size have been modified:FXMAC_MAX_FRAME_SIZE-18->FXMAC_MAX_FRAME_SIZE.
## example

- update network/lwip_startup sdkconfig: CONFIG_LWIP_TCPIP_TASK_PRIO 6->5.
- the above changes will ensure a more stable and higher bandwidth when we run our iperf tests.
- all sdkconfigs and *_aarch*_eg_configs have been updated while using tools: build_all.

## third-party

- add new callback function ethernetif_poll,which can poll network packets.
- modified the initial value of the member variable mtu of netif,which are more in line with the theoretical definition of mtu.

# Phytium FreeRTOS SDK 2023-06-06 ChangeLog

Change Log sinc 2023-6-5

## example

-fix generic timer operation in wdt example

## make

- modify aarch32/gcc/fvectors.S filename in standalone_dependence.mk

## third-party

-fix generic timer operation in freertos_configs.c

# Phytium FreeRTOS SDK 2023-05-09 ChangeLog

Change Log sinc 2023-4-18

## example

-add lvgl indev driver and modify the format the whole media example

# Phytium FreeRTOS SDK 2023-4-18 ChangeLog

Change Log sinc 2023-4-10

## third-party

- freertos/portable modify config for the priority icc_pmr set and icc_rpr get

# Phytium FreeRTOS SDK 2023-4-10 ChangeLog

Change Log since 2023-03-30

## example

- lwip instructions has been updated by which we can choose driver type manually.
- update README.md : add new description about lwip probe instructions.

# Phytium FreeRTOS SDK 2023-4-7 ChangeLog

Change Log sinc 2023-3-16

## third-party

- update cherryusb to 0.8.0
- modify cherryusb (ready to merge to cherryusb baseline)
    -  reconstruct xhci driver
    -  modify usbh_bus usage to support use multiple usb controller
    -  modify enumration proccedure to support enumrate usb 3.0 device (e.g mass storage)  
    -  implment for usb 3.0 hub

## example

- modify cherryusb host example
- modify fatfs usb mass storage part

# Phytium FreeRTOS SDK 2023-4-3 ChangeLog

Change Log sinc 2023-3-15

## driver

- driver/media add the multi-display driver

## example 

- media add  config and example for the multi-display

## third-party

- lvgl-8.3/port modify config for the multi-display

# Phytium FreeRTOS SDK 2023-3-16 ChangeLog

Change Log sinc 2023-3-15

## example

- update sdkconfig about lwipstartup and udp_multicast

## driver

- convert data type of config->irqnum from u32 to int

## third-party

- delete redundant config in lwip-2.1.2/kconfig

# Phytium FreeRTOS SDK 2023-3-15 ChangeLog

Change Log sinc 2023-3-6

## example

- Add exception debug example
- Move amp and nested intr example to system folder

## make

- Remove Copyright in complier.mk

## third-party

- Add aarch64 Serror exception vector and handler function

# Phytium FreeRTOS SDK 2023-3-9 ChangeLog

Change Log sinc 2023-3-2

## example

Adapt OpenAMP routines based on e2000D/Q

# Phytium FreeRTOS SDK 2023-3-2 ChangeLog

Change Log sinc 2023-2-27

## driver

- eth/xmac delete conditional compilation statements about NO_SYS

## example

- modify network/lwip_startup configs and sdkconfig

## third-party

- lwip-2.1.2/ports/* delete conditional compilation statements about NO_SYS
- lwip-2.1.2/ports/fgmac ethernetif_init function name modified
- update lwip-2.1.2/kconfig

# Phytium FreeRTOS SDK 2023-3-1 ChangeLog

Change Log sinc 2023-2-23

## example

- add nested_interrupt example

## third-party

- modify freertos aarch32 and aarch64 port function

# Phytium FreeRTOS SDK 2023-2-20 ChangeLog

Change Log sinc 2023-2-16

## example

- add spim_spiffs example

## third-party

- modify sfud and delete repetitive code

# Phytium FreeRTOS SDK 2023-2-10 ChangeLog

Change Log sinc 2023-2-8

## example

- modify configs of cherryusb_host

## third-party

- update cherryusb from v0.6.0 to v0.7.0
- modify Kconfig and makefile files

# Phytium FreeRTOS SDK 2023-2-9 0.4.0 ChangeLog

Change Log sinc 2023-2-6

## example

- freertos_feature/queue readme update
- peripheral/spi readme update
- peripheral/spi e2000d_aarch32_eg_configs update(CONFIG_SFUD_CTRL_FSPIM=y)

# Phytium FreeRTOS SDK 2023-2-8 ChangeLog

Change Log sinc 2023-2-5

## example

- modify qspi_spiffs example
- modify qspi example

## driver

- modify can example

# Phytium FreeRTOS SDK 2023-2-6 ChangeLog

Change Log sinc 2023-1-30

## driver

- add developer information in file header
- add file description in file header
- all .c .h file format update
- print interface check
- print statement syntax checking and punctuation supplementation

## example

- all example xxxx_eg_configs update
- all example sdkconfig sdkconfig.h update
- add developer information in file header
- add file description in file header
- all .c .h file format update
- print interface check
- print statement syntax checking and punctuation supplementation

## third-party

- add developer information in file header
- add file description in file header
- all .c .h file format update
- third-party/lwip-2.1.2/ports/arch/cc.h modified
- print interface check
- print statement syntax checking and punctuation supplementation

## install.py

- modefiy the standalone_sdk_v、standalone_branche、standalone_remote value
- add script statements which can delete standalone/third-party/lwip-2.1.2/ports/arch dir

# Phytium FreeRTOS SDK 2023-1-6 ChangeLog


Change Log sinc 2023-1-5

## third-party

add lvgl and modify the third-party.mk and the Kconfig

## example

## driver

add the media example and driver,modify the corresponding config

# Phytium FreeRTOS SDK 2023-1-5 ChangeLog

Change Log sinc 2023-1-3

## example

- network part adjust. add new example lwip_startup
- delete xmac_lwip_test.
- delete gmac_lwip_test.

## third-party

- lwip-2.1.2 ports part adjust.
- delete lwip-2.1.2/api.
- delete lwip-2.1.2/apps.
- delete lwip-2.1.2/core.

## drivers

- add fgmac_os.
- add fxmac_os.

# Phytium FreeRTOS SDK 2023-1-3 ChangeLog

Change Log sinc 2022-12-28

## third-party

- Fix freertos interrupt priority get and mask function in port.c

# Phytium FreeRTOS SDK 2022-12-28 ChangeLog

- add sata fatfs_0.1.4 port
- delete fatfs_0.1.3 content
- delete storage/sata_fatfs content

# Phytium FreeRTOS SDK 2022-12-7 ChangeLog

Change Log sinc 2022-12-6

## third-party

- Add FPU support by configUSE_TASK_FPU_SUPPORT in kernel configuration

# Phytium FreeRTOS SDK 2022-12-6 ChangeLog

Change Log sinc 2022-12-6

## example

## driver

- Adapt fparameters.h in standalone sdk

# Phytium FreeRTOS SDK 2022-11-28 ChangeLog

Change Log since 2022-11-25

## example

- add fatfs tests (usb/sdio)

## driver

- remove mmc driver, its implementation has been moved to sdmmc ports

## third-party

- add fatfs 0.1.4 freertos port
- add sdmmc 1.0 freertos port

# Phytium FreeRTOS SDK 2022-11-25 ChangeLog

Change Log sinc 2022-11-17

## example

- add i2c example
- add timer_tacho example

## driver

- add i2c os driver
- add timer_tacho driver

# Phytium FreeRTOS SDK 2022-11-17 ChangeLog

Change Log sinc 2022-11-16

## third-party

- Add kernel configuration in menuconfig
- Rename the "FreeRTOS Setting" to "Third-Party Configuration" in menuconfig
- Rename cmd_os_stats.c to cmd_ps.c

# Phytium FreeRTOS SDK 2022-11-11 0.3.0 ChangeLog

Change Log sinc 2022-11-11

## example

- task/task_notify e2000q support
- modify some README.md description
- modify openamp & freertos_feature\task & storage\sata_fatfs readme.md
- modify freertos_feature\eventgroup interrupt queue resource software_timer readme.md and picture
- remove e2000q adc support
- modify some README.md description

# Phytium FreeRTOS SDK 2022-11-01 ChangeLog

Change Log sinc 2022-11-1

## driver

- add USE_SPI slection in Kconfig

## example

- add fspim_spiffs example

## third-party

- sfud debug
- add spiffs port for spim

# Phytium Standalone SDK 2022-10-31 ChangeLog

Change Log sinc 2022-10-31

- rename some include file with f prefix

# Phytium FreeRTOS SDK 2022-10-21 ChangeLog

Change Log sinc 2022-10-15

## example

- add e2000q example

## third-party

- Adapt fatfs to e2000 demo board for sata
- Add the lwip configuration kconfig
- Add udp multicast function

# Phytium FreeRTOS SDK 2022-10-10 ChangeLog

Change Log sinc 2022-09-23

## example

- add cherryusb host example

## third-party

- add cherryusb 0.5.2 and port to xhci

# Phytium FreeRTOS SDK 2022-09-23 ChangeLog

Change Log sinc 2022-09-15

## driver

- add can os driver

## example

- add can test test example

# Phytium FreeRTOS SDK 2022-09-07 ChangeLog

Change Log sinc 2022-08-30

## driver

- optimize qspi os driver adapt to sfud and spiffs

## example

- add sata controller fatfs test example

## third-party

- modify qspi sfud use qspi os driver interface functions
- add sata controller fatfs port diskio

# Phytium FreeRTOS SDK 2022-08-29 ChangeLog

Change Log sinc 2022-08-24

## driver

- add adc driver

## example

- add adc test example

# Phytium FreeRTOS SDK 2022-08-10 0.2.2 ChangeLog

Change Log sinc 0.2.1

## driver

- add gpio freertos driver
- add spi freertos driver
- add ddma freertos driver
- add gdma freertos driver
- add sdio freertos driver

## example

- add gpio, gdma, ddma example
- add spi nor flash, tf emmc example

## third-party

- add tlsf to implement memory pool
- add sfud to support nor flash
- add sdmmc to support tf/emmc

# Phytium FreeRTOS SDK 2022-08-18 ChangeLog

Change Log sinc 2022-08-16

## driver

- add pwm driver

## example

- add pwm test example

# Phytium FreeRTOS SDK 2022-08-10 0.2.1 ChangeLog

Change Log sinc 0.2.0

## README

- add E2000D/S description

# Phytium FreeRTOS SDK 2022-08-09 0.2.0 ChangeLog

Change Log sinc 2022-07-29

## driver

- modify wdt driver

## example

- add e2000d support
- delete startup and helloworld example

## third-party

- fix qspi spiffs and sata fatfs driver
- fix shell cmd

# Phytium FreeRTOS SDK 2022-08-08 ChangeLog

Change Log sinc 2022-07-14

# driver

- add fxmac driver

## example

- add xmac_lwip_test example

## third-party

- add fxmac ports

# Phytium FreeRTOS SDK 2022-07-26 ChangeLog

Change Log sinc 2022-07-18

# driver

- modify wdt freertos driver

## third-party

- support qspi spiffs
- support sata fatfs

## example

- add usage example for qspi spiffs and sata fatfs

# Phytium FreeRTOS SDK 2022-07-14 ChangeLog

Change Log sinc 2022-07-06

# driver

- add qspi read and write freertos driver
- add wdt freertos driver

## example

- add usage example for qspi and wdt freertos driver

# Phytium FreeRTOS SDK 2022-07-05 ChangeLog

Change Log sinc 2022-07-01

## example

- add usage example for freertos function, include task, interrupt, queue, resource,and so on.

# Phytium FreeRTOS SDK 2022-06-18 0.1.0 ChangeLog

Change Log sinc 2022-05-30

## example

- adapt to the new standalone sdk v0.2.0
- Restruct lwip_test example

## third-party

- Restruct lwip gmac adapter, add gmac port

# Phytium FreeRTOS SDK 0.0.7 ChangeLog

Change Log sinc 2022-03-21,2022-04-20

## example

- adapt to the new standalone sdk v0.1.17
- add Linux OpenAMP example for freertos

## make

- Modified some variable positions and added some configuration related variables

# Phytium FreeRTOS SDK 2022-03-21 ChangeLog

Change Log sinc 0.0.6, 2022.03.21

## example

- adapt to the new standalone sdk v0.1.16
- add OpenAMP for FreeRTOS

## third-party

- add OpenAMP for freertos application
- add Letter_shell for freertos

## LICENSE

- replace LICENSE with Phytium Public License 1.0 (PPL-1.0)
- update file COPYRIGHT declaration with PPL-1.0

# Phytium FreeRTOS SDK v0.0.6 ChangeLog

Change Log sinc 0.0.5, 2021.12.23

## example

- adapt to the new standalone sdk v0.1.15
- reconstruct the aarch framework for freertos

# Phytium FreeRTOS SDK v0.0.5 ChangeLog

Change Log sinc 0.0.4, 2021.11.2

## example

- add freertos function test examples
- change Compile environment and installation script

# Phytium FreeRTOS SDK v0.0.4 ChangeLog

Change Log sinc 0.0.3, 2021.9.24

## example

- add aarch32 example
- add aarch64 example
- add lwip_test example

## third-party

- add lwip for freertos application
- import freertos v0.0.4 source code
- add ft20004/d2000 lwip code

# Phytium FreeRTOS SDK v0.0.1 ChangeLog

Change Log sinc init

## drivers

- port PL110 uart driver to FreeRTOS

## example

- add aarch32 example
- add aarch64 example
- add qemu-aarch32 example

## make

- import makefile scripts

## scripts

- add arm-linux cc install script for qemu

## third-party

- add simple bootloader for qemu application
- import freertos v10.0.1 source code
- add ft20004/e2000/qemu port code
