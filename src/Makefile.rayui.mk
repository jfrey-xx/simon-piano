#!/usr/bin/make -f

# Makefile for using raylib UI through DPF

# --------------------------------------------------------------
# tune for raylib

# set path relative to this folder location no matter where the file is included
RAYUI_PATH := $(dir $(lastword $(MAKEFILE_LIST)))

# adding required file for UI
FILES_UI += \
	$(RAYUI_PATH)/RayUI.cpp 

# everything located in a "resources" folder will be bundled with output -- even if empty 
RESOURCES = $(wildcard $(CURDIR)/resources)

# if we have resources to copy, we need to set bundle to all targets with resources
# WARNING: in case resources come and go we might have duplicated those targets
ifneq ($(RESOURCES),)
# note: to be set before Makefile.plugins.mk inclusions
	USE_VST2_BUNDLE=true
	USE_CLAP_BUNDLE=true
endif

# now the regular makefile for plugins (order matters)
include $(RAYUI_PATH)/../dpf/Makefile.plugins.mk

# points to headers files for raylib
BUILD_CXX_FLAGS += -I$(RAYUI_PATH) -I$(RAYUI_PATH)/raylib/src
ifeq ($(WASM),)
# we need to adapt how RayUI is tight to raylib depending on platform
BUILD_CXX_FLAGS += -DPLATFORM_DPF
LINK_FLAGS += $(RAYUI_PATH)/raylib/src/libraylib.a
else
# special satic lib name for web version
LINK_FLAGS += $(RAYUI_PATH)/raylib/src/libraylib.web.a
endif

# --------------------------------------------------------------
# list resources

# note: WASM defined through Makefile.plugins.mk inclusion
ifeq ($(WASM),true)
# with emscripten the true "resources" folder is exported, 
# WARNING: do not put in there files with the name-project.wasm, .js, .html, or .data, they will conflict with emscripten export
RESOURCES = $(wildcard $(CURDIR)/resources_web)
endif
# tuning or jacks, since plugins will share the same resource folder we want there to link all content instead
RESOURCES_CONTENT=$(wildcard $(RESOURCES)/*)
# include hidden files
RESOURCES_CONTENT+=$(wildcard $(RESOURCES)/.*)
# ...but not link to current and parent folder
RESOURCES_CONTENT:=$(filter-out $(RESOURCES)/. $(RESOURCES)/.., $(RESOURCES_CONTENT))

# --------------------------------------------------------------
# tune for WASM

# adjust flags for web version
ifeq ($(WASM),true)
# embed resources, otherwise cannot be accessed
LINK_FLAGS += --preload-file=./resources
# needed for loading custom raygui style
LINK_FLAGS += -sALLOW_MEMORY_GROWTH
# will be used in conjunction with PLATFORM_WEB
LINK_FLAGS += -sUSE_GLFW=3
# custom template for the final web page
LINK_FLAGS += --shell-file=./emscripten/shell.html
endif
# raylib requires some libraries not always inluded by DPF on mingw
ifeq ($(WINDOWS),true)
LINK_FLAGS += -lpthread # clock_gettime
LINK_FLAGS += -lwinmm # __imp_timeBeginPeriod
endif


# override target
ifeq ($(WASM),true)
TARGETS = jack
endif

# --------------------------------------------------------------
# Export resources

# set destination folders for resources, as per DistrhoPluginUtils.hpp
VST3_RESOURCES_DIR=$(TARGET_DIR)/$(NAME).vst3/Contents/Resources
LV2_RESOURCES_DIR=$(TARGET_DIR)/$(NAME).lv2/resources
# special case for jack target, use in web export: copy to root
ifeq ($(WASM),true)
JACK_RESOURCES_DIR=$(TARGET_DIR)/
else
ifeq ($(MACOS_APP_BUNDLE),true)
JACK_RESOURCES_DIR=$(TARGET_DIR)/$(NAME).app/Contents/Resources
else
JACK_RESOURCES_DIR=$(TARGET_DIR)/resources
endif
endif # WASM true
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
# HOTFIX: remove empty Resources folder set by DPF for MACOS, otherwise our resources will be a subfolder
resources: $(TARGETS) $(RESOURCES)
ifneq ($(RESOURCES),)
ifeq ($(findstring jack,$(TARGETS)),jack)
	@echo "copy resources to jack"
ifeq ($(MACOS),true)
	rm -f $(JACK_RESOURCES_DIR)/empty.lproj
endif
# note: -n to avoid recursively upon re-rerunning
	install -d $(JACK_RESOURCES_DIR)
ifneq ($(RESOURCES_CONTENT),)
	ln -snf $(RESOURCES_CONTENT) $(JACK_RESOURCES_DIR)
endif
endif
ifeq ($(findstring clap,$(TARGETS)),clap)
	@echo "copy resources to clap"
ifeq ($(MACOS),true)
	rm -f $(CLAP_RESOURCES_DIR)/empty.lproj
	rmdir $(CLAP_RESOURCES_DIR) || true
endif
	ln -snf $(RESOURCES) $(CLAP_RESOURCES_DIR)
endif
ifeq ($(findstring lv2,$(TARGETS)),lv2)
	@echo "copy resources to lv2"
	ln -snf $(RESOURCES) $(LV2_RESOURCES_DIR)
endif
ifeq ($(findstring vst3,$(TARGETS)),vst3)
	@echo "copy resources to vst3"
ifeq ($(MACOS),true)
	rm -f $(VST3_RESOURCES_DIR)/empty.lproj
	rmdir $(VST3_RESOURCES_DIR) || true
endif
	ln -snf $(RESOURCES) $(VST3_RESOURCES_DIR)
endif
ifeq ($(findstring vst2,$(TARGETS)),vst2)
	@echo "copy resources to vst2"
ifeq ($(MACOS),true)
	rm -f $(VST2_RESOURCES_DIR)/empty.lproj
	rmdir $(VST2_RESOURCES_DIR) || true
endif
	ln -snf $(RESOURCES) $(VST2_RESOURCES_DIR)
endif
endif

$(RAYUI_PATH)/raylib/src/libraylib.a:
# HOTFIX for WASM, switch to gles3 and platform web, will override pugl
ifeq ($(WASM),true)
	$(MAKE) all -C $(RAYUI_PATH)/raylib/src PLATFORM=PLATFORM_WEB GRAPHICS=GRAPHICS_API_OPENGL_ES3 RAYLIB_MODULE_AUDIO=FALSE RAYLIB_MODULE_RAYGUI=TRUE RAYLIB_MODULE_RAYGUI_PATH=../../
else
# macos needs not to talk about deprecation, we know
# note: both GRAPHICS_API_OPENGL_ES2 and GRAPHICS_API_OPENGL_33 working on macos and linux
ifeq ($(MACOS),true)
	$(MAKE) all -C $(RAYUI_PATH)/raylib/src PLATFORM=PLATFORM_DPF GRAPHICS=GRAPHICS_API_OPENGL_ES2 RAYLIB_MODULE_AUDIO=FALSE RAYLIB_MODULE_RAYGUI=TRUE RAYLIB_MODULE_RAYGUI_PATH=../../ CUSTOM_CFLAGS="-DGL_SILENCE_DEPRECATION"
else
	$(MAKE) all -C $(RAYUI_PATH)/raylib/src PLATFORM=PLATFORM_DPF GRAPHICS=GRAPHICS_API_OPENGL_ES2 RAYLIB_MODULE_AUDIO=FALSE RAYLIB_MODULE_RAYGUI=TRUE RAYLIB_MODULE_RAYGUI_PATH=../../
endif # MAC
endif # WASM

raylib: $(RAYUI_PATH)/raylib/src/libraylib.a
