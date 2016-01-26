set(kritaposterize_SOURCES posterize.cpp )
add_library(kritaposterize MODULE ${kritaposterize_SOURCES})
target_link_libraries(kritaposterize kritaui)
install(TARGETS kritaposterize  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
