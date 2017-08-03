set(kritawatercolorpaintop_SOURCES
    kis_splat.cpp
    kis_wetmap.cpp
    kis_base_splats_plane.cpp
    watercolor_paintop_plugin.cpp
    kis_watercolor_paintop.cpp
    kis_watercolor_paintop_settings.cpp
    kis_watercolor_paintop_settings_widget.cpp
    kis_watercolorop_option.cpp
    kis_watercolor_base_items.cpp
    kis_splat_generator_strategy.cpp
    )
ki18n_wrap_ui(kritawatercolorpaintop_SOURCES wdgwatercoloroptions.ui )
#add_subdirectory(tests)

add_library(kritawatercolorpaintop MODULE ${kritawatercolorpaintop_SOURCES})
generate_export_header(kritawatercolorpaintop EXPORT_MACRO_NAME WATERCOLORPAINT_EXPORT)

target_link_libraries(kritawatercolorpaintop kritalibpaintop)

install(TARGETS kritawatercolorpaintop  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
