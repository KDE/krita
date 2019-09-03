set(KRITA_TOUCHDOCKER_SOURCES 
    TouchDockerPlugin.cpp 
    TouchDockerDock.cpp 
)

qt5_add_resources(KRITA_TOUCHDOCKER_SOURCES touchdocker.qrc)

kis_add_library(kritatouchdocker MODULE ${KRITA_TOUCHDOCKER_SOURCES})
target_link_libraries(kritatouchdocker 
    kritaui 
    kritaqml
    Qt5::Quick  
    Qt5::QuickWidgets 
    Qt5::Qml
)
install(TARGETS kritatouchdocker  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
