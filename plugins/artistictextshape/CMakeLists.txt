project( artistictextshape )

include_directories( ${FLAKE_INCLUDES} )

SET ( ArtisticTextShape_SRCS
    ArtisticTextShapePlugin.cpp
    ArtisticTextShape.cpp
    ArtisticTextShapeFactory.cpp
    ArtisticTextTool.cpp
    ArtisticTextToolFactory.cpp
    ArtisticTextShapeConfigWidget.cpp
    ArtisticTextShapeLoadingUpdater.cpp
    AttachTextToPathCommand.cpp
    DetachTextFromPathCommand.cpp
    ChangeTextOffsetCommand.cpp
    ChangeTextFontCommand.cpp
    ChangeTextAnchorCommand.cpp
)

kde4_add_ui_files( ArtisticTextShape_SRCS
    ArtisticTextShapeConfigWidget.ui
)

kde4_add_plugin( artistictextshape ${ArtisticTextShape_SRCS} )

target_link_libraries(artistictextshape flake)

install( TARGETS artistictextshape DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES artistictextshape.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
