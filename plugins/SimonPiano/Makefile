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
	SimonPianoUI.cpp \
	../../dpf-widgets/opengl/DearImGui.cpp

# --------------------------------------------------------------
# Do some magic

include ../../dpf/Makefile.plugins.mk

# --------------------------------------------------------------
# Adding  custom libs
BUILD_CXX_FLAGS += -I../../src -I../../dpf-widgets/opengl

# --------------------------------------------------------------
# Select plugins

TARGETS += jack
#TARGETS += lv2_sep
#TARGETS += vst2
#TARGETS += vst3
#TARGETS += clap

all: $(TARGETS)
