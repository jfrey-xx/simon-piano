#!/usr/bin/make -f

# Makefile for using raylib UI through DPF

# set path relative to this folder location no matter where the file is included
RAYUI_PATH := $(dir $(lastword $(MAKEFILE_LIST)))

# adding required file for UI
FILES_UI += \
	$(RAYUI_PATH)/RayUI.cpp \
	$(RAYUI_PATH)/raylib/src/rcore.c \
	$(RAYUI_PATH)/raylib/src/rshapes.c \
	$(RAYUI_PATH)/raylib/src/rtext.c \
	$(RAYUI_PATH)/raylib/src/rtextures.c \
	$(RAYUI_PATH)/raylib/src/rmodels.c \
	$(RAYUI_PATH)/raylib/src/utils.c

# now the regular makefile for plugins (order matters)
include $(RAYUI_PATH)/../dpf/Makefile.plugins.mk

# Adding  custom libs
BUILD_CXX_FLAGS += -I$(RAYUI_PATH) -I$(RAYUI_PATH)/raylib/src -DPLATFORM_DPF
# for raylib
BUILD_C_FLAGS += -I$(RAYUI_PATH)/raylib/src -DPLATFORM_DPF -DGRAPHICS_API_OPENGL_ES2
