project( artistictextshape )

include_directories( ${FLAKE_INCLUDES} )

set ( ArtisticTextShape_SRCS
    ArtisticTextShapePlugin.cpp
    ArtisticTextShape.cpp
    ArtisticTextRange.cpp
    ArtisticTextShapeFactory.cpp
    ArtisticTextTool.cpp
    ArtisticTextToolFactory.cpp
    ArtisticTextToolSelection.cpp
    ArtisticTextShapeConfigWidget.cpp
    ArtisticTextShapeOnPathWidget.cpp
    ArtisticTextShapeLoadingUpdater.cpp
    ArtisticTextLoadingContext.cpp
    AttachTextToPathCommand.cpp
    DetachTextFromPathCommand.cpp
    ChangeTextOffsetCommand.cpp
    ChangeTextFontCommand.cpp
    ChangeTextAnchorCommand.cpp
    AddTextRangeCommand.cpp
    RemoveTextRangeCommand.cpp
    MoveStartOffsetStrategy.cpp
    SelectTextStrategy.cpp
    ReplaceTextRangeCommand.cpp
)

kde4_add_ui_files( ArtisticTextShape_SRCS
    ArtisticTextShapeConfigWidget.ui
    ArtisticTextShapeOnPathWidget.ui
)

kde4_add_plugin( calligra_shape_artistictext ${ArtisticTextShape_SRCS} )

target_link_libraries(calligra_shape_artistictext flake)

install( TARGETS calligra_shape_artistictext DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES calligra_shape_artistictext.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
