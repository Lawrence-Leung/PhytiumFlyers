include $(PROJECT_DIR)/sdkconfig

LIBC_CSRCS := strto.c \
			  system_call.c

ifeq ($(CONFIG_ENABLE_CXX),y)
LIBC_CXXSRCS := libcxx_stub.cpp
endif

CSRCS_RELATIVE_FILES := $(LIBC_CSRCS)
ifeq ($(CONFIG_ENABLE_CXX),y)
CXXSRCS_RELATIVE_FILES := $(LIBC_CXXSRCS)
endif

#LIBS_NAME ?= lib_syscall

include $(SDK_DIR)/tools/build/compiler.mk