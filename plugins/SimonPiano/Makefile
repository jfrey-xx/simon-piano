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
	../../dpf-widgets/opengl/DearImGui.cpp \
	../../raylib/src/rcore.c \
	../../raylib/src/rshapes.c \
	../../raylib/src/rtext.c \
	../../raylib/src/rtextures.c \
	../../raylib/src/utils.c


# --------------------------------------------------------------
# Do some magic

include ../../dpf/Makefile.plugins.mk

# --------------------------------------------------------------
# Adding  custom libs
BUILD_CXX_FLAGS += -I../../src -I../../ray/include -I../../dpf-widgets/opengl -g
# for raylib
#BUILD_C_FLAGS += -I../../raylib/src -I../../raylib/src/external/glfw/include -Wall -D_GNU_SOURCE -DPLATFORM_DESKTOP_GLFW -DGRAPHICS_API_OPENGL_33 -Wno-missing-braces -Werror=pointer-arith -fno-strict-aliasing -std=c99 -fPIC -O1 -Werror=implicit-function-declaration -D_GLFW_X11 -U_GNU_SOURCE
#BUILD_C_FLAGS += -I../../raylib/src -I../../raylib/src/external/glfw/include  -DPLATFORM_DESKTOP_RGFW -DGRAPHICS_API_OPENGL_33  -D_GLFW_X11
BUILD_C_FLAGS += -I../../raylib/src -DPLATFORM_DPF -DGRAPHICS_API_OPENGL_33 -g
#LINK_FLAGS += -lglfw -lGL -lX11  -lGL -lm -lX11 -lpthread -lXi -lXrandr -ldl 


#LINK_FLAGS += ../../ray/lib/libraylib.a
# --------------------------------------------------------------
# Select plugins

TARGETS += jack
#TARGETS += lv2_sep
#TARGETS += vst2
#TARGETS += vst3
#TARGETS += clap

all: $(TARGETS)
