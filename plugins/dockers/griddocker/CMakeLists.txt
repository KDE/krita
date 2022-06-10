set(KRITA_GRIDDOCKER_SOURCES griddocker.cpp griddocker_dock.cpp grid_config_widget.cpp)
ki18n_wrap_ui(KRITA_GRIDDOCKER_SOURCES
    grid_config_widget.ui
)

kis_add_library(kritagriddocker MODULE ${KRITA_GRIDDOCKER_SOURCES})
target_link_libraries(kritagriddocker kritaui)
install(TARGETS kritagriddocker  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
