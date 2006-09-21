

set(kritahistogramdocker_PART_SRCS
    histogramdocker.cc
    kis_imagerasteredcache.cc
    kis_cachedhistogram.cc
    kis_accumulating_producer.cc
    )

kde4_automoc(${kritahistogramdocker_PART_SRCS})

kde4_add_plugin(kritahistogramdocker ${kritahistogramdocker_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritahistogramdocker )

target_link_libraries(kritahistogramdocker kritaui)

install(TARGETS kritahistogramdocker  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritahistogramdocker.rc  DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)
install( FILES  kritahistogramdocker.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

