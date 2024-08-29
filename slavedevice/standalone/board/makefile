include $(PROJECT_DIR)/sdkconfig

# Use E2000Q demo board
ifdef CONFIG_E2000Q_DEMO_BOARD
include e2000q_demo/src.mk
CSRCS_RELATIVE_FILES := $(BOARD_CSRC) 
endif

# Use E2000D demo board
ifdef CONFIG_E2000D_DEMO_BOARD
include e2000d_demo/src.mk
CSRCS_RELATIVE_FILES := $(BOARD_CSRC) 
endif

# Use firefly demo board
ifdef CONFIG_FIREFLY_DEMO_BOARD
include firefly/src.mk
CSRCS_RELATIVE_FILES := $(BOARD_CSRC) 
endif

# Use d2000 demo board
ifdef CONFIG_D2000_TEST_BOARD
include d2000_test/src.mk
CSRCS_RELATIVE_FILES := $(BOARD_CSRC) 
endif

# Use ft2004 demo board
ifdef CONFIG_FT2004_DSK_BOARD
include ft2004_dsk/src.mk
CSRCS_RELATIVE_FILES := $(BOARD_CSRC) 
endif

include $(SDK_DIR)/tools/build/compiler.mk
