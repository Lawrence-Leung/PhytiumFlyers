# 为用户准备的提供的对象进行编译
.PHONY: all clean link_info

EXTRA_OBJS ?=

-include $(PROJECT_DIR)/sdkconfig
ifneq ($(CONFIG_ARCH_NAME)/$(CONFIG_ARCH_EXECUTION_STATE),/)
include $(SDK_DIR)/arch/$(subst ",,$(CONFIG_ARCH_NAME)/$(CONFIG_ARCH_EXECUTION_STATE))/arch_compiler.mk
endif

IMAGE_OUT_NAME ?=  $(CONFIG_SOC_NAME)$(CONFIG_TARGET_TYPE_NAME)_$(CONFIG_ARCH_EXECUTION_STATE)_$(CONFIG_BOARD_NAME)_$(CONFIG_TARGET_NAME)

# 明确需要在哪些路径进行编译
include $(SDK_DIR)/tools/build/sdk_components.mk

$(IMAGE_NAME).elf : $(IMAGE_OUTPUT)/$(IMAGE_OUT_NAME).elf 

$(IMAGE_OUTPUT)/$(IMAGE_OUT_NAME).elf: $(BAREMETAL_LIBS) $(LIBS_OBJS) $(EXTRA_OBJS) 
ifndef IDE_EXPORT
	@echo "  LIBS   $(EXTRA_OBJS)"
	@echo "  LINK   $(IMAGE_OUT_NAME).elf"
	@echo "  DUMP   $(IMAGE_OUT_NAME).map"
ifeq ($(CONFIG_ENABLE_CXX),y)
	@$(CXX) $(ARCH_CPU_MARCH) $(ARCH_CPU_FPU)  $(ARCH_DEBUG) $(ARCH_OPTIMIZATION) $(EXTRA_OBJS) -T $(LIBS_OBJS) $(LDFLAGS)  \
		-Wl,-Map,"$(IMAGE_OUTPUT)/$(IMAGE_OUT_NAME).map" -Wl,--start-group $(LIBGCC) $(LIBC) $(LIBM) $(LIBCXX) -Wl,--whole-archive $(BAREMETAL_LIBS) \
		-Wl,--no-whole-archive $(EXTRALIBS) -Wl,--end-group  -o  $@
else
	@$(CC) $(ARCH_CPU_MARCH) $(ARCH_CPU_FPU)  $(ARCH_DEBUG) $(ARCH_OPTIMIZATION) $(EXTRA_OBJS) -T $(LIBS_OBJS) $(LDFLAGS)  \
		-Wl,-Map,"$(IMAGE_OUTPUT)/$(IMAGE_OUT_NAME).map" -Wl,--start-group $(LIBGCC) $(LIBC) $(LIBM) -Wl,--whole-archive $(BAREMETAL_LIBS) \
		-Wl,--no-whole-archive $(EXTRALIBS) -Wl,--end-group  -o  $@
endif

ifdef CONFIG_OUTPUT_BINARY
	@echo "  COPY   $(IMAGE_OUT_NAME).bin"
	@$(OBJCOPY) -O binary   $(IMAGE_OUTPUT)/$(IMAGE_OUT_NAME).elf $(IMAGE_OUTPUT)/$(IMAGE_OUT_NAME).bin
endif

ifdef CONFIG_OUTPUT_ASM_DIS
	@echo "  -D     $(IMAGE_OUT_NAME).asm"
	@$(OD) -D $(IMAGE_OUTPUT)/$(IMAGE_OUT_NAME).elf > $(IMAGE_OUTPUT)/$(IMAGE_OUT_NAME).asm
	@echo "  -S     $(IMAGE_OUT_NAME).dis"
	@$(OD) -S $(IMAGE_OUTPUT)/$(IMAGE_OUT_NAME).elf > $(IMAGE_OUTPUT)/$(IMAGE_OUT_NAME).dis
endif
	@echo "  SOC    $(CONFIG_SOC_NAME)$(CONFIG_TARGET_TYPE_NAME)"
	@echo "  Execution state is $(CONFIG_ARCH_EXECUTION_STATE)"

else
	@echo "  Export SDK Success !!!"
endif #IDE_EXPORT

export_ide: $(BAREMETAL_IDE_JSON)
# 打包所有源码的配置
	@$(PYTHON) $(SDK_PYTHON_TOOLS_DIR)/ide_export_json_pack.py "$(BUILD_OUT_PATH)/" "compile_commands"
# 插入自定义库的配置
	@$(PYTHON) $(SDK_PYTHON_TOOLS_DIR)/ide_export_json_libs.py "$(SOURCE_DEFINED_LIBS) $(EXTRALIBS)"  "$(BUILD_OUT_PATH)/" "compile_commands.json"
	
link_info:
	@echo LDFLAGS: $(LDFLAGS)
	@echo LIBGCC: $(LIBGCC)
	@echo LIBC: $(LIBC)
	@echo LIBM: $(LIBM)
	@echo BAREMETAL_LIBS: $(BAREMETAL_LIBS)
	@echo CFLAGS : $(CFLAGS) 
	
clean:
# 清除所有编译资源
	@rm -rf $(BUILD_OUT_PATH)
	@rm -rf $(IMAGE_OUTPUT)/*.elf
	@rm -rf $(IMAGE_OUTPUT)/*.bin
	@rm -rf $(IMAGE_OUTPUT)/*.map
	@rm -rf $(IMAGE_OUTPUT)/*.dis
	@rm -rf $(IMAGE_OUTPUT)/*.asm