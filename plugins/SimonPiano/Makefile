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
# Select plugins
# (to be set before including Makefile.rayui.mk for resources to work)

TARGETS += jack
TARGETS += lv2_sep
TARGETS += vst2
TARGETS += vst3
TARGETS += clap

# --------------------------------------------------------------
# Do some magic -- customized for using raylib UI and copying resources

include ../../src/Makefile.rayui.mk

# WIP: web version (with target jack), mightwork better with PLATFORM_WEB and GL ES 3 on the raylib side and flag USE_GLFW=3 here
# also, "DISTRHO_PLUGIN_NUM_OUTPUTS 2" for code to compile, "DISTRHO_PLUGIN_NUM_INPUTS 2"  to run without complaining about webaudio 
# finally, grab a html emplate, e.g. from ildaeil, at least to fix mouse mapping
# ...and for requestMIDI() to work as of emsdk 4.0.6, WebBrigde.hpp in DPF needs to be modified, Module._malloc -> _malloc
ifeq ($(WASM),true)
# embed resources, otherwise cannot be accessed
LINK_FLAGS += --preload-file=./resources
# needed for loading custom raygui style
LINK_FLAGS += -sALLOW_MEMORY_GROWTH
endif

# --------------------------------------------------------------
# And... action

all: $(TARGETS) resources
