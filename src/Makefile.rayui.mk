#!/usr/bin/make -f

# Makefile for using raylib UI through DPF

# --------------------------------------------------------------
# list resources

# everything located in a "resources" folder will be bundled with output -- even if empty 
RESOURCES = $(wildcard $(CURDIR)/resources)
# tuning or jacks, since plugins will share the same resource folder we want there to link all content instead
RESOURCES_CONTENT=$(wildcard $(CURDIR)/resources/*)
# include hidden files
RESOURCES_CONTENT+=$(wildcard $(CURDIR)/resources/.*)
# ...but not link to current and parent folder
RESOURCES_CONTENT:=$(filter-out $(CURDIR)/resources/. $(CURDIR)/resources/.., $(RESOURCES_CONTENT))

# if we have resources to copy, we need to set bundle to all targets with resources
# WARNING: in case resources come and go we might have duplicated those targets
ifneq ($(RESOURCES),)
	USE_VST2_BUNDLE=true
	USE_CLAP_BUNDLE=true
endif

# --------------------------------------------------------------
# tune for raylib

# set path relative to this folder location no matter where the file is included
RAYUI_PATH := $(dir $(lastword $(MAKEFILE_LIST)))

# adding required file for UI
FILES_UI += \
	$(RAYUI_PATH)/RayUI.cpp \
	$(RAYUI_PATH)/raylib/src/rcore.c \
	$(RAYUI_PATH)/raylib/src/rshapes.c \
	$(RAYUI_PATH)/raylib/src/rtext.c \
	$(RAYUI_PATH)/raylib/src/rtextures.c \
	$(RAYUI_PATH)/raylib/src/rmodels.c \
	$(RAYUI_PATH)/raylib/src/utils.c

# now the regular makefile for plugins (order matters)
include $(RAYUI_PATH)/../dpf/Makefile.plugins.mk

# points to headers files and options for raylibs
# HOTFIX for WASM, switch to gles3 and platform web will override pugl
ifeq ($(WASM),true)
BUILD_CXX_FLAGS += -I$(RAYUI_PATH) -I$(RAYUI_PATH)/raylib/src -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES3
BUILD_C_FLAGS += -I$(RAYUI_PATH)/raylib/src -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES3
else
BUILD_CXX_FLAGS += -I$(RAYUI_PATH) -I$(RAYUI_PATH)/raylib/src -DPLATFORM_DPF  -DGRAPHICS_API_OPENGL_ES2
BUILD_C_FLAGS += -I$(RAYUI_PATH)/raylib/src -DPLATFORM_DPF  -DGRAPHICS_API_OPENGL_ES2
endif

# --------------------------------------------------------------
# tune for WASM

# adjust flags for web version (with target jack)
# FIXME: for requestMIDI() to work as of emsdk 4.0.6, WebBrigde.hpp in DPF needs to be modified, Module._malloc -> _malloc
ifeq ($(WASM),true)
# embed resources, otherwise cannot be accessed
LINK_FLAGS += --preload-file=./resources
# needed for loading custom raygui style
LINK_FLAGS += -sALLOW_MEMORY_GROWTH
# will be used in conjunction with PLATFORM_WEB
LINK_FLAGS += -sUSE_GLFW=3
LINK_FLAGS += --shell-file=./emscripten/shell.html
endif

# --------------------------------------------------------------
# Export resources

# set destination folders for resources, as per DistrhoPluginUtils.hpp
VST3_RESOURCES_DIR=$(TARGET_DIR)/$(NAME).vst3/Contents/Resources
LV2_RESOURCES_DIR=$(TARGET_DIR)/$(NAME).lv2/resources
ifeq ($(MACOS_APP_BUNDLE),true)
JACK_RESOURCES_DIR=$(TARGET_DIR)/$(NAME).app/Contents/Resources
else
JACK_RESOURCES_DIR=$(TARGET_DIR)/resources
endif
ifeq ($(MACOS),true)
VST2_RESOURCES_DIR=$(TARGET_DIR)/$(NAME).vst/Contents/Resources
CLAP_RESOURCES_DIR=$(TARGET_DIR)/$(NAME).clap/Contents/Resources
else
VST2_RESOURCES_DIR=$(TARGET_DIR)/$(NAME).vst/resources
CLAP_RESOURCES_DIR=$(TARGET_DIR)/$(NAME).clap/resources
endif

# copy (at this stage, link) content of resources folder to destination
# TODO: do not run again if already performed
# WARNING with jack: in case there are several plugins with jack and resources with the same name, one will take precedence
# FIXME: if resources are deleted or renamed between two runs, links will be broken. clean between.
resources: $(TARGETS) $(RESOURCES)
ifneq ($(RESOURCES),)
ifeq ($(findstring jack,$(TARGETS)),jack)
	@echo "copy resources to jack"
# note: -n to avoid recursively upon re-rerunning
	install -d $(JACK_RESOURCES_DIR)
ifneq ($(RESOURCES_CONTENT),)
	ln -snf $(RESOURCES_CONTENT) $(JACK_RESOURCES_DIR)
endif
endif
ifeq ($(findstring clap,$(TARGETS)),clap)
	@echo "copy resources to clap"
	ln -snf $(RESOURCES) $(CLAP_RESOURCES_DIR)
endif
ifeq ($(findstring lv2,$(TARGETS)),lv2)
	@echo "copy resources to lv2"
	ln -snf $(RESOURCES) $(LV2_RESOURCES_DIR)
endif
ifeq ($(findstring vst3,$(TARGETS)),vst3)
	@echo "copy resources to vst3"
	ln -snf $(RESOURCES) $(VST3_RESOURCES_DIR)
endif
ifeq ($(findstring vst2,$(TARGETS)),vst2)
	@echo "copy resources to vst2"
	ln -snf $(RESOURCES) $(VST2_RESOURCES_DIR)
endif
endif
