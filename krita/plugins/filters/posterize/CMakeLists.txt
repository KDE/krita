

set(kritaposterize_SOURCES posterize.cpp )

add_library(kritaposterize MODULE ${kritaposterize_SOURCES})
kcoreaddons_desktop_to_json(kritaposterize kritaposterize.desktop)


target_link_libraries(kritaposterize kritaui)

install(TARGETS kritaposterize  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
