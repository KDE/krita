add_subdirectory( tests )

set(kritadefaultdockers_SOURCES 
  kis_layer_box.cpp
  defaultdockers.cpp
)

set(kritadefaultdockers_PART_HEADERS
    kis_layer_box.h
    defaultdockers.h
)

ki18n_wrap_ui(kritadefaultdockers_SOURCES
    wdglayerbox.ui
)

add_library(kritadefaultdockers MODULE ${kritadefaultdockers_SOURCES} ${kritadefaultdockers_PART_HEADERS})
generate_export_header(kritadefaultdockers BASE_NAME kritadefaultdockers EXPORT_MACRO_NAME KRITADEFAULTDOCKERS_EXPORT)
target_link_libraries(kritadefaultdockers kritaui)
install(TARGETS kritadefaultdockers  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
