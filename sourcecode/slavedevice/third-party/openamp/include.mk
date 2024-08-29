ifdef CONFIG_USE_OPENAMP

THIRDP_CUR_DIR := $(FREERTOS_SDK_DIR)/third-party

# src files
BUILD_INC_PATH_DIR += $(THIRDP_CUR_DIR)/openamp/lib \
			$(THIRDP_CUR_DIR)/openamp/lib/include \
			$(THIRDP_CUR_DIR)/openamp/lib/include/openamp \
			$(THIRDP_CUR_DIR)/openamp/lib/rpmsg \
			$(THIRDP_CUR_DIR)/openamp/ports \
			$(THIRDP_CUR_DIR)/openamp/lib/remoteproc \
			$(THIRDP_CUR_DIR)/openamp/lib/rpmsg \
			$(THIRDP_CUR_DIR)/openamp/lib/service/rpmsg/rpc \
			$(THIRDP_CUR_DIR)/openamp/lib/virtio \
			$(THIRDP_CUR_DIR)/openamp/ports 

endif
