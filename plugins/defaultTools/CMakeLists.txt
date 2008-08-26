project( defaulttools )

include_directories( ${FLAKE_INCLUDES} ${QT_INCLUDES} ${KOMAIN_INCLUDES} ${KOGUIUTILS_INCLUDES})

add_subdirectory(pics)


########### Flake Plugin library ###############

SET ( defaulttools_SRCS
    Plugin.cpp
    defaulttool/DefaultTool.cpp
    defaulttool/DefaultToolFactory.cpp
    defaulttool/DefaultToolWidget.cpp
    defaulttool/ShapeMoveStrategy.cpp
    defaulttool/ShapeResizeStrategy.cpp
    defaulttool/ShapeRotateStrategy.cpp
    defaulttool/ShapeShearStrategy.cpp
    defaulttool/SelectionDecorator.cpp

    guidestool/GuidesTool.cpp
    guidestool/GuidesToolFactory.cpp
    guidestool/GuidesToolOptionWidget.cpp
)

kde4_add_ui_files(defaulttools_SRCS
    defaulttool/DefaultToolWidget.ui
    guidestool/GuidesToolOptionWidget.ui
)

kde4_add_plugin(defaulttools ${defaulttools_SRCS})
target_link_libraries(
    defaulttools
    flake
    koguiutils
    kotext
    komain
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDE3SUPPORT_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QT3SUPPORT_LIBRARY}
    )
install(TARGETS defaulttools DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES defaulttools.desktop DESTINATION ${SERVICES_INSTALL_DIR})

#kde4_install_icons( ${DATA_INSTALL_DIR}/koffice/icons )
