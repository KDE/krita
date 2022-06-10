set(kritatoollazybrush_SOURCES
    tool_lazybrush.cpp
    kis_tool_lazy_brush.cpp
    kis_tool_lazy_brush_options_widget.cpp
    )

ki18n_wrap_ui(kritatoollazybrush_SOURCES kis_tool_lazy_brush_options_widget.ui)

kis_add_library(kritatoollazybrush MODULE ${kritatoollazybrush_SOURCES})

generate_export_header(kritatoollazybrush BASE_NAME kritatoollazybrush)

target_link_libraries(kritatoollazybrush kritaui)

install(TARGETS kritatoollazybrush  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

