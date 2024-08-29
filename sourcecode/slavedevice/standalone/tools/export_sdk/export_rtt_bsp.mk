SRC_DIR += $(PROJECT_DIR)/port \
		   $(PROJECT_DIR)/port/arch
INC_DIR += $(PROJECT_DIR)/port \
		   $(PROJECT_DIR)/port/arch

ifdef CONFIG_TARGET_ARMV8_AARCH64
SRC_DIR += $(PROJECT_DIR)/port/arch/armv8/aarch64
INC_DIR += $(PROJECT_DIR)/port/arch/armv8/aarch64 

endif

ifdef CONFIG_TARGET_ARMV8_AARCH32
SRC_DIR += $(PROJECT_DIR)/port/arch/armv8/aarch32
INC_DIR += $(PROJECT_DIR)/port/arch/armv8/aarch32 
endif

# remove aarch64 source code
EXCL_SRC += $(STANDALONE_SDK_ROOT)/arch/armv8/aarch64/gcc/fboot.S \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch64/gcc/fcrt0.S \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch64/gcc/fgcc_debug.c \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch64/gcc/fsmccc_call.S \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch64/gcc/ftlb.S \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch64/gcc/ftrace_uart.S \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch64/gcc/fvectors_g.c \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch64/gcc/fvectors.S \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch64/faarch64.c \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch64/fcache.c \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch64/fexception.c \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch64/fgeneric_timer.c \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch64/fmmu.c \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch64/fpsci.c

EXCL_INC += $(STANDALONE_SDK_ROOT)/arch/armv8/aarch64/fcache.h \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch64/faarch64.h \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch64/fexception.h \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch64/fgeneric_timer.h \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch64/fmmu.h \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch64/farm_smccc.h \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch64/fpsci.h

# remove aarch32 source code
EXCL_SRC += $(STANDALONE_SDK_ROOT)/arch/armv8/aarch32/gcc/fboot.S \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch32/gcc/fcp15.S \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch32/gcc/fcrt0.S \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch32/gcc/fsmcc_call.S \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch32/gcc/ftrace_uart.S \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch32/gcc/fvector.S \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch32/fcache.c \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch32/fexception.c \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch32/fgeneric_timer.c \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch32/fmmu.c \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch32/fpsci.c

EXCL_INC += $(STANDALONE_SDK_ROOT)/arch/armv8/aarch32/fcache.h \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch32/fcp15.h \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch32/fexception.h \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch32/fgeneric_timer.h \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch32/fmmu.h \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch32/fpsci.h \
			$(STANDALONE_SDK_ROOT)/arch/armv8/aarch32/fsmc.h

EXCL_SRC += $(STANDALONE_SDK_ROOT)/common/finterrupt.c \
			$(STANDALONE_SDK_ROOT)/common/fsleep.c
EXCL_INC += $(STANDALONE_SDK_ROOT)/common/finterrupt.h \
			$(STANDALONE_SDK_ROOT)/common/fsleep.h

EXCL_SRC += $(STANDALONE_SDK_ROOT)/soc/common/fearly_uart.c
EXCL_INC += $(STANDALONE_SDK_ROOT)/soc/common/fearly_uart.h

include $(STANDALONE_SDK_ROOT)/make/build_baremetal.mk

# Commit hash from git
SDK_COMMIT=$(shell git rev-parse HEAD)
SDK_BRANCH=$(shell git symbolic-ref --short HEAD)
SDK_EXPORT_DATE=$(shell date '+%Y-%m-%d %H:%M:%S')
SDK_SRC_INC_PATH := $(STANDALONE_DIR)/soc \
					$(STANDALONE_DIR)/drivers \
					$(STANDALONE_DIR)/common \
					$(STANDALONE_DIR)/doc

SDK_EXPORT_PATH := $(abspath $(PROJECT_DIR)/rtt_bsp)

# remove previous rtt bsp if exists
.rm_rtt_bsp:
	$(shell if [ -e $(SDK_EXPORT_PATH) ];then rm $(SDK_EXPORT_PATH) -rf; fi)

# create directory to export rtt bsp
.create_rtt_bsp:
	@echo $(SDK_SRC_INC_FILES) | tr ' ' '\n'
	@$(shell mkdir -p $(SDK_EXPORT_PATH))

# copy source code to rtt bsp
.copy_rtt_bsp:
	@echo "*****************************"
	@echo "Source code from: " $(STANDALONE_DIR)
	@echo "Export code to: " $(SDK_EXPORT_PATH)
	@$(foreach dir, $(SDK_SRC_INC_PATH), if [ -d $(dir) ]; then mkdir -p $(dir) && cp -R $(dir) $(dir:$(STANDALONE_DIR)/%=$(SDK_EXPORT_PATH)/%);fi;)	
	@$(shell cp -f $(STANDALONE_DIR)/LICENSE $(SDK_EXPORT_PATH)/LICENSE)
	@$(shell cp -f $(STANDALONE_DIR)/README.md $(SDK_EXPORT_PATH)/README.md)
	@$(shell cp -R $(PROJECT_DIR)/port $(SDK_EXPORT_PATH))

# delete some of the source code
.filter_rtt_bsp:
	@$(shell rm $(SDK_EXPORT_PATH)/soc/d2000/fparameters.c -f)
	@$(shell rm $(SDK_EXPORT_PATH)/soc/e2000/fparameters.c -f)
	@$(shell rm $(SDK_EXPORT_PATH)/soc/ft2004/fparameters.c -f)
	@$(shell rm $(SDK_EXPORT_PATH)/soc/common/fsmp.* -f)
	@$(shell rm $(SDK_EXPORT_PATH)/common/finterrupt.* -f)
	@$(shell rm $(SDK_EXPORT_PATH)/common/fsleep.* -f)

# building and export
export_rtt_sdk: all .rm_rtt_bsp .create_rtt_bsp .copy_rtt_bsp .filter_rtt_bsp
	@echo "*****************************"
	@$(shell echo [commit-id]: $(SDK_COMMIT) >> $(SDK_EXPORT_PATH)/gitinfo)
	@$(shell echo [branch]: $(SDK_BRANCH) >> $(SDK_EXPORT_PATH)/gitinfo)
	@$(shell echo [date]: $(SDK_EXPORT_DATE) >> $(SDK_EXPORT_PATH)/gitinfo)
	@echo Export success !!! [$(SDK_EXPORT_DATE)] $(SDK_BRANCH):$(SDK_COMMIT)
	@echo "*****************************"