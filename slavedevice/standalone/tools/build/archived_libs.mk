include $(PROJECT_DIR)/sdkconfig

ifdef CONFIG_ENABLE_FDC_DP_USE_LIB

ifdef CONFIG_TARGET_ARMV8_AARCH64
	BAREMETAL_LIBS += $(SDK_DIR)/drivers/media/fdcdp_lib/fdcdp_standalone_a64.a
else
	BAREMETAL_LIBS += $(SDK_DIR)/drivers/media/fdcdp_lib/fdcdp_standalone_a32.a
endif

else

ifdef CONFIG_TARGET_ARMV8_AARCH64
$(BUILD_OUT_PATH)/fdcdp_standalone_a64.a: fdcdp_standalone_a64.a
fdcdp_standalone_a64.a:
	$(call invoke_make_in_directory,drivers/media/fdcdp,makefile,all,)
fdcdp_standalone_a64_debug:
	$(call invoke_make_in_directory,drivers/media/fdcdp,makefile,debug,)
fdcdp_standalone_a64_info:
	$(call invoke_make_in_directory,drivers/media/fdcdp,makefile,compiler_info,)

else
$(BUILD_OUT_PATH)/fdcdp_standalone_a32.a: fdcdp_standalone_a32.a
fdcdp_standalone_a32.a:
	$(call invoke_make_in_directory,drivers/media/fdcdp,makefile,all,)
fdcdp_standalone_a32_debug:
	$(call invoke_make_in_directory,drivers/media/fdcdp,makefile,debug,)
fdcdp_standalone_a32_info:
	$(call invoke_make_in_directory,drivers/media/fdcdp,makefile,compiler_info,)

endif


endif
