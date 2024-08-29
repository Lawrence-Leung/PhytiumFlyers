
FREERTOS_SDK_DIR := $(SDK_DIR)/..

# Defines a multi-line macro called "rtos_invoke_make_in_dir" 
# Invokes Make in the specified directory, using the specified Makefile, and passing the variables and flags specified as arguments
# $(1) is FreeRTOS root path
# $(2) is the path of the module to compile
# $(3) the file that needs to be run by the makefile
# $(4) rules that need to be enforced
# $(5) is an optional parameter
define rtos_invoke_make_in_dir 
    @$(MAKE) -C $(1)/$(2) -f $(3) SDK_DIR="$(SDK_DIR)" PROJECT_DIR="$(PROJECT_DIR)" BUILD_OUT_PATH="$(BUILD_OUT_PATH)" $(5) $(4) LIBS_NAME=$@
endef


$(BUILD_OUT_PATH)/lib_rtos_driver.a: lib_rtos_driver.a
lib_rtos_driver.a:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),drivers,makefile,all,)
lib_rtos_driver_debug:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),drivers,makefile,debug,)
lib_rtos_driver_info:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),drivers,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_rtos_driver.a


include $(FREERTOS_SDK_DIR)/third-party/thirdparty.mk

