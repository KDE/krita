project(TextNGshape)

set (TextNGShape_SRCS
    TextNGShapePlugin.cpp
    TextNGShape.cpp
    TextNGShapeConfigWidget.cpp
    TextNGShapeFactory.cpp
    TextNGTool.cpp
    TextNGToolFactory.cpp
    ChangeTextNGDataCommand.cpp
)

ki18n_wrap_ui(TextNGShape_SRCS wdgTextNGtexteditor.ui)

add_library(krita_shape_TextNG MODULE ${TextNGShape_SRCS})

target_link_libraries(krita_shape_TextNG kritaflake kritawidgets KF5::I18n)

install(TARGETS krita_shape_TextNG DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

