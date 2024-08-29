
	CSRCS_RELATIVE_FILES += $(wildcard src/*.c) \
				$(wildcard src/core/*.c) \
				$(wildcard src/draw/*.c) \
				$(wildcard src/extra/*.c) \
				$(wildcard src/font/*.c) \
				$(wildcard src/hal/*.c) \
				$(wildcard src/misc/*.c) \
				$(wildcard src/widgets/*.c) \
				$(wildcard src/draw/arm2d/*.c) \
				$(wildcard src/draw/nxp/*.c) \
				$(wildcard src/draw/sdl/*.c) \
				$(wildcard src/draw/stm32_dma2d/*.c) \
				$(wildcard src/draw/sw/*.c) \
				$(wildcard src/draw/swm342_dma2d/*.c) \
				$(wildcard src/layouts/*.c) \
				$(wildcard src/libs/*.c) \
				$(wildcard src/others/*.c) \
				$(wildcard src/themes/*.c) \
				$(wildcard src/layouts/flex/*.c) \
				$(wildcard src/layouts/grid/*.c) \
				$(wildcard src/libs/bmp/*.c) \
				$(wildcard src/libs/ffmpeg/*.c) \
				$(wildcard src/libs/freetype/*.c) \
				$(wildcard src/libs/fsdrv/*.c) \
				$(wildcard src/libs/gif/*.c) \
				$(wildcard src/libs/png/*.c) \
				$(wildcard src/libs/qrcode/*.c) \
				$(wildcard src/libs/rlottie/*.c) \
				$(wildcard src/libs/sjgp/*.c) \
				$(wildcard src/others/fragment/*.c) \
				$(wildcard src/others/gridnav/*.c) \
				$(wildcard src/others/ime/*.c) \
				$(wildcard src/others/imgfont/*.c) \
				$(wildcard src/others/monkey/*.c) \
				$(wildcard src/others/msg/*.c) \
				$(wildcard src/others/snapshot/*.c) \
				$(wildcard src/themes/basic/*.c) \
				$(wildcard src/themes/default/*.c) \
				$(wildcard src/themes/mono/*.c) \
				$(wildcard src/widgets/animimg/*.c) \
				$(wildcard src/widgets/calendar/*.c) \
				$(wildcard src/widgets/chart/*.c) \
				$(wildcard src/widgets/colorwheel/*.c) \
				$(wildcard src/widgets/imgbtn/*.c) \
				$(wildcard src/widgets/btn/*.c) \
				$(wildcard src/widgets/slider/*.c) \
				$(wildcard src/widgets/btnmatrix/*.c) \
				$(wildcard src/widgets/keyboard/*.c) \
				$(wildcard src/widgets/led/*.c) \
				$(wildcard src/widgets/table/*.c) \
				$(wildcard src/widgets/bar/*.c) \
				$(wildcard src/widgets/switch/*.c) \
				$(wildcard src/widgets/dropdown/*.c) \
				$(wildcard src/widgets/list/*.c) \
				$(wildcard src/widgets/label/*.c) \
				$(wildcard src/widgets/arc/*.c) \
				$(wildcard src/widgets/line/*.c) \
				$(wildcard src/widgets/roller/*.c) \
				$(wildcard src/widgets/menu/*.c) \
				$(wildcard src/widgets/meter/*.c) \
				$(wildcard src/widgets/msgbox/*.c) \
				$(wildcard src/widgets/span/*.c) \
				$(wildcard src/widgets/img/*.c) \
				$(wildcard src/widgets/spinbox/*.c) \
				$(wildcard src/widgets/lv_line/*.c) \
				$(wildcard src/widgets/spinner/*.c) \
				$(wildcard src/widgets/tabview/*.c) \
				$(wildcard src/widgets/textarea/*.c) \
				$(wildcard src/widgets/checkbox/*.c) \
				$(wildcard src/widgets/tileview/*.c) \
				$(wildcard demos/benchmark/assets/*.c) \
				$(wildcard demos/benchmark/*.c)\
				$(wildcard src/widgets/win/*.c) \
				$(wildcard demos/widgets/assets/*.c)\
				$(wildcard demos/widgets/*.c)\
				$(wildcard demos/stress/*.c)
				  

ifdef CONFIG_USE_BAREMETAL

CSRCS_RELATIVE_FILES += $(wildcard port/*.c) \
				$(wildcard port/lv_port_demo/*.c)

endif #CONFIG_USE_BAREMETAL

