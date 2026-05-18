XBE_TITLE = xbnehe_110_nxgl_demo_disk
GEN_XISO = $(XBE_TITLE).iso
NXDK_DIR ?= $(CURDIR)/../../.nxdk
NXGL_DIR ?= $(CURDIR)/../../nxgl

include $(NXGL_DIR)/nxgl.mk

SRCS = $(NXGL_SRCS) $(CURDIR)/main.c
SHADER_OBJS = $(NXGL_SHADER_OBJS)
CFLAGS += -I$(CURDIR)/../common_nehe $(NXGL_CFLAGS)
include $(NXDK_DIR)/Makefile

