NEHE_NXGL_APPS = \
	111_nehe_nxgl_01_window \
	112_nehe_nxgl_02_first_polygons \
	113_nehe_nxgl_03_color \
	114_nehe_nxgl_04_rotation \
	115_nehe_nxgl_05_3d_shapes \
	116_nehe_nxgl_06_texture_mapping \
	117_nehe_nxgl_07_filters_lighting \
	118_nehe_nxgl_08_blending \
	119_nehe_nxgl_09_moving_bitmaps \
	120_nehe_nxgl_10_3d_world \
	121_nehe_nxgl_11_flag_effect \
	122_nehe_nxgl_12_display_lists \
	213_nehe_nxgl_13_bitmap_fonts \
	214_nehe_nxgl_14_outline_fonts \
	215_nehe_nxgl_15_texture_mapped_outline_fonts

NEHE_PB_APPS = \
	123_nehe_pb_01_window \
	124_nehe_pb_02_first_polygons \
	125_nehe_pb_03_color \
	126_nehe_pb_04_rotation \
	127_nehe_pb_05_3d_shapes \
	128_nehe_pb_06_texture_mapping \
	129_nehe_pb_07_filters_lighting \
	130_nehe_pb_08_blending \
	131_nehe_pb_09_moving_bitmaps \
	132_nehe_pb_10_3d_world \
	133_nehe_pb_11_flag_effect \
	134_nehe_pb_12_display_lists \
	313_nehe_pb_13_bitmap_fonts \
	314_nehe_pb_14_outline_fonts \
	315_nehe_pb_15_texture_mapped_outline_fonts

NEHE_APPS = $(NEHE_NXGL_APPS) $(NEHE_PB_APPS)

.PHONY: all clean distclean release print-nehe-apps print-nehe-nxgl-apps print-nehe-pb-apps $(NEHE_APPS)

all: $(NEHE_APPS)

$(NEHE_APPS):
	$(MAKE) -C $@

clean:
	@for app in $(NEHE_APPS); do $(MAKE) -C $$app clean || exit $$?; done

distclean: clean
	rm -rf dist

release: all
	./tools/collect_release_isos.sh

print-nehe-apps:
	@printf '%s\n' $(NEHE_APPS)

print-nehe-nxgl-apps:
	@printf '%s\n' $(NEHE_NXGL_APPS)

print-nehe-pb-apps:
	@printf '%s\n' $(NEHE_PB_APPS)
