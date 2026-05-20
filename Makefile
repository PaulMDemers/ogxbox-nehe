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
	215_nehe_nxgl_15_texture_mapped_outline_fonts \
	216_nehe_nxgl_16_fog \
	217_nehe_nxgl_17_texture_fonts \
	218_nehe_nxgl_18_quadrics \
	219_nehe_nxgl_19_particle_engine \
	220_nehe_nxgl_20_masking \
	221_nehe_nxgl_21_lines_timing_ortho \
	222_nehe_nxgl_22_bump_mapping \
	223_nehe_nxgl_23_sphere_mapping \
	224_nehe_nxgl_24_tokens_scissor_tga \
	225_nehe_nxgl_25_morphing_loading_objects \
	226_nehe_nxgl_26_stencil_reflections \
	227_nehe_nxgl_27_shadows \
	228_nehe_nxgl_28_bezier_patches \
	229_nehe_nxgl_29_blitter_raw_textures \
	230_nehe_nxgl_30_collision_detection \
	231_nehe_nxgl_31_model_loading \
	232_nehe_nxgl_32_picking_sorting \
	233_nehe_nxgl_33_tga_variants \
	234_nehe_nxgl_34_height_map_terrain \
	235_nehe_nxgl_35_avi_texture_playback \
	236_nehe_nxgl_36_radial_blur \
	237_nehe_nxgl_37_cel_shading \
	238_nehe_nxgl_38_resource_textures \
	239_nehe_nxgl_39_physics_simulation \
	240_nehe_nxgl_40_rope_physics \
	241_nehe_nxgl_41_volumetric_fog \
	242_nehe_nxgl_42_multiple_viewports \
	243_nehe_nxgl_43_freetype_fonts \
	244_nehe_nxgl_44_lens_flare \
	245_nehe_nxgl_45_vertex_buffers \
	246_nehe_nxgl_46_antialiasing \
	247_nehe_nxgl_47_vertex_shader \
	248_nehe_nxgl_48_arcball_rotation

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
	315_nehe_pb_15_texture_mapped_outline_fonts \
	316_nehe_pb_16_fog \
	317_nehe_pb_17_texture_fonts \
	318_nehe_pb_18_quadrics \
	319_nehe_pb_19_particle_engine \
	320_nehe_pb_20_masking \
	321_nehe_pb_21_lines_timing_ortho \
	322_nehe_pb_22_bump_mapping \
	323_nehe_pb_23_sphere_mapping \
	324_nehe_pb_24_tokens_scissor_tga \
	325_nehe_pb_25_morphing_loading_objects \
	326_nehe_pb_26_stencil_reflections \
	327_nehe_pb_27_shadows \
	328_nehe_pb_28_bezier_patches \
	329_nehe_pb_29_blitter_raw_textures \
	330_nehe_pb_30_collision_detection \
	331_nehe_pb_31_model_loading \
	332_nehe_pb_32_picking_sorting \
	333_nehe_pb_33_tga_variants \
	334_nehe_pb_34_height_map_terrain \
	335_nehe_pb_35_avi_texture_playback \
	336_nehe_pb_36_radial_blur \
	337_nehe_pb_37_cel_shading \
	338_nehe_pb_38_resource_textures \
	339_nehe_pb_39_physics_simulation \
	340_nehe_pb_40_rope_physics \
	341_nehe_pb_41_volumetric_fog \
	342_nehe_pb_42_multiple_viewports \
	343_nehe_pb_43_freetype_fonts \
	344_nehe_pb_44_lens_flare \
	345_nehe_pb_45_vertex_buffers \
	346_nehe_pb_46_antialiasing \
	347_nehe_pb_47_vertex_shader \
	348_nehe_pb_48_arcball_rotation

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
