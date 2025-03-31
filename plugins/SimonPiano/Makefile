#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #

# --------------------------------------------------------------
# Project name, used for binaries

NAME = simon-piano

# --------------------------------------------------------------
# Files to build

FILES_DSP = \
	SimonPiano.cpp

FILES_UI = \
	SimonPianoUI.cpp 

# --------------------------------------------------------------
# Do some magic -- customized for using raylib UI

# everything located in a "resources" folder will be bundled with output -- even if empty 
RESOURCES = $(wildcard $(CURDIR)/resources)
# if we have resources to copy, we need to set bundle to all targets with resources
# WARNING: in case resources come and go we might have duplicated those targets
ifneq ($(RESOURCES),)
	USE_VST2_BUNDLE=true
	USE_CLAP_BUNDLE=true
endif

include ../../src/Makefile.rayui.mk

# --------------------------------------------------------------
# Select plugins

TARGETS += jack
#TARGETS += lv2_sep
#TARGETS += vst2
#TARGETS += vst3
#TARGETS += clap

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
# FIXME: in case there are several plugins with jack, one will take precedence. Might want to copy or recursively link instead
resources: $(TARGETS) $(RESOURCES)
ifneq ($(RESOURCES),)
ifeq ($(findstring jack,$(TARGETS)),jack)
	@echo "copy resources to jack"
# note: -n to avoid recursively upon re-rerunning
	ln -snf $(RESOURCES) $(JACK_RESOURCES_DIR)
	echo $(RESOURCES) $(JACK_RESOURCES_DIR)
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


all: $(TARGETS) resources
