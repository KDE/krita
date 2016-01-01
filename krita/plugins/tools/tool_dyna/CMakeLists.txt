set(kritatooldyna_SOURCES
    tool_dyna.cpp
    kis_tool_dyna.cpp
    )


add_library(kritatooldyna MODULE ${kritatooldyna_SOURCES})

target_link_libraries(kritatooldyna kritaui)

install(TARGETS kritatooldyna  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

