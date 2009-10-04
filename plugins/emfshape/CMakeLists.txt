project( emfShape)

include_directories( ${KOMAIN_INCLUDES} ${FLAKE_INCLUDES} )

SET ( EmfShape_SRCS
    EmfShapePlugin.cpp
    EmfShape.cpp
    EmfShapeFactory.cpp
    #EmfTool.cpp
    #EmfToolFactory.cpp
)

#kde4_add_ui_files(EmfShape_SRCS
#)

kde4_add_plugin(emfshape ${EmfShape_SRCS})

target_link_libraries(emfshape flake)

install(TARGETS emfshape DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES emfshape.desktop DESTINATION ${SERVICES_INSTALL_DIR})
#kde4_install_icons( ${DATA_INSTALL_DIR}/koffice/icons )
