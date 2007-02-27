project( defaulttools )

include_directories( ${FLAKE_INCLUDES} ${QT_INCLUDES} ${KOFFICECORE_INCLUDES} )

########### Flake Plugin library ###############

SET ( defaulttools_SRCS
    Plugin.cpp
    #ConnectionTool.cpp
    #ConnectionToolFactory.cpp
    ZoomTool.cpp
)

#kde4_add_ui_files(defaulttools_SRCS
#)

kde4_automoc(${defaulttools_SRCS})
kde4_add_plugin(defaulttools ${defaulttools_SRCS})
target_link_libraries(defaulttools flake kofficeui kotext)
install(TARGETS defaulttools DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES defaulttools.desktop DESTINATION ${SERVICES_INSTALL_DIR})
#kde4_install_icons( ${DATA_INSTALL_DIR}/koffice/icons )
