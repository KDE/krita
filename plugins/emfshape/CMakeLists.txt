project( emfShape)

# The library is directly under this directory.
# When it becomes an entity on its own we have to create
# some cmake code here to find it.
SET( EMF_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR/libemf} )

include_directories( ${KOMAIN_INCLUDES} ${FLAKE_INCLUDES} ${EMF_INCLUDE_DIR})

SET ( EmfShape_SRCS
    EmfShapePlugin.cpp
    EmfShape.cpp
    EmfShapeFactory.cpp
    # No tool yet.
    #EmfTool.cpp
    #EmfToolFactory.cpp

    libemf/EmfRecords.cpp
    libemf/EmfHeader.cpp
    libemf/EmfParser.cpp
    libemf/EmfOutput.cpp
    libemf/EmfOutputDebugStrategy.cpp
    libemf/EmfOutputPainterStrategy.cpp
)

#kde4_add_ui_files(EmfShape_SRCS
#)

kde4_add_plugin(emfshape ${EmfShape_SRCS})

target_link_libraries(emfshape flake qemf)

install(TARGETS emfshape DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES emfshape.desktop DESTINATION ${SERVICES_INSTALL_DIR})
#kde4_install_icons( ${DATA_INSTALL_DIR}/koffice/icons )


########### subdirectories ###############

add_subdirectory( libemf )
