#!umsr/bin/make -f
# Makefile for DPF #
# ---------------- #
# Created by falkTX
#

USE_GLES2=true

include dpf/Makefile.base.mk

all: dgl examples gen

# --------------------------------------------------------------

ifneq ($(CROSS_COMPILING),true)
CAN_GENERATE_TTL = true
else ifneq ($(EXE_WRAPPER),)
CAN_GENERATE_TTL = true
endif

dgl:
ifeq ($(HAVE_OPENGL),true)
	$(MAKE) -C dpf/dgl opengl
endif

plugins: dgl
	$(MAKE) all -C plugins/SimonPiano

ifeq ($(CAN_GENERATE_TTL),true)
gen: plugins utils/lv2_ttl_generator
	@$(CURDIR)/dpf/utils/generate-ttl.sh

utils/lv2_ttl_generator:
	$(MAKE) -C dpf/utils/lv2-ttl-generator
else
gen:
endif

tests: dgl
	$(MAKE) -C tests

# --------------------------------------------------------------

clean:
	$(MAKE) clean -C dpf/dgl
	$(MAKE) clean -C plugins/SimonPiano
	$(MAKE) clean -C dpf/utils/lv2-ttl-generator
	$(MAKE) clean -C src
	rm -rf bin build 

# --------------------------------------------------------------

.PHONY: dgl examples tests
