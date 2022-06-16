set(kritaexample_SOURCES example.cpp )
add_library(kritaexample MODULE ${kritaexample_SOURCES})
target_link_libraries(kritaexample kritaui)
install(TARGETS kritaexample  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
