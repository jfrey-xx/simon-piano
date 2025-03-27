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
	../../src/RayUI.cpp \
	../../raylib/src/rcore.c \
	../../raylib/src/rshapes.c \
	../../raylib/src/rtext.c \
	../../raylib/src/rtextures.c \
	../../raylib/src/rmodels.c \
	../../raylib/src/utils.c

# --------------------------------------------------------------
# Do some magic

include ../../dpf/Makefile.plugins.mk

# --------------------------------------------------------------
# Adding  custom libs
BUILD_CXX_FLAGS += -I../../src -I../../raylib/src -DPLATFORM_DPF
# for raylib
BUILD_C_FLAGS += -I../../raylib/src -DPLATFORM_DPF -DGRAPHICS_API_OPENGL_ES2

# --------------------------------------------------------------
# Select plugins

TARGETS += jack
#TARGETS += lv2_sep
#TARGETS += vst2
#TARGETS += vst3
#TARGETS += clap

all: $(TARGETS)
