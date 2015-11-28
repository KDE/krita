

set(kritaexample_SOURCES example.cpp )

add_library(kritaexample MODULE ${kritaexample_SOURCES})
kcoreaddons_desktop_to_json(kritaexample kritaexample.desktop)


target_link_libraries(kritaexample kritaui)

install(TARGETS kritaexample  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
