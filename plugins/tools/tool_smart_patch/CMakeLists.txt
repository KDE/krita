set(kritatoolSmartPatch_SOURCES
    tool_smartpatch.cpp
    kis_tool_smart_patch.cpp
    kis_tool_smart_patch_options_widget.cpp
    kis_inpaint.cpp
    )

ki18n_wrap_ui(kritatoolSmartPatch_SOURCES kis_tool_smart_patch_options_widget.ui)

kis_add_library(kritatoolSmartPatch MODULE ${kritatoolSmartPatch_SOURCES})

generate_export_header(kritatoolSmartPatch BASE_NAME kritatoolSmartPatch)

target_link_libraries(kritatoolSmartPatch kritaui)

install(TARGETS kritatoolSmartPatch  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

