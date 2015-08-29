set(kritadefaultdockers_PART_SRCS 
    kis_layer_box.cpp
    defaultdockers.cpp
)

set(kritadefaultdockers_PART_HEADERS
    kis_layer_box.h
    defaultdockers.h
)

ki18n_wrap_ui(kritadefaultdockers_PART_SRCS
    wdglayerbox.ui
)


add_library(kritadefaultdockers MODULE ${kritadefaultdockers_PART_SRCS} ${kritadefaultdockers_PART_HEADERS})
kcoreaddons_desktop_to_json(kritadefaultdockers kritadefaultdockers.desktop)

target_link_libraries(kritadefaultdockers kritaui)

install(TARGETS kritadefaultdockers  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

install( FILES  kritadefaultdockers.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)

