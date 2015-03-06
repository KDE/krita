project( vectorshape)

include_directories(${FLAKE_INCLUDES} ${VECTORIMAGE_INCLUDES})

set ( VectorShape_SRCS
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

#ki18n_wrap_ui(VectorShape_SRCS
#)

add_library(calligra_shape_vector MODULE ${VectorShape_SRCS})

target_link_libraries(calligra_shape_vector flake kovectorimage ${KDE4_KFILE_LIBRARY})

install(TARGETS calligra_shape_vector DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES calligra_shape_vector.desktop DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
#ecm_install_icons( ${DATA_INSTALL_DIR}/calligra/icons )

if (SHARED_MIME_INFO_FOUND)
    install(FILES calligra_svm.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
    update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})
endif ()

########### subdirectories ###############

#add_subdirectory( libemf )
