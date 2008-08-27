project( simpletextshape )

include_directories( ${KOMAIN_INCLUDES} ${FLAKE_INCLUDES} )

SET ( SimpleTextShape_SRCS
    SimpleTextShapePlugin.cpp
    SimpleTextShape.cpp
    SimpleTextShapeFactory.cpp
    SimpleTextTool.cpp
    SimpleTextToolFactory.cpp
    SimpleTextShapeConfigWidget.cpp
)

kde4_add_ui_files( SimpleTextShape_SRCS
    SimpleTextShapeConfigWidget.ui
)

kde4_add_plugin( simpletextshape ${SimpleTextShape_SRCS} )

target_link_libraries(
    simpletextshape
    flake
    komain
    kstore
    ${KDE4_KDEUI_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    )

install( TARGETS simpletextshape DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES simpletextshape.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
