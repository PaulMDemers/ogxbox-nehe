XBE_TITLE = xbnehe_119_nxgl_09_moving_bitmaps
GEN_XISO = $(XBE_TITLE).iso
NXDK_DIR ?= $(CURDIR)/../../.nxdk
NXGL_DIR ?= $(CURDIR)/../../nxgl

include $(NXGL_DIR)/nxgl.mk

SRCS = $(NXGL_SRCS) $(CURDIR)/main.c
SHADER_OBJS = $(NXGL_SHADER_OBJS)
CFLAGS += -I$(CURDIR)/../common_nehe $(NXGL_CFLAGS)
include $(CURDIR)/../common_nehe/nehe_capture.mk
include $(NXDK_DIR)/Makefile

