include_directories( ../.. )

set(kopabackgroundtool_PART_SRCS
    Plugin.cpp
    KoPABackgroundTool.cpp
    KoPABackgroundToolFactory.cpp
    KoPAMasterPageDocker.cpp
    )


#kde4_add_ui_files(kpresentertoolanimation_PART_SRCS wdg_tool_crop.ui)

kde4_add_plugin(kopabackgroundtool ${kopabackgroundtool_PART_SRCS})

target_link_libraries(
    kopabackgroundtool
    koguiutils
    flake
    kpresenterprivate
    kopageapp
    ${KDE4_KDECORE_LIBS}
    ${KDE4_PHONON_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QT3SUPPORT_LIBRARY}
    )

install(TARGETS kopabackgroundtool  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kopabackgroundtool.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

#kde4_install_icons( ${DATA_INSTALL_DIR}/koffice/icons  )
