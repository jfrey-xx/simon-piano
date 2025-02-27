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
BUILD_CXX_FLAGS += -I../../src -I../../raylib/src -I../../dpf-widgets/opengl  
# for raylib
#BUILD_C_FLAGS += -I../../raylib/src -I../../raylib/src/external/glfw/include -Wall -D_GNU_SOURCE -DPLATFORM_DESKTOP_GLFW -DGRAPHICS_API_OPENGL_33 -Wno-missing-braces -Werror=pointer-arith -fno-strict-aliasing -std=c99 -fPIC -O1 -Werror=implicit-function-declaration -D_GLFW_X11 -U_GNU_SOURCE
#BUILD_C_FLAGS += -I../../raylib/src -I../../raylib/src/external/glfw/include  -DPLATFORM_DESKTOP_RGFW -DGRAPHICS_API_OPENGL_33  -D_GLFW_X11
BUILD_C_FLAGS += -I../../raylib/src -DPLATFORM_DPF -DGRAPHICS_API_OPENGL_33 

#BUILD_C_FLAGS += -I../../raylib/src -DPLATFORM_DPF -DGRAPHICS_API_OPENGL_33 -Wall -Wextra -pipe -MD -MP -fno-gnu-unique -fPIC -DPIC -DNDEBUG -O3 -ffast-math -fdata-sections -ffunction-sections -mtune=generic -msse -msse2 -mfpmath=sse -fvisibility=hidden -DHAVE_ALSA -DHAVE_JACK -DHAVE_LIBLO -DHAVE_RTAUDIO -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include  -DHAVE_DBUS  -DHAVE_X11  -DHAVE_XCURSOR  -DHAVE_XEXT -DHAVE_XSYNC  -DHAVE_XRANDR -DHAVE_CAIRO -DHAVE_OPENGL -DDGL_OPENGL -DHAVE_DGL  -fdata-sections -ffunction-sections -Wl,-O1,--as-needed,--gc-sections -Wl,--strip-all  -Wl,--no-undefined -ldl    -lGL -lX11  -ldbus-1  -lX11  -lXcursor  -lXext  -lXrandr  -lm -lasound  -lpthread  

#BUILD_C_FLAGS += -I../../raylib/src -Wall -D_GNU_SOURCE -DPLATFORM_DPF -DGRAPHICS_API_OPENGL_33 -Wno-missing-braces -Werror=pointer-arith -fno-strict-aliasing -std=c99 -fPIC -O1 -Werror=implicit-function-declaration  -U_GNU_SOURCE
#LINK_FLAGS += -lglfw -lGL -lX11  -lGL -lm -lX11 -lpthread -lXi -lXrandr -ldl 


#LINK_FLAGS += ../../raylib/src/libraylib.a
# --------------------------------------------------------------
# Select plugins

TARGETS += jack
#TARGETS += lv2_sep
#TARGETS += vst2
#TARGETS += vst3
#TARGETS += clap

all: $(TARGETS)
