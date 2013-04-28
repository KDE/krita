project( vectorshape)

include_directories(${KOMAIN_INCLUDES} ${FLAKE_INCLUDES} ${VECTORIMAGE_INCLUDES})

SET ( VectorShape_SRCS
    VectorShapePlugin.cpp
    VectorShape.cpp
    VectorShapeConfigWidget.cpp
    VectorShapeFactory.cpp
    #VectorCollection.cpp
    #VectorData.cpp
    VectorTool.cpp
    VectorToolFactory.cpp
    ChangeVectorDataCommand.cpp
)

#kde4_add_ui_files(VectorShape_SRCS
#)

kde4_add_plugin(vectorshape ${VectorShape_SRCS})

target_link_libraries(vectorshape flake vectorimage ${KDE4_KFILE_LIBRARY})

install(TARGETS vectorshape DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES vectorshape.desktop DESTINATION ${SERVICES_INSTALL_DIR})
#kde4_install_icons( ${DATA_INSTALL_DIR}/calligra/icons )

if (SHARED_MIME_INFO_FOUND)
    install(FILES calligra_svm.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
    update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})
endif (SHARED_MIME_INFO_FOUND)

########### subdirectories ###############

#add_subdirectory( libemf )
