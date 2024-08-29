ifdef CONFIG_USE_OPENAMP

CSRCS_RELATIVE_FILES += $(wildcard lib/*.c \
					lib/remoteproc/*.c \
					lib/rpmsg/*.c \
					lib/service/rpmsg/rpc/*.c \
					lib/virtio/*.c \
					ports/*.c )

endif
