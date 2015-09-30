project(vectorshape)

add_definitions(-DTRANSLATION_DOMAIN=\"krita_shape_vector\")

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

add_library(krita_shape_vector MODULE ${VectorShape_SRCS})
kcoreaddons_desktop_to_json(krita_shape_vector calligra_shape_vector.desktop)

target_link_libraries(krita_shape_vector kritaflake kritavectorimage KF5::I18n KF5::KDELibs4Support Qt5::Svg)

install(TARGETS krita_shape_vector DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

########### install files ###############

if (SHARED_MIME_INFO_FOUND)
    install(FILES krita_svm.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
    update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})
endif ()

########### subdirectories ###############

#add_subdirectory( libemf )
