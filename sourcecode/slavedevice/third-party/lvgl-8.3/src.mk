
ifdef CONFIG_USE_LVGL

LVGL_C_DIR = $(SDK_DIR)/third-party/lvgl-8.3

ABSOLUTE_CFILES +=$(wildcard $(LVGL_C_DIR)/src/core/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/draw/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/extra/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/font/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/hal/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/misc/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/draw/arm2d/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/draw/nxp/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/draw/sdl/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/draw/stm32_dma2d/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/draw/sw/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/draw/swm342_dma2d/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/layouts/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/libs/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/others/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/themes/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/layouts/flex/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/layouts/grid/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/libs/bmp/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/libs/ffmpeg/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/libs/freetype/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/libs/fsdrv/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/libs/gif/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/libs/png/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/libs/qrcode/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/libs/rlottie/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/libs/sjgp/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/others/fragment/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/others/gridnav/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/others/ime/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/others/imgfont/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/others/monkey/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/others/msg/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/others/snapshot/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/themes/basic/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/themes/default/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/themes/mono/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/animimg/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/calendar/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/chart/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/colorwheel/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/imgbtn/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/btn/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/slider/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/btnmatrix/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/keyboard/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/led/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/table/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/bar/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/switch/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/dropdown/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/list/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/label/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/arc/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/line/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/roller/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/menu/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/meter/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/msgbox/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/span/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/img/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/spinbox/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/lv_line/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/spinner/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/tabview/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/textarea/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/checkbox/*.c) \
				$(wildcard $(LVGL_C_DIR)/src/widgets/tileview/*.c) \
				$(wildcard $(LVGL_C_DIR)/demos/benchmark/assets/*.c) \
				$(wildcard $(LVGL_C_DIR)/demos/benchmark/*.c)\
				$(wildcard $(LVGL_C_DIR)/src/widgets/win/*.c) \
				$(wildcard $(LVGL_C_DIR)/demos/widgets/assets/*.c)\
				$(wildcard $(LVGL_C_DIR)/demos/widgets/*.c)\
				$(wildcard $(LVGL_C_DIR)/demos/stress/*.c)


	    ifdef CONFIG_FREERTOS_USE_MEDIA

		CSRCS_RELATIVE_FILES += $(wildcard port/*.c)

        endif
endif #CONFIG_USE_SFUD


