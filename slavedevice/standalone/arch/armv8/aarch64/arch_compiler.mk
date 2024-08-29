# 此文档主要决定编译过程中的优化调试选项，记忆CC 编译条件的选择
# 定义变量，分为 编译过程中的条件（CPU 架构、fpu）包含优化和调试的内容、链接过程中的条件、C库编译选择
#*
#* Copyright : (C) 2022 Phytium Information Technology, Inc.
#* All Rights Reserved.
#*
#* This program is OPEN SOURCE software: you can redistribute it and/or modify it
#* under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,
#* either version 1.0 of the License, or (at your option) any later version.
#*
#* This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;
#* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#* See the Phytium Public License for more details.
#*
#*
#* FilePath: arch.mk
#* Date: 2022-02-10 14:53:41
#* LastEditTime: 2022-02-17 17:36:17
#* Description:  This file is for phytium baremetal hello world
#*
#* Modify History:
#*  Ver   Who        Date         Changes
#* ----- ------     --------    --------------------------------------
#*

#
#	local value
#
#	TOOL_CHAIN_PREFIX The GNU toolchain triple
#	ARCH_OPTIMIZATION Optimization options for the compile process
#   ARCH_DEBUG	    Debug options for the compile process,
#					          optimized content for the code execution phase	
#   ARCH_CPU_MARCH	Tells the compiler to compile with a specific CPU architecture.
#   ARCH_CPU_FPU	  Fpu options for the compile process
#
#
#	Global value
#	/* toolchain */
#	  CC					C Compiler
#   CCX					C++ Compiler
#	  CPP					C Preprocessor
#   STRIP				Strip Symbol Tool
#	  OBJCOPY				Object Copy Tool
#   LD					Linker
# 	AR					Archive Tool
#	  NM					Name List Tool
# 	
# Output
#   LDFLAGS
#   CFLAG
#   CPPFLAG
#   AFLAGS
#   


# This compiler prefix is a standard prefix for bare-metal program or application development for the ARMv8-A architecture, and is commonly used in embedded systems development, operating system kernel development, and related fields.
TOOL_CHAIN_PREFIX	?= $(AARCH64_CROSS_PATH)/bin/aarch64-none-elf-

#  Sets the compiler option for the target CPU architecture to ARMv8-A. Specifically
ARCH_CPU_MARCH += -march=armv8-a

#  Sets the instruction extension for compiler
ifdef CONFIG_ARM_NEON
ARCH_CPU_MARCH := $(ARCH_CPU_MARCH)+simd
endif

ifdef CONFIG_ARM_CRC
ARCH_CPU_MARCH := $(ARCH_CPU_MARCH)+crc
endif

ifdef CONFIG_ARM_CRYPTO
ARCH_CPU_MARCH := $(ARCH_CPU_MARCH)+crypto
else
ARCH_CPU_MARCH := $(ARCH_CPU_MARCH)+nocrypto
endif

ifdef CONFIG_ARM_FLOAT_POINT
ARCH_CPU_MARCH := $(ARCH_CPU_MARCH)+fp
else
ARCH_CPU_MARCH := $(ARCH_CPU_MARCH)+nofp
endif

ARCH_CPU_MARCH += -mlittle-endian

# Set the code model for compiler
ifdef CONFIG_GCC_CODE_MODEL_TINY
ARCH_CPU_MARCH += -mcmodel=tiny
endif

ifdef CONFIG_GCC_CODE_MODEL_SMALL
ARCH_CPU_MARCH += -mcmodel=small
endif

ifdef CONFIG_GCC_CODE_MODEL_LARGE
ARCH_CPU_MARCH += -mcmodel=large
endif

# Optimization configuration
ifeq ($(CONFIG_DEBUG_CUSTOMOPT),y)
  ARCH_OPTIMIZATION += $(CONFIG_DEBUG_OPTLEVEL) # Add custom optimization 
else ifeq ($(CONFIG_DEBUG_FULLOPT),y)
  ARCH_OPTIMIZATION += -Os  # Full optimization is enabled, use -Os
else
  ARCH_OPTIMIZATION += -O0
endif


# Add -fno-strict-aliasing if optimization is not disabled
ifneq ($(CONFIG_DEBUG_NOOPT),y)
  ARCH_OPTIMIZATION += -fno-strict-aliasing
endif

# Full link-time optimization is enabled, use -flto
ifeq ($(CONFIG_LTO_FULL),y)
  ARCH_OPTIMIZATION += -flto
endif

# Add -pipe
ARCH_OPTIMIZATION += -pipe
ARCH_OPTIMIZATION += -ffreestanding

ARCH_OPTIMIZATION += -DGUEST

# Debug configuration
# Add debug flags for frame pointer
ifeq ($(CONFIG_FRAME_POINTER),y)
	ARCH_DEBUG += -fno-omit-frame-pointer -fno-optimize-sibling-calls
else
	ARCH_DEBUG += -fomit-frame-pointer
endif

# Enable stack canaries if enabled
ifeq ($(CONFIG_STACK_CANARIES),y)
  ARCH_DEBUG += -fstack-protector-all
endif

# Add coverage flags if enabled

ifeq ($(CONFIG_ARCH_COVERAGE_ALL),y)
  ARCH_DEBUG += -fprofile-generate -ftest-coverage
endif

# Add flags to optimize unused sections if enabled
ifeq ($(CONFIG_DEBUG_OPT_UNUSED_SECTIONS),y)
  ARCH_DEBUG += -ffunction-sections -fdata-sections
endif

# Add debug symbols if enabled
ifeq ($(CONFIG_DEBUG_SYMBOLS),y)
  ARCH_DEBUG += -g3
endif

# Enable all warnings if enabled
ifeq ($(CONFIG_DEBUG_ENABLE_ALL_WARNING),y)
  ARCH_DEBUG += -Wall
endif


ifeq ($(CONFIG_STRICT_PROTOTYPES),y)
  ARCH_DEBUG +=  -Wstrict-prototypes
endif

ifeq ($(CONFIG_WALL_WARNING_ERROR),y)
  ARCH_DEBUG += -Werror
endif

ifeq ($(CONFIG_DOWNGRADE_DIAG_WARNING),y)
# only valid for C/C++
  ARCH_CXX_DEBUG += -fpermissive
endif

# fpu
# ifeq ($(CONFIG_ARCH_FPU),y)
# ARCH_CPU_FPU := -mfpu=$(CONFIG_ARM_FPU_SYMBOL)
# ifeq ($(CONFIG_ARM_FPU_ABI_SOFT),y)
# ARCH_CPU_FPU += -mfloat-abi=softfp
# else
# ARCH_CPU_FPU += -mfloat-abi=hard
# endif
# else
# # default hard
# ARCH_CPU_FPU := 
# endif
ARCH_CPU_FPU := 

# linker
LDFLAGS += -nostartfiles

ifeq ($(CONFIG_DEBUG_OPT_UNUSED_SECTIONS),y)
  LDFLAGS += -Xlinker --gc-sections
endif


# default toolchain

CC = $(TOOL_CHAIN_PREFIX)gcc
CXX = $(TOOL_CHAIN_PREFIX)g++
CPP = $(TOOL_CHAIN_PREFIX)gcc -E -P -x c
STRIP = $(TOOL_CHAIN_PREFIX)strip --strip-unneeded
OBJCOPY = $(TOOL_CHAIN_PREFIX)objcopy
LD = $(TOOL_CHAIN_PREFIX)ld
AR = $(TOOL_CHAIN_PREFIX)ar rcs
NM = $(TOOL_CHAIN_PREFIX)nm
OD = $(TOOL_CHAIN_PREFIX)objdump
FILTER = $(TOOL_CHAIN_PREFIX)c++filt
# 

ARCHCFLAGS += -fno-common
ARCHCXXFLAGS += -fno-common


ifeq ($(CONFIG_ENABLE_WSHADOW),y)
ARCHCFLAGS += -Wshadow
ARCHCXXFLAGS += -Wshadow
endif

ifeq ($(CONFIG_ENABLE_WUNDEF),y)
ARCHCFLAGS += -Wundef
ARCHCXXFLAGS += -Wundef
endif


# libc

ifeq ($(CONFIG_USE_COMPILE_CHAIN),y)
  LIBC += -lc
  LIBM += -lm
  LIBGCC += -lgcc
  LIBCXX += -lstdc++ 
endif

ifeq ($(CONFIG_SELECT_CXX_98),y)
  ARCH_CXX_STD := -ansi
endif

ifeq ($(CONFIG_SELECT_CXX_11),y)
  ARCH_CXX_STD := -std=c++11
endif

ifeq ($(CONFIG_SELECT_CXX_14),y)
  ARCH_CXX_STD := -std=c++14
endif

CFLAGS   := $(ARCHCFLAGS) $(ARCH_OPTIMIZATION) $(ARCH_DEBUG) $(ARCH_CPU_MARCH) -std=gnu99	-pipe
CXXFLAGS	:= $(ARCHCXXFLAGS) $(ARCH_OPTIMIZATION) $(ARCH_DEBUG) $(ARCH_CXX_DEBUG) $(ARCH_CPU_MARCH) $(ARCH_CXX_STD)	-pipe
AFLAGS	:= $(CFLAGS)

# speficy explicitly the language for input files
ifeq ($(CONFIG_ENABLE_CXX),y)
AFLAGS  += -x assembler-with-cpp
endif