project(vectorshape)

add_definitions(-DTRANSLATION_DOMAIN=\"krita_shape_vector\")

include_directories(${KOWIDGETS_INCLUDES} ${VECTORIMAGE_INCLUDES})

set ( VectorShape_SRCS
    VectorShapePlugin.cpp
    VectorShape.cpp
    VectorShapeConfigWidget.cpp
    VectorShapeFactory.cpp
    VectorTool.cpp
    VectorToolFactory.cpp
    ChangeVectorDataCommand.cpp
)

add_library(krita_shape_vector MODULE ${VectorShape_SRCS})

target_link_libraries(krita_shape_vector kritaflake kritawidgets kritavectorimage KF5::I18n Qt5::Svg)

install(TARGETS krita_shape_vector DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})

if (SHARED_MIME_INFO_FOUND)
    install(FILES krita_svm.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
    update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})
endif ()

