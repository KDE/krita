set(kritalibkis_LIB_SRCS
    Action.cpp
    Canvas.cpp
    Channel.cpp
    ColorDepth.cpp
    ColorModel.cpp
    ColorProfile.cpp
    DockWidget.cpp
    DockWidgetFactoryBase.cpp
    Document.cpp
    Filter.cpp
    Generator.cpp
    InfoObject.cpp
    Krita.cpp
    Node.cpp
    Notifier.cpp
    Resource.cpp
    Selection.cpp
    Transformation.cpp
    View.cpp
    ViewExtension.cpp
    Window.cpp
)

add_library(kritalibkis SHARED ${kritalibkis_LIB_SRCS} )
generate_export_header(kritalibkis)

target_link_libraries(kritalibkis kritaui kritaimage kritaversion)
target_link_libraries(kritalibkis LINK_INTERFACE_LIBRARIES kritaimage kritaui)

set_target_properties(kritalibkis PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)

install(TARGETS kritalibkis  ${INSTALL_TARGETS_DEFAULT_ARGS})

