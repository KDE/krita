include_directories( ${FLAKE_INCLUDES} )

########### Flake Plugin library ###############

set (pictureshape_LIB_SRCS
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

ki18n_wrap_ui(pictureshape_LIB_SRCS forms/wdgPictureTool.ui)
add_library(calligra_shape_picture MODULE ${pictureshape_LIB_SRCS})

target_link_libraries(calligra_shape_picture flake ${KDE4_KFILE_LIBRARY})
install(TARGETS calligra_shape_picture DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES calligra_shape_picture.desktop DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
ecm_install_icons( ${DATA_INSTALL_DIR}/calligra/icons )
