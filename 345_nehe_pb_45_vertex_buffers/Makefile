XBE_TITLE = xbnehe_345_nehe_pb_45_vertex_buffers
GEN_XISO = $(XBE_TITLE).iso
SRCS = $(CURDIR)/../common_nehe/nehe_native.c $(CURDIR)/nehe_lessons.c $(CURDIR)/main.c
SHADER_OBJS = $(CURDIR)/../common3d/ps.inl $(CURDIR)/../common3d/vs.inl $(CURDIR)/../common_nehe/nehe_tex_ps.inl $(CURDIR)/../common_nehe/nehe_tex_replace_ps.inl $(CURDIR)/../common_nehe/nehe_tex_decal_ps.inl $(CURDIR)/../common_nehe/nehe_tex_blend_ps.inl $(CURDIR)/../common_nehe/nehe_tex_add_ps.inl $(CURDIR)/../common_nehe/nehe_tex_subtract_ps.inl $(CURDIR)/../common_nehe/nehe_tex_add_signed_ps.inl $(CURDIR)/../common_nehe/nehe_tex_interpolate_ps.inl $(CURDIR)/../common_nehe/nehe_tex_vs.inl $(CURDIR)/../common_nehe/nehe_tex2_vs.inl $(CURDIR)/../common_nehe/nehe_tex2_modulate_ps.inl $(CURDIR)/../common_nehe/nehe_cube_ps.inl $(CURDIR)/../common_nehe/nehe_tex3d_ps.inl
NXDK_DIR ?= $(CURDIR)/../../.nxdk
CFLAGS += -I$(CURDIR)/../common_nehe

include $(CURDIR)/../common_nehe/nehe_capture.mk
include $(NXDK_DIR)/Makefile
