add_subdirectory(illuminants)
add_subdirectory(tests)

#################################################
# Painterly Common Libraries

set(kritakscolorspacesplugin_PART_SRCS
    ks_colorspaces.cpp
    kis_illuminant_profile.cpp)

kde4_add_plugin(kritakscolorspacesplugin ${kritakscolorspacesplugin_PART_SRCS})
target_link_libraries(kritakscolorspacesplugin pigmentcms)
install(TARGETS kritakscolorspacesplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES  kritakscolorspacesplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})
