set(KRITA_PRESETHISTORY_SOURCES 
    presethistory.cpp 
    presethistory_dock.cpp 
)

add_library(kritapresethistory MODULE ${KRITA_PRESETHISTORY_SOURCES})

target_link_libraries(kritapresethistory kritaui)
kcoreaddons_desktop_to_json(kritapresethistory krita_presethistory.desktop)

install(TARGETS kritapresethistory  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

