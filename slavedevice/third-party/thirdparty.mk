include $(PROJECT_DIR)/sdkconfig

FREERTOS_SDK_DIR ?= $(SDK_DIR)/..

ifdef CONFIG_USE_FREERTOS
$(BUILD_OUT_PATH)/lib_freertos.a: lib_freertos.a
lib_freertos.a: 
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/freertos,makefile,all,)
lib_freertos_debug:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/freertos,makefile,debug,)
lib_freertos_info:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/freertos,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_freertos.a
endif

ifdef CONFIG_USE_LETTER_SHELL
$(BUILD_OUT_PATH)/lib_letter_shell.a: lib_letter_shell.a
lib_letter_shell.a:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/letter-shell-3.1,makefile,all,)
lib_letter_shell_debug:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/letter-shell-3.1,makefile,debug,)
lib_letter_shell_info:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/letter-shell-3.1,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_letter_shell.a
endif

ifdef CONFIG_USE_BACKTRACE
$(BUILD_OUT_PATH)/lib_backtrace.a: lib_backtrace.a
lib_backtrace.a: 
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/backtrace,makefile,all,)
lib_backtrace_debug:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/backtrace,makefile,debug,)
lib_backtrace_info:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/backtrace,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_backtrace.a
endif

ifdef CONFIG_USE_FATFS_0_1_4
$(BUILD_OUT_PATH)/lib_fatfs.a: lib_fatfs.a
lib_fatfs.a: 
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/fatfs-0.1.4,makefile,all,)
lib_fatfs_debug:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/fatfs-0.1.4,makefile,debug,)
lib_fatfs_info:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/fatfs-0.1.4,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_fatfs.a
endif

ifdef CONFIG_USE_TLSF
$(BUILD_OUT_PATH)/lib_tlsf.a: lib_tlsf.a
lib_tlsf.a:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/tlsf-3.1.0,makefile,all,)
lib_tlsf_debug:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/tlsf-3.1.0,makefile,debug,)
lib_tlsf_info:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/tlsf-3.1.0,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_tlsf.a
endif

ifdef CONFIG_USE_LWIP
$(BUILD_OUT_PATH)/lib_lwip.a: lib_lwip.a
lib_lwip.a: 
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/lwip-2.1.2,makefile,all,)
lib_lwip_debug:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/lwip-2.1.2,makefile,debug,)
lib_lwip_info:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/lwip-2.1.2,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_lwip.a
endif

ifdef CONFIG_USE_SFUD
$(BUILD_OUT_PATH)/lib_sfud.a: lib_sfud.a
lib_sfud.a:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/sfud-1.1.0,makefile,all,)
lib_sfud_debug:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/sfud-1.1.0,makefile,debug,)
lib_sfud_info:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/sfud-1.1.0,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_sfud.a
endif

ifdef CONFIG_USE_SPIFFS
$(BUILD_OUT_PATH)/lib_spiffs.a: lib_spiffs.a
lib_spiffs.a:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/spiffs-0.3.7,makefile,all,)
lib_spiffs_debug:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/spiffs-0.3.7,makefile,debug,)
lib_spiffs_info:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/spiffs-0.3.7,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_spiffs.a
endif

ifdef CONFIG_USE_LVGL
$(BUILD_OUT_PATH)/lib_lvgl_8.3.a:lib_lvgl_8.3.a
lib_lvgl_8.3.a:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/lvgl-8.3,makefile,all,)
lib_lvgl_debug:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/lvgl-8.3,makefile,debug,)
lib_lvgl_info:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/lvgl-8.3,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_lvgl_8.3.a
endif

# openamp
ifdef CONFIG_USE_OPENAMP
$(BUILD_OUT_PATH)/lib_openamp.a: lib_openamp.a
lib_openamp.a:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/openamp,makefile,all,)
lib_openamp_debug:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/openamp,makefile,debug,)
lib_openamp_info:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/openamp,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_openamp.a
endif

ifdef CONFIG_USE_CHERRY_USB
$(BUILD_OUT_PATH)/lib_cherryusb.a: lib_cherryusb.a
lib_cherryusb.a:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/cherryusb,makefile,all,)
lib_cherryusb_debug:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/cherryusb,makefile,debug,)
lib_cherryusb_info:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/cherryusb,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_cherryusb.a
endif

ifdef CONFIG_USE_SDMMC_CMD
$(BUILD_OUT_PATH)/lib_sdmmc.a: lib_sdmmc.a
lib_sdmmc.a:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/sdmmc-1.0,makefile,all,)
lib_sdmmc_debug:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/sdmmc-1.0,makefile,debug,)
lib_sdmmc_info:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/sdmmc-1.0,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_sdmmc.a
endif

# libmetal
ifdef CONFIG_USE_LIBMETAL
$(BUILD_OUT_PATH)/lib_libmetal.a: lib_libmetal.a
lib_libmetal.a:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/libmetal,makefile,all,)
lib_libmetal_debug:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/libmetal,makefile,debug,)
lib_libmetal_info:
	$(call rtos_invoke_make_in_dir,$(FREERTOS_SDK_DIR),third-party/libmetal,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_libmetal.a
endif

# ifdef CONFIG_USE_SDMMC_CMD
# $(BUILD_OUT_PATH)/libsdmmc.a: libsdmmc.a
# libsdmmc.a: 
# 	$(call invoke_make_in_directory,third-party/sdmmc,makefile,all,)
# libsdmmc_debug:
# 	$(call invoke_make_in_directory,third-party/sdmmc,makefile,debug,)
# libsdmmc_info:
# 	$(call invoke_make_in_directory,third-party/sdmmc,makefile,compiler_info,)
# BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/libsdmmc.a
# endif


# # freemodbus-v1.6
# ifdef CONFIG_USE_FREEMODBUS
# $(BUILD_OUT_PATH)/libfreemodbus.a: libfreemodbus.a
# libfreemodbus.a:
# 	$(call invoke_make_in_directory,third-party/freemodbus-v1.6,makefile,all,)
# freemodbus_debug:
# 	$(call invoke_make_in_directory,third-party/freemodbus-v1.6,makefile,debug,)
# freemodbus_info:
# 	$(call invoke_make_in_directory,third-party/freemodbus-v1.6,makefile,compiler_info,)
# BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/libfreemodbus.a
# endif

# # Crypto++
# ifdef CONFIG_USE_CRYPTO_PLUS_PLUS
# $(BUILD_OUT_PATH)/lib_crypto_pp.a: lib_crypto_pp.a
# lib_crypto_pp.a:
# 	$(call invoke_make_in_directory,third-party/crypto++,makefile,all,)
# lib_crypto_pp_debug:
# 	$(call invoke_make_in_directory,third-party/crypto++,makefile,debug,)
# lib_crypto_pp_info:
# 	$(call invoke_make_in_directory,third-party/crypto++,makefile,compiler_info,)
# BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_crypto_pp.a
# endif