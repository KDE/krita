set(kritagmic-qt_SOURCES gmic-qt.cpp converter.cpp)
add_library(kritagmic-qt MODULE ${kritagmic-qt_SOURCES})
target_link_libraries(kritagmic-qt kritaui)
install(TARGETS kritagmic-qt  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
