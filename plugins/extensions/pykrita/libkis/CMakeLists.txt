set(kritalibkis_LIB_SRCS
    application.cpp
    canvas.cpp
    document.cpp
    dockwidget.cpp
    dockwidgetfactorybase.cpp
    image.cpp
    mainwindow.cpp
    krita.cpp
    node.cpp
    view.cpp
    viewextension.cpp
    viewmanager.cpp
)

add_library(kritalibkis SHARED ${kritalibkis_LIB_SRCS} )
generate_export_header(kritalibkis)

target_link_libraries(kritalibkis kritaui kritaimage)
target_link_libraries(kritalibkis LINK_INTERFACE_LIBRARIES kritaimage kritaui)

set_target_properties(kritalibkis PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)

install(TARGETS kritalibkis  ${INSTALL_TARGETS_DEFAULT_ARGS})

