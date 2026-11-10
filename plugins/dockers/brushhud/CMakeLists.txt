add_subdirectory( tests )

set(KRITA_BRUSHHUD_STATIC_SOURCES 
    kis_brush_hud_properties_config.cpp
)

kis_add_library(kritabrushhud_static STATIC ${KRITA_BRUSHHUD_STATIC_SOURCES})

target_link_libraries(kritabrushhud_static kritaui)

set(KRITA_BRUSHHUD_SOURCES 
    kis_uniform_paintop_property_widget.cpp
    kis_brush_hud.cpp
    kis_dlg_brush_hud_config.cpp
    kis_brush_hud_properties_list.cpp
    brushhuddocker.cpp
    brushhud_dock.cpp
)

ki18n_wrap_ui(KRITA_BRUSHHUD_SOURCES 
    kis_dlg_brush_hud_config.ui
)

kis_add_library(kritabrushhud MODULE ${KRITA_BRUSHHUD_SOURCES})
target_link_libraries(kritabrushhud kritaui kritabrushhud_static)
install(TARGETS kritabrushhud DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
