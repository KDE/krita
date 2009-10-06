include_directories(  )

set(kritadefaultdockers_PART_SRCS 
    kis_palette_docker.cc
    kis_birdeye_box.cc
    kis_layer_box.cpp
    histogramdock.cpp
    histogram_updater.cpp
    kis_accumulating_producer.cc
    kis_cachedhistogram.cc
    kis_imagerasteredcache.cc
    defaultdockers.cpp
)

set(kritadefaultdockers_PART_HEADERS
    kis_palette_docker.h
    kis_birdeye_box.h
    kis_layer_box.h
    histogramdock_factory.h
    histogramdock.h
    histogram_updater.h
    kis_accumulating_producer.h
    kis_cachedhistogram.h
    kis_imagerasteredcache.h
    defaultdockers.h
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

