include_directories(  )

########### Flake Plugin library ###############

SET (pictureshape_LIB_SRCS
    PictureShape.cpp
    PictureShapeFactory.cpp
    PictureTool.cpp
    PictureToolFactory.cpp
    Plugin.cpp
    PictureShapeConfigWidget.cpp
    ChangeImageCommand.cpp
)

kde4_add_plugin(pictureshape ${pictureshape_LIB_SRCS})

target_link_libraries(
    pictureshape
    flake
    komain
    kostore
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KFILE_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    )
install(TARGETS pictureshape DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES pictureshape.desktop DESTINATION ${SERVICES_INSTALL_DIR})
kde4_install_icons( ${DATA_INSTALL_DIR}/koffice/icons )
