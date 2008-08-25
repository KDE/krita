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

target_link_libraries(simpletextshape flake ${QT_QTGUI_LIBRARY} ${KDE4_KUTILS_LIBS} komain)

install( TARGETS simpletextshape DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES simpletextshape.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
