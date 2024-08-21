add_subdirectory(tests)

set(kritalayerdocker_static_SRCS
    LayerBox.cpp
    NodeDelegate.cpp
    NodeToolTip.cpp
    NodeView.cpp
    NodeViewVisibilityDelegate.cpp
)

ki18n_wrap_ui(kritalayerdocker_static_SRCS
    WdgLayerBox.ui
)


if (QT_MAJOR_VERSION STREQUAL "6")

QT_WRAP_CPP(kritalayerdocker_HEADERS_MOC 
    NodePropertyAction_p.h
    SyncButtonAndAction.h
)

else()

QT5_WRAP_CPP(kritalayerdocker_HEADERS_MOC 
    NodePropertyAction_p.h
    SyncButtonAndAction.h
)

endif()


kis_add_library(kritalayerdocker_static STATIC ${kritalayerdocker_static_SRCS} ${kritalayerdocker_HEADERS_MOC})

generate_export_header(kritalayerdocker_static BASE_NAME kritalayerdocker)

target_link_libraries(kritalayerdocker_static kritaui)

set(kritalayerdocker_SOURCES 
    LayerDocker.cpp
)

kis_add_library(kritalayerdocker MODULE ${kritalayerdocker_SOURCES})

target_link_libraries(kritalayerdocker kritaui kritalayerdocker_static)
install(TARGETS kritalayerdocker  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
