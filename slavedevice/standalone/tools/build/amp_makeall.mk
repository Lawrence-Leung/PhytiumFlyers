# 定义环境变量用于承接sdk的路径
SDK_DIR ?= $(CURDIR)/../..
TOOLS_BUILD_DIR ?= $(SDK_DIR)/tools/build
PROJECT_DIR ?= $(CURDIR)
# amp 组合相关变量
AMP_PATH ?= 
AMP_PACK_NAME ?= packed.bin

# 编译中间文件输出的路径
BUILD_OUT_PATH ?= $(PROJECT_DIR)/build


Q ?= @

export Q

.PHONY: all clean

export SDK_DIR
ifneq ($(wildcard $(SDK_DIR)/tools/build_baremetal.mk),)
$(error SDK_DIR/tools/build_baremetal.mk dose not exist)
endif

# 检查AMP_PATH是否存在，如果存在则显示路径
ifneq ($(wildcard $(AMP_PATH)),)
    $(info AMP_PATH exists: $(AMP_PATH))
else
    $(error AMP_PATH does not exist: $(AMP_PATH))
endif

AMP_IMG_EXPORT_IMG ?= $(PROJECT_DIR)/$(AMP_PACK_NAME)


# build boot code
all: amp_pack
	@$(MAKE) -C $(SDK_DIR)/tools/build/boot_code all AMP_PROJECT_PATH="$(PROJECT_DIR)" BUILD_OUT_PATH="$(BUILD_OUT_PATH)"  AMP_IMG_EXPORT_IMG="$(AMP_IMG_EXPORT_IMG)"

# 增加boot code 编译功能

menuconfig:
	$(MAKE) -C $(SDK_DIR)/tools/build/boot_code menuconfig

memory_check:
	@python3 $(TOOLS_BUILD_DIR)/py_tools/amp_memory_checker.py $(SDK_DIR)/tools/build/boot_code $(AMP_PATH)

amp_compiler: memory_check
	@for path in $(AMP_PATH); do \
        echo "Compiling $$path..."; \
		make -C $$path all || exit $$?; \
        elf_file=$$(find $$path -name "*.elf"); \
		echo $$elf_file; \
		if [ -z "$$elf_file" ]; then \
			echo "Error: no elf file found in $$path"; \
			exit 1; \
		fi; \
	done;

AMP_ELF_FILES := $(foreach dir, $(AMP_PATH), $(wildcard $(dir)/*.elf))

amp_pack: amp_compiler $(AMP_ELF_FILES)
	@echo AMP_ELF_FILES: $(AMP_ELF_FILES)
	@echo AMP_PATH : $(AMP_PATH)
	python3 $(TOOLS_BUILD_DIR)/py_tools/amp_elf_pack.py $(AMP_PATH)


clean:
	$(MAKE) -C $(SDK_DIR)/tools/build/boot_code AMP_PROJECT_PATH="$(PROJECT_DIR)" BUILD_OUT_PATH="$(BUILD_OUT_PATH)" AMP_IMG_EXPORT_IMG="$(AMP_IMG_EXPORT_IMG)" clean
	@for path in $(AMP_PATH); do \
        echo "clean $$path..."; \
		make -C $$path clean; \
	done;

backup_kconfig:
	@$(MAKE) -C $(SDK_DIR)/tools/build/boot_code backup_kconfig AMP_PROJECT_PATH="$(PROJECT_DIR)" BUILD_OUT_PATH="$(BUILD_OUT_PATH)"  AMP_IMG_EXPORT_IMG="$(AMP_IMG_EXPORT_IMG)"


boot_memory_check:
	@$(MAKE) -C $(SDK_DIR)/tools/build/boot_code boot_memory_check AMP_PROJECT_PATH="$(PROJECT_DIR)" BUILD_OUT_PATH="$(BUILD_OUT_PATH)"  AMP_IMG_EXPORT_IMG="$(AMP_IMG_EXPORT_IMG)"

list_kconfig:
	@$(MAKE) -C $(SDK_DIR)/tools/build/boot_code list_kconfig AMP_PROJECT_PATH="$(PROJECT_DIR)" BUILD_OUT_PATH="$(BUILD_OUT_PATH)"  AMP_IMG_EXPORT_IMG="$(AMP_IMG_EXPORT_IMG)"

load_kconfig:
	@$(MAKE) -C $(SDK_DIR)/tools/build/boot_code load_kconfig AMP_PROJECT_PATH="$(PROJECT_DIR)" BUILD_OUT_PATH="$(BUILD_OUT_PATH)"  AMP_IMG_EXPORT_IMG="$(AMP_IMG_EXPORT_IMG)"

update_menuconfig:
	@$(MAKE) -C $(SDK_DIR)/tools/build/boot_code update_menuconfig AMP_PROJECT_PATH="$(PROJECT_DIR)" BUILD_OUT_PATH="$(BUILD_OUT_PATH)"  AMP_IMG_EXPORT_IMG="$(AMP_IMG_EXPORT_IMG)"

