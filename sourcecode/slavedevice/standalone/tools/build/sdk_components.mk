
# Link script

LIBS_NAME ?= $(CONFIG_ARCH_EXECUTION_STATE)_ram
LIBS_FILE ?= $(CONFIG_ARCH_EXECUTION_STATE)_ram.ld.o

LIBS_OBJS = $(BUILD_OUT_PATH)/$(LIBS_FILE)

$(BUILD_OUT_PATH)/$(LIBS_FILE):
	@$(MAKE) -C $(SDK_DIR)/tools/build/ld SDK_DIR="$(SDK_DIR)" PROJECT_DIR="$(PROJECT_DIR)" LD_NAME="$(LIBS_NAME)" BUILD_OUT_PATH="$(BUILD_OUT_PATH)"  all

# Defines a multi-line macro called "invoke_make_in_directory" 
# Invokes Make in the specified directory, using the specified Makefile, and passing the variables and flags specified as arguments
# $(1) is the path of the module to compile
# $(2) the file that needs to be run by the makefile
# $(3) rules that need to be enforced
# $(4) is an optional parameter
define invoke_make_in_directory 
    @$(MAKE) -C $(SDK_DIR)/$(1) -f $(2) SDK_DIR="$(SDK_DIR)" PROJECT_DIR="$(PROJECT_DIR)" BUILD_OUT_PATH="$(BUILD_OUT_PATH)" $(4) $(3) LIBS_NAME=$@
endef

# drivers code
$(BUILD_OUT_PATH)/libdrivers.a: libdrivers.a
libdrivers.a: 
	$(call invoke_make_in_directory,drivers,makefile,all,)
libdrivers_debug:
	$(call invoke_make_in_directory,drivers,makefile,debug,)
libdrivers_info:
	$(call invoke_make_in_directory,drivers,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/libdrivers.a

# soc arch code
$(BUILD_OUT_PATH)/libarch.a: libarch.a
libarch.a: 
	$(call invoke_make_in_directory,arch/armv8,makefile,all,)
libarch_debug:
	$(call invoke_make_in_directory,arch/armv8,makefile,debug,)
libarch_info:
	$(call invoke_make_in_directory,arch/armv8,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/libarch.a

# common function
$(BUILD_OUT_PATH)/libcommon.a: libcommon.a
libcommon.a: 
	$(call invoke_make_in_directory,common,makefile,all,)
libcommon_debug:
	$(call invoke_make_in_directory,common,makefile,debug,)
libcommon_info:
	$(call invoke_make_in_directory,common,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/libcommon.a

# soc parameters
$(BUILD_OUT_PATH)/libsoc.a: libsoc.a
libsoc.a: 
	$(call invoke_make_in_directory,soc,makefile,all,)
libsoc_debug:
	$(call invoke_make_in_directory,soc,makefile,debug,)
libsoc_info:
	$(call invoke_make_in_directory,soc,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/libsoc.a

# libc code
$(BUILD_OUT_PATH)/lib_syscall.a: lib_syscall.a
lib_syscall.a:
	$(call invoke_make_in_directory,lib/libc,makefile,all,)
lib_syscall_debug:
	$(call invoke_make_in_directory,lib/libc,makefile,debug,)
lib_syscall_info:
	$(call invoke_make_in_directory,lib/libc,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_syscall.a

# archived static
include $(SDK_DIR)/tools/build/archived_libs.mk

ifdef CONFIG_USE_BAREMETAL
# thirdparty
include $(SDK_DIR)/third-party/thirdparty.mk
endif
