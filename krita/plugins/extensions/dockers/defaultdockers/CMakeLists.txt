set(kritadefaultdockers_PART_SRCS 
    kis_palette_docker.cc
    kis_birdeye_box.cc
    kis_layer_box.cpp
    defaultdockers.cpp
    kis_drop_button.cpp
)

set(kritadefaultdockers_PART_HEADERS
    kis_palette_docker.h
    kis_birdeye_box.h
    kis_layer_box.h
    defaultdockers.h
    kis_drop_button.h
)

kde4_add_ui_files(kritadefaultdockers_PART_SRCS
    wdgpalettechooser.ui
    wdgbirdeye.ui
    wdglayerbox.ui
)


kde4_add_plugin(kritadefaultdockers ${kritadefaultdockers_PART_SRCS} ${kritadefaultdockers_PART_HEADERS})

target_link_libraries(kritadefaultdockers kritaui)

install(TARGETS kritadefaultdockers  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES  kritadefaultdockers.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

