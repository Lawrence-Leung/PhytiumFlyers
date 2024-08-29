
INCLUDE_SOC_NAME :=  $(subst ",,$(CONFIG_SOC_NAME))
BUILD_INC_PATH_DIR += $(SDK_DIR)/soc/$(INCLUDE_SOC_NAME) \
				 $(SDK_DIR)/soc/common

include $(SDK_DIR)/soc/$(INCLUDE_SOC_NAME)/cpu_inc.mk
