# Read in dependencies to all Kconfig* files, make sure to run
# oldconfig if changes are detected.
# DO NOT USE CONFIG ITEM BEFORE INCLUDE KCONFIG
# specify menuconfig setting 
export MENUCONFIG_STYLE=aquatic
export KCONFIG_CONFIG=sdkconfig

CONFIGS_PATH_NAME=configs

CONFIGS_OUTPUT_PATH ?= $(PROJECT_DIR)

.PHONY: load_kconfig

# 红色字体
RED=\033[0;31m
# 绿色字体
GREEN=\033[0;32m
# 重置字体颜色
NC=\033[0m


#if sdkconfig not exits, prompt user to load default sdkconfig first
FILE_EXISTS = $(shell if [ -f ./$(KCONFIG_CONFIG) ]; then echo "exist"; else echo "notexist"; fi;)

ifneq ($(MAKECMDGOALS),lddefconfig)
ifeq (FILE_EXISTS, "notexist")
$(error error, please type in 'make lddefconfig DEF_KCONFIG=<file_name>' first!!)
endif

$(shell if [ ! -d $(KCONFIG_CONFIG) ]; then touch ./$(KCONFIG_CONFIG); fi)
include $(PROJECT_DIR)/$(KCONFIG_CONFIG)
else

ifndef DEF_KCONFIG
# make lddefconfig DEF_KCONFIG=qemu_aarch64_baremetal_defconfig
$(error error, please type in 'make lddefconfig DEF_KCONFIG=<file_name>'!!)
endif

endif

backup_kconfig:
	@mkdir -p $(CONFIGS_OUTPUT_PATH)/$(CONFIGS_PATH_NAME)
	@cp sdkconfig $(CONFIGS_OUTPUT_PATH)/$(CONFIGS_PATH_NAME)/$(CONFIG_SOC_NAME)$(CONFIG_TARGET_TYPE_NAME)_$(CONFIG_ARCH_EXECUTION_STATE)_$(CONFIG_BOARD_NAME)_$(CONFIG_TARGET_NAME).config
	@echo "${GREEN}The configuration file of $(CONFIG_SOC_NAME)$(CONFIG_TARGET_TYPE_NAME)_$(CONFIG_ARCH_EXECUTION_STATE)_$(CONFIG_BOARD_NAME)_$(CONFIG_TARGET_NAME) is successfully backed up.${NC}" 

# load_kconfig:
# 	$(shell if [ -z "$(filter-out load_kconfig,$(MAKECMDGOALS))" ]; then echo "Error: No configuration file specified.";exit 1; fi;)
# 	$(shell if [ ! -f "$(CONFIGS_OUTPUT_PATH)/configs/$(filter-out load_kconfig,$(MAKECMDGOALS)).config" ]; then echo "Error: Configuration file '$(filter-out load_kconfig,$(MAKECMDGOALS))' not found.";exit 1; fi;)
# 	@export LOAD_CONFIG_NAME=$(filter-out load_kconfig,$(MAKECMDGOALS))
# 	@echo cp $(CONFIGS_OUTPUT_PATH)/configs/$(LOAD_CONFIG_NAME).config $(CONFIGS_OUTPUT_PATH)/sdkconfig
# 	@echo "${GREEN}load $(LOAD_CONFIG_NAME) config is right${NC}"
# 	@exit 0


ifeq ($(MAKECMDGOALS),load_kconfig)
# 判断文件 $(CONFIGS_OUTPUT_PATH)/$(CONFIGS_PATH_NAME)/$(LOAD_CONFIG_NAME).config 是否存在
ifeq ($(wildcard $(CONFIGS_OUTPUT_PATH)/$(CONFIGS_PATH_NAME)/$(LOAD_CONFIG_NAME).config),)
$(error File "$(CONFIGS_OUTPUT_PATH)/$(CONFIGS_PATH_NAME)/$(LOAD_CONFIG_NAME).config does not exist!,please use make list_kconfig to check how to work)
endif
endif
 
load_kconfig:$(CONFIGS_OUTPUT_PATH)/configs/$(LOAD_CONFIG_NAME).config
	cp -rf $(CONFIGS_OUTPUT_PATH)/configs/$(LOAD_CONFIG_NAME).config $(PROJECT_DIR)/sdkconfig 
	@echo "${GREEN}load $(LOAD_CONFIG_NAME) config is right${NC}"
	$(SDK_KCONFIG_DIR)/genconfig.py

list_kconfig:
	@echo The following kconfig configuration files are supported
	@for file in $(CONFIGS_OUTPUT_PATH)/$(CONFIGS_PATH_NAME)/*; do \
		config=$$(basename $$file); \
		config=$$(basename $$config .config); \
		echo $$config; \
	done
	@echo "${RED}you can use '${GREEN}make load_kconfig LOAD_CONFIG_NAME=<kconfig configuration files>${RED}' to load the kconfig.${NC}"


