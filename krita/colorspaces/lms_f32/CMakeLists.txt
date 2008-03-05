


########### next target ###############

set(krita_lms_f32_plugin_PART_SRCS lms_f32_plugin.cc kis_lms_f32_colorspace.cc)


kde4_add_plugin(krita_lms_f32_plugin ${krita_lms_f32_plugin_PART_SRCS})



target_link_libraries(krita_lms_f32_plugin  ${KDE4_KDECORE_LIBS} pigmentcms )

install(TARGETS krita_lms_f32_plugin  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  krita_lms_f32_plugin.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


install(FILES
    lmsaf32.ctlp
    DESTINATION ${DATA_INSTALL_DIR}/pigmentcms/ctlprofiles)

