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
# note: for web export only jack will be forced

TARGETS += jack
TARGETS += lv2_sep
TARGETS += vst2
TARGETS += vst3
TARGETS += clap

# --------------------------------------------------------------
# Do some magic -- customized for using raylib UI and copying resources

include ../../src/Makefile.rayui.mk

# --------------------------------------------------------------
# And... action

all: $(TARGETS) resources
