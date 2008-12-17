

set(kritafastcolortransferfilter_PART_SRCS
    fastcolortransfer.cc
    kis_wdg_fastcolortransfer.cpp
    )


kde4_add_ui_files(kritafastcolortransferfilter_PART_SRCS
    wdgfastcolortransfer.ui
    )

kde4_add_plugin(kritafastcolortransferfilter ${kritafastcolortransferfilter_PART_SRCS})



target_link_libraries(kritafastcolortransferfilter koguiutils kritaimage kritaui ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS})

install(TARGETS kritafastcolortransferfilter  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritafastcolortransfer.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
