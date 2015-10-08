project( artistictextshape )

add_definitions(-DTRANSLATION_DOMAIN=\"krita_shape_artistictext\")

include_directories(
    ${CMAKE_SOURCE_DIR}/libs/widgets
    ${CMAKE_SOURCE_DIR}/libs/main # for KoFontComboBox.h
    ${FLAKE_INCLUDES}
)

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

ki18n_wrap_ui( ArtisticTextShape_SRCS
    ArtisticTextShapeConfigWidget.ui
    ArtisticTextShapeOnPathWidget.ui
)

add_library(krita_shape_artistictext MODULE ${ArtisticTextShape_SRCS} )

kcoreaddons_desktop_to_json(krita_shape_artistictext calligra_shape_artistictext.desktop)

target_link_libraries(krita_shape_artistictext kritaflake)

install( TARGETS krita_shape_artistictext DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR} )
