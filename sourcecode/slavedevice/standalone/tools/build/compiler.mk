


# Include necessary makefiles for building bare-metal projects
include $(SDK_DIR)/tools/build/baremetal_include.mk
# for other sdk depends on standalone sdk
-include $(EXTERNAL_PATH)/include.mk
# compiler 
include $(SDK_DIR)/arch/$(subst ",,$(CONFIG_ARCH_NAME)/$(CONFIG_ARCH_EXECUTION_STATE))/arch_compiler.mk

# Set the current directory to the current working directory, or the directory containing the Makefile
CURRENT_DIR ?=  $(shell pwd)/
# Set the build subdirectory path to the current directory
BUILD_SUBDIRECTORY_PATH ?=  $(shell basename $(CURRENT_DIR))
# Define the build path as the build output path with the build subdirectory path appended to it
BUILD_PATH := $(BUILD_OUT_PATH)/$(BUILD_SUBDIRECTORY_PATH)

# Define the library name as the output path and library name
LIBS ?= $(BUILD_OUT_PATH)/$(LIBS_NAME) 

# need check deps
ifdef CONFIG_CHECK_DEPS
all: $(LIBS) depend
$(OBJS): depend
else
all: $(LIBS)
endif

# include path
INCLUDE_PATH  += $(patsubst %, -I %, $(BUILD_INC_PATH_DIR))

# relative files
CSRCS_RELATIVE_FILES := $(addprefix $(CURRENT_DIR),$(CSRCS_RELATIVE_FILES))
ASRCS_RELATIVE_FILES := $(addprefix $(CURRENT_DIR),$(ASRCS_RELATIVE_FILES))
ifeq ($(CONFIG_ENABLE_CXX),y)
CXXSRCS_RELATIVE_FILES := $(addprefix $(CURRENT_DIR),$(CXXSRCS_RELATIVE_FILES))
endif

# absolute files
ABSOLUTE_CFILES += $(filter %.c, $(CSRCS_RELATIVE_FILES))
ABSOLUTE_AFILES += $(filter %.S, $(ASRCS_RELATIVE_FILES))
ifeq ($(CONFIG_ENABLE_CXX),y)
ABSOLUTE_CXXFILES += $(filter %.cpp, $(CXXSRCS_RELATIVE_FILES))
ABSOLUTE_CXXFILES += $(filter %.cc, $(CXXSRCS_RELATIVE_FILES))
endif

# source files targets


OBJS := $(foreach file,$(filter %.c,$(ABSOLUTE_CFILES)),$(BUILD_PATH)/$(notdir $(shell dirname $(file)))/$(notdir $(file:.c=.o)))
OBJS += $(foreach file,$(filter %.S,$(ABSOLUTE_AFILES)),$(BUILD_PATH)/$(notdir $(shell dirname $(file)))/$(notdir $(file:.S=.o)))

ifeq ($(CONFIG_ENABLE_CXX),y)
OBJS += $(foreach file,$(filter %.cpp,$(ABSOLUTE_CXXFILES)),$(BUILD_PATH)/$(notdir $(shell dirname $(file)))/$(notdir $(file:.cpp=.o)))
OBJS += $(foreach file,$(filter %.cc,$(ABSOLUTE_CXXFILES)),$(BUILD_PATH)/$(notdir $(shell dirname $(file)))/$(notdir $(file:.cc=.o)))
endif

CTARGET = ${2}/$(notdir $(shell dirname $(1)))/$(notdir $(patsubst %.c,%.o,$(abspath ${1})))
ATARGET = ${2}/$(notdir $(shell dirname $(1)))/$(notdir $(patsubst %.S,%.o,$(abspath ${1})))

ifeq ($(CONFIG_ENABLE_CXX),y)
CXXTARGET = ${2}/$(notdir $(shell dirname $(1)))/$(notdir $(patsubst %.cpp,%.o,$(abspath ${1})))
endif

define COMPILE_C
$(call CTARGET, ${1}, $(BUILD_PATH)) : ${1}
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname $(1)))
	@$(CC) $(CFLAGS) $(INCLUDE_PATH) -c -o $(call CTARGET, ${1}, $(BUILD_PATH)) ${1}
endef

define COMPILE_AS
$(call ATARGET, ${1}, $(BUILD_PATH)) : ${1}
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname $(1)))
	@$(CC) $(AFLAGS) $(INCLUDE_PATH) -c -o $(call ATARGET, ${1}, $(BUILD_PATH)) ${1}
endef

define COMPILE_CXX
$(call CXXTARGET, ${1}, $(BUILD_PATH)) : ${1} 
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname $(1)))
	@$(CXX) $(CXXFLAGS) $(INCLUDE_PATH) -c -o $(call CXXTARGET, ${1}, $(BUILD_PATH)) ${1}
endef

$(foreach cfile, $(ABSOLUTE_CFILES), $(eval $(call COMPILE_C, $(cfile))))
$(foreach afile, $(ABSOLUTE_AFILES), $(eval $(call COMPILE_AS, $(afile))))
ifeq ($(CONFIG_ENABLE_CXX),y)
$(foreach cxxfile, $(ABSOLUTE_CXXFILES), $(eval $(call COMPILE_CXX, $(cxxfile))))
endif

$(LIBS): $(OBJS)
	$(Q) echo "  AR    $@"
	$(Q) rm -f $@
	$(Q) $(AR)  $@ $^


# precompiler


DEPENDS_OBJS := $(foreach file,$(filter %.c,$(ABSOLUTE_CFILES)),$(BUILD_PATH)/$(notdir $(shell dirname $(file)))/$(notdir $(file:.c=.d)))
DEPENDS_OBJS += $(foreach file,$(filter %.S,$(ABSOLUTE_AFILES)),$(BUILD_PATH)/$(notdir $(shell dirname $(file)))/$(notdir $(file:.S=.d)))

ifeq ($(CONFIG_ENABLE_CXX),y)
DEPENDS_OBJS += $(foreach file,$(filter %.cpp,$(ABSOLUTE_CXXFILES)),$(BUILD_PATH)/$(notdir $(shell dirname $(file)))/$(notdir $(file:.cpp=.d)))
DEPENDS_OBJS += $(foreach file,$(filter %.cc,$(ABSOLUTE_CXXFILES)),$(BUILD_PATH)/$(notdir $(shell dirname $(file)))/$(notdir $(file:.cc=.d)))
endif

define CATFILE
	@ if [ -z "$(strip $(2))" ]; then echo '' > $(1); else cat $(2) > $1; fi
endef

CDEPS = ${2}/$(notdir $(shell dirname $(1)))/$(notdir $(patsubst %.c,%.d,$(abspath ${1})))
ADEPS = ${2}/$(notdir $(shell dirname $(1)))/$(notdir $(patsubst %.S,%.d,$(abspath ${1})))


ifeq ($(CONFIG_ENABLE_CXX),y)
CXXDEPS =  ${2}/$(notdir $(shell dirname $(1)))/$(notdir $(patsubst %.cpp,%.d,$(abspath ${1})))
endif

define DEP_C
$(call CDEPS, ${1}, $(BUILD_PATH)) : ${1} 
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname $(1)))
	@$(CC) $(CFLAGS) $(INCLUDE_PATH) -M -MG -MT $(call CTARGET, ${1}, $(BUILD_PATH)) -MF $(call CDEPS, ${1}, $(BUILD_PATH)) ${1}
endef


define DEP_A
$(call ADEPS, ${1}, $(BUILD_PATH)) : ${1} 
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname $(1)))
	@$(CC) $(CFLAGS) $(INCLUDE_PATH) -M -MG -MT $(call ATARGET, ${1}, $(BUILD_PATH)) -MF $(call ADEPS, ${1}, $(BUILD_PATH)) ${1}
endef

define DEP_CXX
$(call CXXDEPS, ${1}, $(BUILD_PATH)) : ${1} 
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname $(1)))
	@$(CXX) $(CXXFLAGS) $(INCLUDE_PATH) -M -MG -MT $(call CXXTARGET, ${1}, $(BUILD_PATH)) -MF $(call CXXDEPS, ${1}, $(BUILD_PATH)) ${1}
endef

$(foreach cfile, $(ABSOLUTE_CFILES), $(eval $(call DEP_C, $(cfile))))
$(foreach afile, $(ABSOLUTE_AFILES), $(eval $(call DEP_A, $(afile))))
ifeq ($(CONFIG_ENABLE_CXX),y)
$(foreach cxxfile, $(ABSOLUTE_CXXFILES), $(eval $(call DEP_CXX, $(cxxfile))))
endif

depend: $(DEPENDS_OBJS)
	$(Q) mkdir -p $(BUILD_PATH)
	$(call CATFILE, $(BUILD_PATH)/Make.dep, $^)


# debug


DEBUG_OBJS := $(foreach file,$(filter %.c,$(ABSOLUTE_CFILES)),$(BUILD_PATH)/$(notdir $(shell dirname $(file)))/$(notdir $(file:.c=.dis)))
DEBUG_OBJS += $(foreach file,$(filter %.S,$(ABSOLUTE_AFILES)),$(BUILD_PATH)/$(notdir $(shell dirname $(file)))/$(notdir $(file:.S=.dis)))

ifeq ($(CONFIG_ENABLE_CXX),y)
DEBUG_OBJS += $(foreach file,$(filter %.cpp,$(ABSOLUTE_CXXFILES)),$(BUILD_PATH)/$(notdir $(shell dirname $(file)))/$(notdir $(file:.cpp=.dis)))
DEBUG_OBJS += $(foreach file,$(filter %.cc,$(ABSOLUTE_CXXFILES)),$(BUILD_PATH)/$(notdir $(shell dirname $(file)))/$(notdir $(file:.cc=.dis)))
endif

CDIS = ${2}/$(notdir $(shell dirname $(1)))/$(notdir $(patsubst %.c,%.dis,$(abspath ${1})))
ADIS = ${2}/$(notdir $(shell dirname $(1)))/$(notdir $(patsubst %.S,%.dis,$(abspath ${1})))

ifeq ($(CONFIG_ENABLE_CXX),y)
CXXDIS =  ${2}/$(notdir $(shell dirname $(1)))/$(notdir $(patsubst %.cpp,%.dis,$(abspath ${1})))
endif

define DEBUG_C
$(call CDIS, ${1}, $(BUILD_PATH)) : ${1} 
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname $(1)))
	$(Q) $(CC) $(CFLAGS) $(INCLUDE_PATH) -E -P -o $(call CDIS, ${1}, $(BUILD_PATH)) ${1}
endef

define DEBUG_A
$(call ADIS, ${1}, $(BUILD_PATH)) : ${1} 
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname $(1)))
	$(Q) $(CC) $(AFLAGS) $(INCLUDE_PATH) -E -P -o $(call ADIS, ${1}, $(BUILD_PATH)) ${1}
endef

define DEBUG_CXX
$(call CXXDIS, ${1}, $(BUILD_PATH)) : ${1} 
	@mkdir -p $(BUILD_PATH)/$(notdir $(shell dirname $(1)))
	@$(CXX) $(CXXFLAGS) $(INCLUDE_PATH) -E -P -o $(call CXXDIS, ${1}, $(BUILD_PATH)) ${1}
endef

$(foreach cfile, $(ABSOLUTE_CFILES), $(eval $(call DEBUG_C, $(cfile))))
$(foreach afile, $(ABSOLUTE_AFILES), $(eval $(call DEBUG_A, $(afile))))
ifeq ($(CONFIG_ENABLE_CXX),y)
$(foreach cxxfile, $(ABSOLUTE_CXXFILES), $(eval $(call DEBUG_CXX, $(cxxfile))))
endif

debug:  $(DEBUG_OBJS)

compiler_info:
	-@echo The dependent path during compilation :
	-$(foreach include, $(INCLUDE_PATH), @echo $(include) "\n")
	-@echo The relative C files path during compilation :
	-$(foreach cfiles, $(CSRCS_RELATIVE_FILES), @echo $(cfiles) "\n")
	-@echo The absolute C files path during compilation :
	-$(foreach cfiles, $(ABSOLUTE_CFILES), @echo $(cfiles) "\n")
	-@echo The absolute asm files path during compilation :
	-$(foreach cfiles, $(ABSOLUTE_AFILES), @echo $(cfiles) "\n")

	-@echo OBJS:
	-$(foreach cfiles, $(OBJS), @echo $(cfiles) "\n")
	
	-@echo The CXX files path during compilation :
	-$(foreach cxx_files, $(ABSOLUTE_CXXFILES), @echo $(cxx_files) "\n")
	-@echo CXXSRCS_FLIES: $(CXXSRCS_FLIES) 
	-@echo CFLAGS : $(CFLAGS)
	-@echo AFLAGS : $(AFLAGS)
	-@echo CXXFLAGS : $(CXXFLAGS)
	-@echo LDFLAGS : $(LDFLAGS)
	-@echo BUILD_PATH : $(BUILD_PATH)
	-@echo OBJS_WITH_DIRS : $(OBJS_WITH_DIRS)
	-@echo ABSOLUTE_CXXFILES :$(ABSOLUTE_CXXFILES)
# -@echo DEPENDS_OBJS: $(DEPENDS_OBJS)



	
-include $(BUILD_PATH)/Make.dep


.PHONY: all depend debug