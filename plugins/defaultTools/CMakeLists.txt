project( defaulttools )

add_definitions(-DTRANSLATION_DOMAIN=\"calligra-defaulttools\")

include_directories(
    ${TEXTLAYOUT_INCLUDES} # needed because ConnectionTool.cpp includes TextShape.h to get the id
    ${CMAKE_SOURCE_DIR}/libs/widgets
    ${FLAKE_INCLUDES}
    ${TEXTLAYOUT_INCLUDES}
)

add_subdirectory(pics)


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

add_library(calligra_tool_defaults MODULE ${defaulttools_SRCS})
kcoreaddons_desktop_to_json(calligra_tool_defaults calligra_tool_defaults.desktop)
target_link_libraries(calligra_tool_defaults flake kowidgets KF5::IconThemes )
install(TARGETS calligra_tool_defaults DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
