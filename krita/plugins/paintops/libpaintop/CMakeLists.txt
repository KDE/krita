set(kritalibpaintop_LIB_SRCS
	kis_paintop_option.cpp	
	kis_paintop_presets_widget.cpp
    kis_brush_option.cpp
    kis_curve_option.cpp
    kis_filter_option.cpp
    kis_pressure_darken_option.cpp
    kis_pressure_opacity_option.cpp
    kis_pressure_size_option.cpp
)

#kde4_add_ui_files(kritalibpaintop_LIB_SRCS
#)

kde4_add_library(kritalibpaintop SHARED ${kritalibpaintop_LIB_SRCS} )

target_link_libraries(kritalibpaintop kritaui)

set_target_properties(kritalibpaintop PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )

install(TARGETS kritalibpaintop  DESTINATION ${LIB_INSTALL_DIR})

