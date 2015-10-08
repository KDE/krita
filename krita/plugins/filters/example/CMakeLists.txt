

set(kritaexample_PART_SRCS example.cpp )

add_library(kritaexample MODULE ${kritaexample_PART_SRCS})
kcoreaddons_desktop_to_json(kritaexample kritaexample.desktop)


target_link_libraries(kritaexample kritaui)

install(TARGETS kritaexample  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
