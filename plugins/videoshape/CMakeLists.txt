include_directories( ${PHONON_INCLUDE_DIR} )

########### Flake Plugin library ###############

SET (videoshape_LIB_SRCS
    VideoData.cpp
    VideoCollection.cpp
    VideoShape.cpp
    VideoShapeFactory.cpp
    VideoEventAction.cpp
    Plugin.cpp
    VideoShapeConfigWidget.cpp
)

kde4_add_plugin(videoshape ${videoshape_LIB_SRCS})

target_link_libraries(videoshape komain
    ${KDE4_KFILE_LIBRARY}
    ${KDE4_PHONON_LIBRARY}
)
install(TARGETS videoshape DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES videoshape.desktop DESTINATION ${SERVICES_INSTALL_DIR})
kde4_install_icons( ${DATA_INSTALL_DIR}/koffice/icons )
