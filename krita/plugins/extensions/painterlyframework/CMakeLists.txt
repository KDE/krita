include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(illuminants)
add_subdirectory(tests)

#################################################
# Painterly Common Libraries

set(kritakscolorspacesplugin_PART_SRCS
    ks_colorspaces.cpp
    kis_illuminant_profile.cpp)

kde4_add_plugin(kritakscolorspacesplugin ${kritakscolorspacesplugin_PART_SRCS})
target_link_libraries(kritakscolorspacesplugin pigmentcms ${OPENEXR_LIBRARIES})
install(TARGETS kritakscolorspacesplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES  kritakscolorspacesplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})

#################################################

# Painterly Mixer

set(kritapainterlymixer_PART_SRCS
    colorspot.cpp
    mixercanvas.cpp
    mixertool.cpp
    kis_painterlymixer.cpp
    kis_painterlymixerdocker.cpp
    painterlymixer.cpp
    kis_illuminant_profile.cpp
    mathematics.cpp
    )

kde4_add_ui_files(kritapainterlymixer_PART_SRCS kis_painterlymixer.ui)
kde4_add_plugin(kritapainterlymixer ${kritapainterlymixer_PART_SRCS})
target_link_libraries(kritapainterlymixer kritaui ${OPENEXR_LIBRARIES})
install(TARGETS kritapainterlymixer DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kritapainterlymixer.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES
    krita_tool_knife.png
DESTINATION ${DATA_INSTALL_DIR}/krita/pics)

