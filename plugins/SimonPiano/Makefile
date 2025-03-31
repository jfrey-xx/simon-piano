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

include ../../src/Makefile.rayui.mk

# --------------------------------------------------------------
# Select plugins

TARGETS += jack
#TARGETS += lv2_sep
#TARGETS += vst2
#TARGETS += vst3
#TARGETS += clap

all: $(TARGETS)
