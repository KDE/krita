set(KRITA_TOOLPRESETSDOCKER_SOURCES 
    ToolPresetsPlugin.cpp 
    ToolPresets.cpp 
)

ki18n_wrap_ui(KRITA_TOOLPRESETSDOCKER_SOURCES
    WdgToolPresets.ui
)


add_library(kritatoolpresetsdocker MODULE ${KRITA_TOOLPRESETSDOCKER_SOURCES})
target_link_libraries(kritatoolpresetsdocker
    kritaui 
)
install(TARGETS kritatoolpresetsdocker DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

