# 指定工程项目根目录为当前（只能指定一个目录）
export PROJECT_DIR 	?= .
# 用户添加的源文件夹和头文件夹（可以指定多个）
export USR_SRC_DIR  ?= . \
					   ./src
export USR_INC_DIR  ?= . \
					   ./inc

include ./sdkconfig

ifdef CONFIG_EXPORT_TYPE_RTT
# 指定编译项目使用的makefile
include ./export_rtt_bsp.mk
endif
