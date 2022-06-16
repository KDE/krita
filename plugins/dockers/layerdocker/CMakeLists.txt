#add_subdirectory(tests)

set(kritalayerdocker_SOURCES 
    LayerBox.cpp
    LayerDocker.cpp
    NodeDelegate.cpp
    NodeToolTip.cpp
    NodeView.cpp
    NodeViewVisibilityDelegate.cpp
)


set(kritalayerdocker_PART_HEADERS
    LayerBox.h
    LayerDocker.h
    NodeDelegate.h
    NodeToolTip.h
    NodeView.h
    NodeViewVisibilityDelegate.h
)

ki18n_wrap_ui(kritalayerdocker_SOURCES
    WdgLayerBox.ui
)

QT5_WRAP_CPP(kritalayerdocker_HEADERS_MOC 
    NodePropertyAction_p.h
    SyncButtonAndAction.h
)

add_library(kritalayerdocker MODULE ${kritalayerdocker_SOURCES} 
        ${kritalayerdocker_PART_HEADERS}
        ${kritalayerdocker_HEADERS_MOC}
)

target_link_libraries(kritalayerdocker kritaui)
install(TARGETS kritalayerdocker  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
