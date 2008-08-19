add_subdirectory( tests )

set(kritalibpaintop_LIB_SRCS
    kis_auto_brush.cpp
    kis_brush_based_paintop.cpp
    kis_auto_brush_widget.cpp
    kis_brush.cc
    kis_brush_chooser.cpp
    kis_brush_option.cpp
    kis_brush_p.cc
    kis_curve_option.cpp
    kis_custom_brush.cc
    kis_filter_option.cpp
    kis_imagepipe_brush.cc
    kis_paintop_option.cpp
    kis_paintop_presets_widget.cpp
    kis_pressure_darken_option.cpp
    kis_pressure_opacity_option.cpp
    kis_pressure_size_option.cpp
    kis_text_brush.cc
    kis_brush_server.cpp
    kis_brush_selection_widget.cpp
)

kde4_add_ui_files(kritalibpaintop_LIB_SRCS
    wdgautobrush.ui
    wdgcustombrush.ui
    wdgtextbrush.ui
)

kde4_add_library(kritalibpaintop SHARED ${kritalibpaintop_LIB_SRCS} )

target_link_libraries(kritalibpaintop kritaui)

set_target_properties(kritalibpaintop PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )

install(TARGETS kritalibpaintop  DESTINATION ${LIB_INSTALL_DIR})

