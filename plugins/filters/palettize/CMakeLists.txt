set(kritapalettize_SOURCES palettize.cpp)
add_library(kritapalettize MODULE ${kritapalettize_SOURCES})
target_link_libraries(kritapalettize kritaui)
install(TARGETS kritapalettize  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
