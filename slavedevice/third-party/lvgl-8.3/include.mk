

ifdef CONFIG_USE_LVGL

	BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party \
		  $(SDK_DIR)/third-party/lvgl-8.3\
                  $(SDK_DIR)/third-party/lvgl-8.3/src \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/core \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/draw \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/extra \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/font \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/hal \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/misc \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/draw/arm2d \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/draw/nxp \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/draw/sdl \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/draw/stm32_dma2d \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/draw/sw \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/draw/swm342_dma2d \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/layouts \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/libs \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/others \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/themes \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/layouts/flex \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/layouts/grid \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/libs/bmp \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/libs/ffmpeg \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/libs/freetype \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/libs/fsdrv \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/libs/gif \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/libs/png \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/libs/qrcode \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/libs/rlottie \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/libs/sjgp \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/others/fragment \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/others/gridnav \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/others/ime \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/others/imgfont \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/others/monkey \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/others/msg \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/others/snapshot \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/themes/basic \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/themes/default \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/themes/mono \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/animimg \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/calendar \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/chart \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/colorwheel \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/imgbtn \
		  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/btn \
		  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/dropdown \
		  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/slider \
		  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/btnmatrix \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/keyboard \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/led \
		  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/bar \
		  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/label \
		  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/roller \
		  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/checkbox \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/list \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/menu \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/meter \
		  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/lv_line\
		  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/textarea \
		  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/arc \
		  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/img \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/msgbox \
		  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/switch \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/span \
		  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/table \
		  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/line \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/spinbox \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/spinner \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/tabview \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/tileview \
		  $(SDK_DIR)/third-party/lvgl-8.3/demos/benchmark/assets \
		  $(SDK_DIR)/third-party/lvgl-8.3/demos/benchmark\
		  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets/win \
		  $(SDK_DIR)/third-party/lvgl-8.3/demos/widgets/assets\
		  $(SDK_DIR)/third-party/lvgl-8.3/demos/widgets\
		  $(SDK_DIR)/third-party/lvgl-8.3/demos/stress

	ifdef CONFIG_FREERTOS_USE_MEDIA

        BUILD_INC_PATH_DIR += $(FREERTOS_SDK_DIR)/third-party/lvgl-8.3/port

        endif

endif


