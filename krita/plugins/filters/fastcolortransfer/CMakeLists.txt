

set(kritafastcolortransferfilter_PART_SRCS
    fastcolortransfer.cc
    kis_wdg_fastcolortransfer.cpp
    )

kde4_automoc(${kritafastcolortransferfilter_PART_SRCS})

kde4_add_ui_files(kritafastcolortransferfilter_PART_SRCS
    wdgfastcolortransfer.ui
    )

kde4_add_plugin(kritafastcolortransferfilter ${kritafastcolortransferfilter_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritafastcolortransferfilter )

target_link_libraries(kritafastcolortransferfilter kritaui)

install(TARGETS kritafastcolortransferfilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritafastcolortransferfilter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
