set(KRITA_STORYBOARDDOCKER_SOURCES
    storyboarddocker.cpp storyboarddocker_dock.cpp
    )

ki18n_wrap_ui(KRITA_STORYBOARDDOCKER_SOURCES
    wdgstoryboarddock.ui wdgarrangemenu.ui wdgcommentmenu.ui
)

add_library(kritastoryboarddocker MODULE ${KRITA_STORYBOARDDOCKER_SOURCES})
target_link_libraries(kritastoryboarddocker kritaui)
install(TARGETS kritastoryboarddocker  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})