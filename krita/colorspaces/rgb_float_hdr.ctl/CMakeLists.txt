
set(kritargbfloathdrplugin_PART_SRCS rgb_float_hdr.cc kis_rgb_f16_hdr_colorspace.cc kis_rgb_f32_hdr_colorspace.cc )

kde4_add_plugin(kritargbfloathdrplugin ${kritargbfloathdrplugin_PART_SRCS})

target_link_libraries(kritargbfloathdrplugin  ${KDE4_KDECORE_LIBS} ${RGBF16_LIB} kritargbf32hdr  pigmentcms)

install(TARGETS kritargbfloathdrplugin  DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES
    srgbaf.ctlp
    scrgbaf.ctlp
    DESTINATION ${DATA_INSTALL_DIR}/pigmentcms/ctlprofiles)
