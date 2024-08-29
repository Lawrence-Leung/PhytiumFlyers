ifdef CONFIG_USE_OPENAMP

BUILD_INC_PATH_DIR +=  $(SDK_DIR)/third-party/openamp/lib \
			$(SDK_DIR)/third-party/openamp/lib/include \
			$(SDK_DIR)/third-party/openamp/lib/include/openamp \
			$(SDK_DIR)/third-party/openamp/lib/rpmsg \
			$(SDK_DIR)/third-party/openamp/ports \
			$(SDK_DIR)/third-party/openamp/lib/remoteproc \
			$(SDK_DIR)/third-party/openamp/lib/rpmsg \
			$(SDK_DIR)/third-party/openamp/lib/service/rpmsg/rpc \
			$(SDK_DIR)/third-party/openamp/lib/virtio \
			$(SDK_DIR)/third-party/openamp/ports
			 

endif #CONFIG_USE_OPENAMP