set(kritatoolsmartpatch_SOURCES
    tool_smartpatch.cpp
    kis_tool_smart_patch.cpp
    kis_tool_smart_patch_options_widget.cpp
    )

#ki18n_wrap_ui(kritatoolsmartpatch_SOURCES kis_tool_smart_patch_options_widget.ui)

add_library(kritatoolsmartpatch MODULE ${kritatoolsmartpatch_SOURCES})

generate_export_header(kritatoolsmartpatch BASE_NAME kritatoolsmartpatch)

target_link_libraries(kritatoolsmartpatch kritaui)

install(TARGETS kritatoolsmartpatch  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

