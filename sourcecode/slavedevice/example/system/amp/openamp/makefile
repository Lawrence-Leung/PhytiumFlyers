# Compiling for specific instance

.PHONY: all clean boot menuconfig_core0 menuconfig_core1 

clean_core0:
	$(MAKE) -C ./core0 clean

clean_core1:
	$(MAKE) -C ./core1 clean

clean: clean_core0 clean_core1

config_d2000_aarch64:
	$(MAKE) -C ./core0 load_kconfig LOAD_CONFIG_NAME=d2000_aarch64_TEST_openamp_core0
	$(MAKE) -C ./core1 load_kconfig LOAD_CONFIG_NAME=d2000_aarch64_TEST_openamp_core1

config_d2000_aarch32:
	$(MAKE) -C ./core0 load_kconfig LOAD_CONFIG_NAME=d2000_aarch32_TEST_openamp_core0
	$(MAKE) -C ./core1 load_kconfig LOAD_CONFIG_NAME=d2000_aarch32_TEST_openamp_core1

config_ft2004_aarch64:
	$(MAKE) -C ./core0 load_kconfig LOAD_CONFIG_NAME=ft2004_aarch64_DSK_openamp_core0
	$(MAKE) -C ./core1 load_kconfig LOAD_CONFIG_NAME=ft2004_aarch64_DSK_openamp_core1

config_ft2004_aarch32:
	$(MAKE) -C ./core0 load_kconfig LOAD_CONFIG_NAME=ft2004_aarch32_DSK_openamp_core0
	$(MAKE) -C ./core1 load_kconfig LOAD_CONFIG_NAME=ft2004_aarch32_DSK_openamp_core1

config_e2000d_aarch64:
	$(MAKE) -C ./core0 load_kconfig LOAD_CONFIG_NAME=e2000d_aarch64_demo_openamp_core0
	$(MAKE) -C ./core1 load_kconfig LOAD_CONFIG_NAME=e2000d_aarch64_demo_openamp_core1

config_e2000d_aarch32:
	$(MAKE) -C ./core0 load_kconfig LOAD_CONFIG_NAME=e2000d_aarch32_demo_openamp_core0
	$(MAKE) -C ./core1 load_kconfig LOAD_CONFIG_NAME=e2000d_aarch32_demo_openamp_core1

config_e2000q_aarch64:
	$(MAKE) -C ./core0 load_kconfig LOAD_CONFIG_NAME=e2000q_aarch64_demo_openamp_core0
	$(MAKE) -C ./core1 load_kconfig LOAD_CONFIG_NAME=e2000q_aarch64_demo_openamp_core1

config_e2000q_aarch32:
	$(MAKE) -C ./core0 load_kconfig LOAD_CONFIG_NAME=e2000q_aarch32_demo_openamp_core0
	$(MAKE) -C ./core1 load_kconfig LOAD_CONFIG_NAME=e2000q_aarch32_demo_openamp_core1

config_phytiumpi_aarch64:
	$(MAKE) -C ./core0 load_kconfig LOAD_CONFIG_NAME=phytiumpi_aarch64_firefly_openamp_core0
	$(MAKE) -C ./core1 load_kconfig LOAD_CONFIG_NAME=phytiumpi_aarch64_firefly_openamp_core1

config_phytiumpi_aarch32:
	$(MAKE) -C ./core0 load_kconfig LOAD_CONFIG_NAME=phytiumpi_aarch32_firefly_openamp_core0
	$(MAKE) -C ./core1 load_kconfig LOAD_CONFIG_NAME=phytiumpi_aarch32_firefly_openamp_core1

menuconfig_core0:
	$(MAKE) -C ./core0 menuconfig

menuconfig_core1:
	$(MAKE) -C ./core1 menuconfig

image:
	$(MAKE) -C ./core0 image
	$(MAKE) -C ./core1 image

backupconfig:
	$(MAKE) -C ./core0 backup_kconfig
	$(MAKE) -C ./core1 backup_kconfig

