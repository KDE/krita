add_subdirectory(tests)

#################################################

# Painterly Common Libraries

set(kritapainterlycommon_PART_SRCS
	kis_illuminant_profile.cpp
	kis_ks_colorspace.cpp
	kis_rgbks_colorspace.cpp
	mathematics.cpp
	)

kde4_add_library(kritapainterlycommon SHARED ${kritapainterlycommon_PART_SRCS})
target_link_libraries(kritapainterlycommon kritaimage glpk)
install(TARGETS kritapainterlycommon DESTINATION ${LIB_INSTALL_DIR})

#################################################

# Painterly Mixer

set(kritapainterlymixer_PART_SRCS
	colorspot.cpp
	mixercanvas.cpp
    mixertool.cpp
    kis_painterlymixer.cpp
    kis_painterlymixerdocker.cpp
    painterlymixer.cpp
    )

kde4_add_ui_files(kritapainterlymixer_PART_SRCS kis_painterlymixer.ui)
kde4_add_plugin(kritapainterlymixer ${kritapainterlymixer_PART_SRCS})
target_link_libraries(kritapainterlymixer kritapainterlycommon kritaui)
install(TARGETS kritapainterlymixer DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kritapainterlymixer.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES IlluminantD50.ill IlluminantD65.ill IlluminantD75.ill DESTINATION ${DATA_INSTALL_DIR}/krita/profiles)

#################################################

# Painterly Complex Paintop

set(kritacomplexbrush_PART_SRCS
    complexbrush.cpp
    kis_complexop.cpp
    )

kde4_add_plugin(kritacomplexbrush ${kritacomplexbrush_PART_SRCS})
target_link_libraries(kritacomplexbrush kritapainterlycommon)
install(TARGETS kritacomplexbrush DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kritacomplexbrush.desktop DESTINATION ${SERVICES_INSTALL_DIR})
