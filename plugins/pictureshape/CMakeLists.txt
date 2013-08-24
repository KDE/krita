include_directories( ${FLAKE_INCLUDES} )

########### Flake Plugin library ###############

SET (pictureshape_LIB_SRCS
    PictureShape.cpp
    PictureShapeFactory.cpp
    PictureTool.cpp
    PictureToolFactory.cpp
    Plugin.cpp
    PictureShapeConfigWidget.cpp
    ChangeImageCommand.cpp
    ClipCommand.cpp
    CropWidget.cpp
    SelectionRect.cpp
    ClippingRect.cpp
    filters/GreyscaleFilterEffect.cpp
    filters/MonoFilterEffect.cpp
    filters/WatermarkFilterEffect.cpp
)

kde4_add_ui_files(pictureshape_LIB_SRCS forms/wdgPictureTool.ui)
kde4_add_plugin(calligra_shape_picture ${pictureshape_LIB_SRCS})

target_link_libraries(calligra_shape_picture komain flake ${KDE4_KFILE_LIBRARY})
install(TARGETS calligra_shape_picture DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES calligra_shape_picture.desktop DESTINATION ${SERVICES_INSTALL_DIR})
kde4_install_icons( ${DATA_INSTALL_DIR}/calligra/icons )
