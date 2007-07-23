add_subdirectory(tests)

set(kritapainterlymixer_PART_SRCS
    painterlymixer.cpp
    kis_painterlymixer.cpp
    kis_painterlymixerdocker.cpp
	colorspot.cpp
    mixercanvas.cpp
    mixertool.cpp
	kis_illuminant_profile.cpp
	kis_ks_colorspace.cpp
    )


kde4_add_ui_files(kritapainterlymixer_PART_SRCS kis_painterlymixer.ui)

kde4_add_plugin(kritapainterlymixer ${kritapainterlymixer_PART_SRCS})

target_link_libraries(kritapainterlymixer kritaui)

install(TARGETS kritapainterlymixer  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES  kritapainterlymixer.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES  IlluminantD65.ill CIEXYZ10deg.txt DESTINATION ${DATA_INSTALL_DIR}/krita/profiles)
