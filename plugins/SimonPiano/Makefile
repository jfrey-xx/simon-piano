#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
#

# --------------------------------------------------------------
# Project name, used for binaries

NAME = simon-piano

# --------------------------------------------------------------
# Files to build

FILES_DSP = \
	SimonPiano.cpp

# --------------------------------------------------------------
# Do some magic

include ../../dpf/Makefile.plugins.mk

# --------------------------------------------------------------
# Adding  custom libs
BUILD_CXX_FLAGS += -I../../src

# --------------------------------------------------------------
# Select plugins

TARGETS += jack
TARGETS += lv2_sep
TARGETS += vst2
TARGETS += vst3
TARGETS += clap

all: $(TARGETS)
