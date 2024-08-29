
ifdef CONFIG_USE_LETTER_SHELL
$(BUILD_OUT_PATH)/lib_letter_shell.a: lib_letter_shell.a
lib_letter_shell.a: 
	$(call invoke_make_in_directory,third-party/letter-shell-3.1,makefile,all,)
lib_letter_shell_debug:
	$(call invoke_make_in_directory,third-party/letter-shell-3.1,makefile,debug,)
lib_letter_shell_info:
	$(call invoke_make_in_directory,third-party/letter-shell-3.1,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_letter_shell.a
endif

$(BUILD_OUT_PATH)/lib_backtrace.a: lib_backtrace.a
lib_backtrace.a: 
	$(call invoke_make_in_directory,third-party/backtrace,makefile,all,)
lib_backtrace_debug:
	$(call invoke_make_in_directory,third-party/backtrace,makefile,debug,)
lib_backtrace_info:
	$(call invoke_make_in_directory,third-party/backtrace,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_backtrace.a

ifdef CONFIG_USE_LWIP
$(BUILD_OUT_PATH)/liblwip.a: liblwip.a
liblwip.a: 
	$(call invoke_make_in_directory,third-party/lwip-2.1.2,makefile,all,)
liblwip_debug:
	$(call invoke_make_in_directory,third-party/lwip-2.1.2,makefile,debug,)
liblwip_info:
	$(call invoke_make_in_directory,third-party/lwip-2.1.2,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/liblwip.a
endif

ifdef CONFIG_USE_SFUD
$(BUILD_OUT_PATH)/libsfud.a: libsfud.a
libsfud.a: 
	$(call invoke_make_in_directory,third-party/sfud-1.1.0,makefile,all,)
libsfud_debug:
	$(call invoke_make_in_directory,third-party/sfud-1.1.0,makefile,debug,)
libsfud_info:
	$(call invoke_make_in_directory,third-party/sfud-1.1.0,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/libsfud.a
endif #CONFIG_USE_SFUD

ifdef CONFIG_USE_SDMMC_CMD
$(BUILD_OUT_PATH)/libsdmmc.a: libsdmmc.a
libsdmmc.a: 
	$(call invoke_make_in_directory,third-party/sdmmc,makefile,all,)
libsdmmc_debug:
	$(call invoke_make_in_directory,third-party/sdmmc,makefile,debug,)
libsdmmc_info:
	$(call invoke_make_in_directory,third-party/sdmmc,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/libsdmmc.a
endif

ifdef CONFIG_USE_TLSF
$(BUILD_OUT_PATH)/libtlsf.a: libtlsf.a
libtlsf.a:
	$(call invoke_make_in_directory,third-party/tlsf-3.1.0,makefile,all,)
libtlsf_debug:
	$(call invoke_make_in_directory,third-party/tlsf-3.1.0,makefile,debug,)
libtlsf_info:
	$(call invoke_make_in_directory,third-party/tlsf-3.1.0,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/libtlsf.a
endif

ifdef CONFIG_USE_SPIFFS
$(BUILD_OUT_PATH)/libspiffs.a: libspiffs.a
libspiffs.a:
	$(call invoke_make_in_directory,third-party/spiffs-0.3.7,makefile,all,)
spiffs_debug:
	$(call invoke_make_in_directory,third-party/spiffs-0.3.7,makefile,debug,)
spiffs_iinfo:
	$(call invoke_make_in_directory,third-party/spiffs-0.3.7,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/libspiffs.a
endif

# freemodbus-v1.6
ifdef CONFIG_USE_FREEMODBUS
$(BUILD_OUT_PATH)/libfreemodbus.a: libfreemodbus.a
libfreemodbus.a:
	$(call invoke_make_in_directory,third-party/freemodbus-v1.6,makefile,all,)
freemodbus_debug:
	$(call invoke_make_in_directory,third-party/freemodbus-v1.6,makefile,debug,)
freemodbus_info:
	$(call invoke_make_in_directory,third-party/freemodbus-v1.6,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/libfreemodbus.a
endif
# libmetal
ifdef CONFIG_USE_LIBMETAL
$(BUILD_OUT_PATH)/lib_libmetal.a: lib_libmetal.a
lib_libmetal.a:
	$(call invoke_make_in_directory,third-party/libmetal,makefile,all,)
lib_libmetal_debug:
	$(call invoke_make_in_directory,third-party/libmetal,makefile,debug,)
lib_libmetal_info:
	$(call invoke_make_in_directory,third-party/libmetal,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_libmetal.a
endif

# openamp
ifdef CONFIG_USE_OPENAMP
$(BUILD_OUT_PATH)/lib_openamp.a: lib_openamp.a
lib_openamp.a:
	$(call invoke_make_in_directory,third-party/openamp,makefile,all,)
lib_openamp_debug:
	$(call invoke_make_in_directory,third-party/openamp,makefile,debug,)
lib_openamp_info:
	$(call invoke_make_in_directory,third-party/openamp,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_openamp.a
endif


# Crypto++
ifdef CONFIG_USE_CRYPTO_PLUS_PLUS
$(BUILD_OUT_PATH)/lib_crypto_pp.a: lib_crypto_pp.a
lib_crypto_pp.a:
	$(call invoke_make_in_directory,third-party/crypto++,makefile,all,)
lib_crypto_pp_debug:
	$(call invoke_make_in_directory,third-party/crypto++,makefile,debug,)
lib_crypto_pp_info:
	$(call invoke_make_in_directory,third-party/crypto++,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_crypto_pp.a
endif


ifdef CONFIG_USE_LVGL
$(BUILD_OUT_PATH)/lib_lv.a: lib_lv.a
lib_lv.a:
	$(call invoke_make_in_directory,third-party/lvgl-8.3,makefile,all,)
lib_lv_debug:
	$(call invoke_make_in_directory,third-party/lvgl-8.3,makefile,debug,)
lib_lv_info:
	$(call invoke_make_in_directory,third-party/lvgl-8.3,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_lv.a
endif

# FatFs_0_1_4
ifdef CONFIG_USE_FATFS_0_1_4
$(BUILD_OUT_PATH)/lib_fatfs.a: lib_fatfs.a
lib_fatfs.a:
	$(call invoke_make_in_directory,third-party/fatfs-0.1.4,makefile,all,)
lib_fatfs_debug:
	$(call invoke_make_in_directory,third-party/fatfs-0.1.4,makefile,debug,)
lib_fatfs_info:
	$(call invoke_make_in_directory,third-party/fatfs-0.1.4,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_fatfs.a
endif