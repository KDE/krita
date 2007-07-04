set(kritapainterlymixer_PART_SRCS
    mixercanvas.cpp
    mixertool.hpp
    kis_painterlymixer.cpp
    kis_painterlymixerdocker.cpp
    painterlymixer.cpp
    )

kde4_automoc(${kritapainterlymixer_PART_SRCS})

kde4_add_ui_files(kritapainterlymixer_PART_SRCS kis_painterlymixer.ui)

kde4_add_plugin(kritapainterlymixer ${kritapainterlymixer_PART_SRCS})

target_link_libraries(kritapainterlymixer kritaui)

install(TARGETS kritapainterlymixer  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES  kritapainterlymixer.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
