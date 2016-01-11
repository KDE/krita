project( defaulttools )

add_definitions(-DTRANSLATION_DOMAIN=\"krita_flaketools\")

include_directories(
    ${TEXTLAYOUT_INCLUDES} # needed because ConnectionTool.cpp includes TextShape.h to get the id
    ${CMAKE_SOURCE_DIR}/libs/widgets
    ${FLAKE_INCLUDES}
    ${TEXTLAYOUT_INCLUDES}
)

########### Flake Plugin library ###############

set ( defaulttools_SRCS
    Plugin.cpp
    defaulttool/DefaultTool.cpp
    defaulttool/DefaultToolFactory.cpp
    defaulttool/DefaultToolWidget.cpp
    defaulttool/DefaultToolArrangeWidget.cpp
    defaulttool/DefaultToolTransformWidget.cpp
    defaulttool/ShapeMoveStrategy.cpp
    defaulttool/ShapeResizeStrategy.cpp
    defaulttool/ShapeRotateStrategy.cpp
    defaulttool/ShapeShearStrategy.cpp
    defaulttool/SelectionDecorator.cpp
    defaulttool/SelectionTransformCommand.cpp

    guidestool/GuidesTool.cpp
    guidestool/GuidesToolFactory.cpp
    guidestool/GuidesToolOptionWidget.cpp
    guidestool/InsertGuidesToolOptionWidget.cpp

    connectionTool/ConnectionTool.cpp
    connectionTool/ConnectionToolFactory.cpp
    connectionTool/AddConnectionPointCommand.cpp
    connectionTool/RemoveConnectionPointCommand.cpp
    connectionTool/ChangeConnectionPointCommand.cpp
    connectionTool/MoveConnectionPointStrategy.cpp
    connectionTool/ConnectionPointWidget.cpp
)

ki18n_wrap_ui(defaulttools_SRCS
    defaulttool/DefaultToolWidget.ui
    defaulttool/DefaultToolArrangeWidget.ui
    defaulttool/DefaultToolTransformWidget.ui
    guidestool/GuidesToolOptionWidget.ui
    guidestool/InsertGuidesToolOptionWidget.ui
    connectionTool/ConnectionPointWidget.ui
)

qt5_add_resources(defaulttools_SRCS defaulttools.qrc)

add_library(krita_flaketools MODULE ${defaulttools_SRCS})
target_link_libraries(krita_flaketools kritaflake kritawidgets)
install(TARGETS krita_flaketools DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
