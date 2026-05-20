XBE_TITLE = xbnehe_242_nehe_nxgl_42_multiple_viewports
GEN_XISO = $(XBE_TITLE).iso
NXDK_DIR ?= $(CURDIR)/../../.nxdk
NXGL_DIR ?= $(CURDIR)/../../nxgl

include $(NXGL_DIR)/nxgl.mk

SRCS = $(NXGL_SRCS) $(CURDIR)/main.c
SHADER_OBJS = $(NXGL_SHADER_OBJS)
CFLAGS += -I$(CURDIR)/../common_nehe $(NXGL_CFLAGS)
include $(CURDIR)/../common_nehe/nehe_capture.mk
include $(NXDK_DIR)/Makefile
